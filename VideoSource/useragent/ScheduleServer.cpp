// **********************************************************************
// 作者: 傅佑铭
// 版本: 1.0
// 日期: 2011-01 ~ 2011-11
// 修改历史记录: 
// 日期, 作者, 变更内容
// **********************************************************************
// ScheduleServer.cpp : Defines the entry point for the console application.
//
#include "ScheduleServer.h"
#include "CommandContextParse.h"
//#include "ICECommunicator.h"
#include "MediaData.h"
//#include "json.h"
#include "TimeConsuming.h"
#include "resource.h"
#include "VUASendMediaTask.h"
#include "RTMPPushTask.h"

#include "HttpService.h"

using namespace ScheduleServer;

double CTimeConsuming::_clock_frequency = 0;

CScheduleServer::CScheduleServer() :
_cur_path(""),
_enalble(false),
_register_thread(NULL),
_status_edit(NULL)
{
	_ua_map.clear();
}

CScheduleServer::~CScheduleServer()
{
}

#
SS_Error CScheduleServer::start(std::string path)
{
	//设置当前程序路径
	_cur_path = path;

	//根据系统时间设置随机数
	::srand(timeGetTime());
		
	WSADATA dat;
	WSAStartup(MAKEWORD(2,2),&dat);

	//获取系统信息
	SYSTEM_INFO theSystemInfo;
	::GetSystemInfo(&theSystemInfo);

	//启动任务线程////////////////////////////////////////////////////////////////////////
	CTaskThreadPool::add_threads((!(theSystemInfo.dwNumberOfProcessors) ? 1 : theSystemInfo.dwNumberOfProcessors), this);

	SINGLETON(HttpServer::CHttpService).Initialize();
	SINGLETON(HttpServer::CHttpService).Start("0.0.0.0", SINGLETON(CConfigBox).get_property_as_int("HttpServerPort", 80));

	_enalble = true;//服务可用

	_status_edit = (CEdit*)(AfxGetApp()->GetMainWnd()->GetDlgItem(IDC_EDIT_HTTP_SERVICE));//->SetWindowText(decodedURI.c_str());
	_status = "";

	return SS_NoErr;

}

SS_Error CScheduleServer::shutdown()
{
	_enalble = false;

	//关闭任务线程////////////////////////////////////////////////////////////////////////
	CTaskThreadPool::remove_threads();

	LOG_WRITE("Task threads have been removed!", 1, true);

	//删除所有UA////////////////////////////////////////////////////////////////////////
	remove_all_ua();

	//关闭socket////////////////////////////////////////////////////////////////////////
	WSACleanup();//for JRTP

	return SS_NoErr;
}

void CScheduleServer::write_log(std::string& log, int level, bool show_on_screen)
{
	SINGLETON(CLogger).trace_out(log, level, DEFAULT_TRACE_TYPE, show_on_screen);
}

SS_Error CScheduleServer::add_task(CTask* task, unsigned long index)
{
	if(false == _enalble)
		return SS_AddTaskFail;

	if(NULL  == task)
		return SS_InvalidTask;

	CTaskThread* task_thread = CTaskThreadPool::select_thread(index);

	if(NULL == task_thread)
		return SS_AddTaskFail;

	if(SS_NoErr != task_thread->add_task(task))
	{
		return SS_AddTaskFail;
	}

	return SS_NoErr;
}

int CScheduleServer::start_rtmp_push(const unsigned long uuid)
{
#if 0
	reg_ua(uuid, UA_MobilePhone);

	CUserAgent* ua = SINGLETON(CScheduleServer).fetch_ua(uuid);
	
	int ret = -1;
	if(NULL != ua)
	{
		string url = "rtmp://";
		url += SINGLETON(CConfigBox).get_property("HLSServer", "localhost");
		url += "/hls/";
		url += MiscTools::parse_type_to_string<unsigned long>(uuid);
		ret = 1;//ua->rtmp_connect(const_cast<char*>(url.c_str()));
		if(!ret)
			ua->_start_hls = true;
		else
			return ret;
	}

	memset(ua->_dh_video_frame, 0, sizeof(ua->_dh_video_frame));
	ua->_dh_video_frame_length = 0;
	ua->_last_dh_video_type = NAL_INVALID;

	VUA_SEND_MEDIA_TASK_INFO task_info;

	task_info;
	task_info.task_id = ::timeGetTime();
	task_info.ua_id = uuid;
	task_info.send_audio = false;

	//创建模拟UA发送音视频任务
	_video_task = new CVUASendMediaTask(task_info);

	if(SS_NoErr != SINGLETON(CScheduleServer).add_task(_video_task, task_info.task_id))
	{
		delete _video_task;
		_video_task = NULL;
	}
#endif

	return 0;//ret;
}

int CScheduleServer::stop_rtmp_push(unsigned long uuid)
{
	return 0;
}

void CScheduleServer::singleton_test()
{
	cout << "<ScheduleServer: " << reinterpret_cast<unsigned long>(this) << "> ";
}

void CScheduleServer::print_contex(const SS_CTX& ctx, const bool to_send)
{
	std::string content = "";

	for(SS_CTX::const_iterator iter = ctx.begin();
		iter != ctx.end();
		++iter)
	{
		content += iter->first;
		content += ",";
		content += iter->second;
		content += ";\r\n";
	}

	if(false == to_send)
	{
		LOG_WRITE("\n<<<<<< Receive ICE request >>>>>>\n" + content, 1, true);
	}
	else
	{
		LOG_WRITE("\n<<<<<< Send ICE request >>>>>>\n" + content, 1, true);
	}
}

void CScheduleServer::shutdown_cast(unsigned long id)
{
	CSSLocker lock(&_shutdown_cast_mutex);
#if 0
	CRTMPPushTask* rtmp_task = dynamic_cast<CRTMPPushTask*>(_rtmp_task_map[id]);
	if(NULL != rtmp_task)
	{
		rtmp_task->shutdown();
	}

	CRTSPRecvTask* rtsp_task = dynamic_cast<CRTSPRecvTask*>(_pull_task_map[id]);
	if(NULL != rtsp_task)
	{
		rtsp_task->shutdown();
	}
#else
	CTask* task = _push_task_map[id];
	if(NULL != task)
	{
		task->shutdown();
	}

	task = _pull_task_map[id];
	if(NULL != task)
	{
		task->shutdown();
	}
#endif

	remove_livecast_map(SINGLETON(CScheduleServer).query_livecast_request(MiscTools::parse_type_to_string<unsigned long>(id)));
	remove_vod_map(SINGLETON(CScheduleServer).query_vod_request(MiscTools::parse_type_to_string<unsigned long>(id)));
}

void CScheduleServer::update_expire(unsigned long id, unsigned long expire)
{
	CTask* task = _push_task_map[id];
	if(NULL != task)
	{
		task->update_expire(expire);
	}

	task = _pull_task_map[id];
	if(NULL != task)
	{
		task->update_expire(expire);
	}
}

#define WIDTH	1280
#define HEIGHT	720
//#define WIDTH	352
//#define HEIGHT	288

int CScheduleServer::h264_test()
{
	FILE* f = NULL;
#if 0
	f = fopen("cif.yuv", "rb");
#else
	f = fopen("720p.yuv", "rb");
#endif

	fseek(f, 0, SEEK_SET);

	unsigned char stream_buf[655360];
	int nal_len[1024];

	long handle = H264EncodeInit(WIDTH, HEIGHT, 30, 1000, 30, 1024, 15);

	while(true)
	{
		int length = WIDTH * HEIGHT * 3 / 2;

		unsigned char yuv[WIDTH * HEIGHT * 3 / 2];
		::memset(yuv, 0, sizeof(yuv));

		if(length != fread(yuv, sizeof(unsigned char), length, f))
		{
			fseek(f, 0, SEEK_SET);
			continue;
		}
		
		memset(stream_buf, 0, sizeof(stream_buf));
		memset(nal_len, 0, sizeof(nal_len));

		int i_nal = H264EncodeFrame(handle, -1, yuv, stream_buf, nal_len);

		TRACE("\nnal: %d", i_nal);
	}

	return 0;
}

int CScheduleServer::http_test()
{
	return 0;
}

#include "RTMPPushTask.h"
#include "VideoPullTask.h"
SS_Error CScheduleServer::add_video_pull_task(unsigned long id, string file)
{
	LiveCastRequest req;

	LiveCastResponse res = SINGLETON(CScheduleServer).query_livecast_response(req);

	{
		//unsigned long id = 123;//timeGetTime() & 0xffffff;
		CRTMPPushTask* rtmp_push_task = NULL;
		CDataRecvTask* sdk_recv_task = NULL;//CDPSDKLivePullTask* sdk_recv_task = NULL;

		//RTMP
		{
			RTMP_PUSH_TASK_INFO task_info;

			task_info.task_id = id;
			task_info.ua_id = id;

			task_info.rtmp_url = "rtmp://";
			task_info.rtmp_url += SINGLETON(CConfigBox).get_property("HLSServer", "localhost");
			task_info.rtmp_url += ":" + SINGLETON(CConfigBox).get_property("RTMPServerPort", "1935");
			task_info.rtmp_url += "/hls/";
			task_info.rtmp_url += MiscTools::parse_type_to_string<unsigned long>(task_info.ua_id);
			
			rtmp_push_task = new CRTMPPushTask(task_info);

			if(false == rtmp_push_task->is_initialized())
			{
				return SS_NoErr;
			}

			if(SS_NoErr != SINGLETON(CScheduleServer).add_task(rtmp_push_task, task_info.task_id))
			{
				delete rtmp_push_task;
				rtmp_push_task = NULL;

				return SS_NoErr;
			}
		}

		//SDK
		{
			SDK_RECV_TASK_INFO task_info;

			task_info.task_id = id + 1;
			task_info.ua_id = id;

			task_info.data_url = file;
			//task_info.data_url = "cif.yuv";
			task_info.fps = 15;
			task_info.video_width = 1280;//352;
			task_info.video_height = 720;//288;

			sdk_recv_task = new CVideoPullTask(task_info);

			if(false == sdk_recv_task->is_initialized())
			{
				rtmp_push_task->shutdown();
				return SS_NoErr;
			}

			if(SS_NoErr != SINGLETON(CScheduleServer).add_task(sdk_recv_task, task_info.task_id))
			{
				delete sdk_recv_task;
				sdk_recv_task = NULL;

				rtmp_push_task->shutdown();

				return SS_NoErr;
			}
		}

		//success////////////////////////////////////////////////////////////////////////
		SINGLETON(CScheduleServer)._push_task_map[id] = rtmp_push_task;
		SINGLETON(CScheduleServer)._pull_task_map[id] = sdk_recv_task;

		res.num = MiscTools::parse_type_to_string<unsigned long>(id);
		//res.url = hls_url;
		//res.url2 = rtmp_url;
		res.expire = "300";
		SINGLETON(CScheduleServer).insert_livecast_map(req, res);

		//Json::Value json_value;
		//json_value["StreamNo"] = Json::Value(MiscTools::parse_type_to_string<unsigned long>(id));
		//json_value["StreamUrl"] = Json::Value(hls_url);
		//json_value["StreamUrl2"] = Json::Value(rtmp_url);
		//json_value["ExpireTime"] = Json::Value("300");

		//Json::FastWriter fast_writer;
		//return return_response(0, "success", fast_writer.write(json_value), 6000);//return fast_writer.write(json_value);
	}

	return SS_NoErr;
}

#include "CaptureScreenTask.h"
SS_Error CScheduleServer::add_capture_screen_task(unsigned long id, string window_caption)
{
	LiveCastRequest req;

	LiveCastResponse res = SINGLETON(CScheduleServer).query_livecast_response(req);

	{
		//unsigned long id = 123;//timeGetTime() & 0xffffff;
		//CRTMPPushTask* rtmp_push_task = NULL;
		//CDataRecvTask* sdk_recv_task = NULL;//CDPSDKLivePullTask* sdk_recv_task = NULL;
		rtmp_push_task = NULL;
		sdk_recv_task = NULL;//CDPSDKLivePullTask* sdk_recv_task = NULL;

		//RTMP
		{
			RTMP_PUSH_TASK_INFO task_info;

			task_info.task_id = id;
			task_info.ua_id = id;

			task_info.rtmp_url = "rtmp://";
			task_info.rtmp_url += SINGLETON(CConfigBox).get_property("HLSServer", "localhost");
			task_info.rtmp_url += ":" + SINGLETON(CConfigBox).get_property("RTMPServerPort", "1935");
			task_info.rtmp_url += "/hls/";
			task_info.rtmp_url += MiscTools::parse_type_to_string<unsigned long>(task_info.ua_id);

			rtmp_push_task = new CRTMPPushTask(task_info);

			if(false == dynamic_cast<CRTMPPushTask*>(rtmp_push_task)->is_initialized())
			{
				return SS_NoErr;
			}

			if(SS_NoErr != SINGLETON(CScheduleServer).add_task(rtmp_push_task, task_info.task_id))
			{
				delete rtmp_push_task;
				rtmp_push_task = NULL;

				return SS_NoErr;
			}
		}

		//SDK
		{
			SDK_RECV_TASK_INFO task_info;

			task_info.task_id = id + 1;
			task_info.ua_id = id;

			task_info.fps = 15;
			task_info.video_width = 1280;//352;
			task_info.video_height = 720;//288;

			task_info.window_caption = window_caption;

			sdk_recv_task = new CCaptureScreenTask(task_info);

			if(false == dynamic_cast<CCaptureScreenTask*>(sdk_recv_task)->is_initialized())
			{
				rtmp_push_task->shutdown();
				return SS_NoErr;
			}

			if(SS_NoErr != SINGLETON(CScheduleServer).add_task(sdk_recv_task, task_info.task_id))
			{
				delete sdk_recv_task;
				sdk_recv_task = NULL;

				rtmp_push_task->shutdown();

				return SS_NoErr;
			}
		}

		if(true)
		{
			//窗口置顶
			string caption = SINGLETON(CConfigBox).get_property("Window", "");

			if(false == caption.empty())
			{
				CWnd* pWnd = CWnd::GetDesktopWindow()->GetWindow(GW_CHILD);
		
				while(pWnd != NULL)
				{
					CString strClassName = _T("");
					::GetClassName(pWnd->GetSafeHwnd(),strClassName.GetBuffer(256),256);

					CString strWindowText = _T("");
					::GetWindowText(pWnd->GetSafeHwnd(),strWindowText.GetBuffer(256),256);

					if(-1 != strWindowText.Find(caption.c_str()))
					{
						//pWnd->SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);//窗口置顶
						//::SetWindowPos(pWnd->GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
						::SetForegroundWindow(pWnd->GetSafeHwnd());//激活窗口但不置顶
						break;
					}

					pWnd = pWnd->GetWindow(GW_HWNDNEXT);
				}
			}

			//光标居中
			SetCursorPos(GetSystemMetrics(SM_CXSCREEN) / 2,  GetSystemMetrics(SM_CYSCREEN) / 2);
		}

		//success////////////////////////////////////////////////////////////////////////
		SINGLETON(CScheduleServer)._push_task_map[id] = rtmp_push_task;
		SINGLETON(CScheduleServer)._pull_task_map[id] = sdk_recv_task;

		res.num = MiscTools::parse_type_to_string<unsigned long>(id);
		//res.url = hls_url;
		//res.url2 = rtmp_url;
		res.expire = "300";
		SINGLETON(CScheduleServer).insert_livecast_map(req, res);
	}

	return SS_NoErr;
}

SS_Error CScheduleServer::remove_capture_screen_task()
{
	if(NULL != rtmp_push_task) rtmp_push_task->shutdown();
	if(NULL != sdk_recv_task) sdk_recv_task->shutdown();

	Sleep(3000);

	return SS_NoErr;
}

void CScheduleServer::on_mouse_action(string action, string arg)
{
	//窗口置顶
	string caption = SINGLETON(CConfigBox).get_property("Window", "");

	if(false == caption.empty())
	{
		CWnd* pWnd = CWnd::GetDesktopWindow()->GetWindow(GW_CHILD);
		
		while(pWnd != NULL)
		{
			CString strClassName = _T("");
			::GetClassName(pWnd->GetSafeHwnd(),strClassName.GetBuffer(256),256);

			CString strWindowText = _T("");
			::GetWindowText(pWnd->GetSafeHwnd(),strWindowText.GetBuffer(256),256);

			if(-1 != strWindowText.Find(caption.c_str()))
			{
				//pWnd->SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);//窗口置顶
				//::SetWindowPos(pWnd->GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
				::SetForegroundWindow(pWnd->GetSafeHwnd());//激活窗口但不置顶
				break;
			}

			pWnd = pWnd->GetWindow(GW_HWNDNEXT);
		}
	}

	//光标居中
	SetCursorPos(GetSystemMetrics(SM_CXSCREEN) / 2,  GetSystemMetrics(SM_CYSCREEN) / 2);

	int x_step = SINGLETON(CConfigBox).get_property_as_int("XStep", 50);
	int y_step = SINGLETON(CConfigBox).get_property_as_int("YStep", 50);
	int zoom_step = SINGLETON(CConfigBox).get_property_as_int("ZoomStep", 50);

	if("left" == action)
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		mouse_event(MOUSEEVENTF_MOVE, -x_step, 0, 0, 0);
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	}
	else if("right" == action)
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		mouse_event(MOUSEEVENTF_MOVE, x_step, 0, 0, 0);
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	}
	else if("up" == action)
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		mouse_event(MOUSEEVENTF_MOVE, 0, -y_step, 0, 0);
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	}
	else if("down" == action)
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		mouse_event(MOUSEEVENTF_MOVE, 0, y_step, 0, 0);
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	}
	else if("zoomin" == action)
	{
		mouse_event(MOUSEEVENTF_WHEEL, 0, 0, zoom_step, 0);
	}
	else if("zoomout" == action)
	{
		mouse_event(MOUSEEVENTF_WHEEL, 0, 0, -zoom_step, 0);
	}
}
