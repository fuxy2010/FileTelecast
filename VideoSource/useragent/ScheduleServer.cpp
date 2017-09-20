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
_register_thread(NULL)
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
	SINGLETON(HttpServer::CHttpService).Start(SINGLETON(CConfigBox).get_property("HttpServer", "localhost"), SINGLETON(CConfigBox).get_property_as_int("HttpServerPort", 80));

	_enalble = true;//服务可用

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
