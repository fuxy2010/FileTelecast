// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2011-01 ~ 2011-11
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#include "CaptureScreenTask.h"
#include "MiscTool.h"
#include "TimeConsuming.h"
#include "Transcoding.h"
#include "Sizescale.h"
#include "JRTPSession.h"

#include "scheduleserver.h"

using namespace ScheduleServer;

//int32_t CCaptureScreenTask::_sdk_handler = -1;
//bool CCaptureScreenTask::_sdk_initialized = false;
//CSSMutex CCaptureScreenTask::_sdk_init_mutex;

CCaptureScreenTask::CCaptureScreenTask(SDK_RECV_TASK_INFO& task_info):
CDataRecvTask(task_info),
_window_exist(false),
_bmp_yuv(NULL)
{
	if(!connect_device())
	{
		_status = DataRecvTask_Connected;
		_initialized = true;
	}
	else
	{
		SINGLETON(CScheduleServer).show_log("Failed in cpaturing window!");

		_status = DataRecvTask_Done;
		_initialized = false;
	}

	_sequence = 0;

	//_got_sps_pps = false;
}

#include <Windows.h>
int CCaptureScreenTask::init()
{
	_window_exist = check_window();//if(false == check_window()) return -2;

	if(0 != start_capture()) return -1;

	_next_fetch_video_frame_timestamp = 0;

	_frame_interval = CLOCKS_PER_SEC / _task_info.fps;

	int bitrate = SINGLETON(CConfigBox).get_property_as_int("VideoBitrate", 1024);
	int QP = SINGLETON(CConfigBox).get_property_as_int("VideoQP", 45);
	//_x264_handle = H264EncodeInit(_task_info.video_width, _task_info.video_height, 30, 1000, 30, 1024, _task_info.fps);
	//_x264_handle = H264EncodeInit(_task_info.video_width, _task_info.video_height, QP, 1000, 60, bitrate, _task_info.fps);
	
	//_x264_handle = H264EncodeInit(_task_info.video_width, _task_info.video_height, 35, 1000, 60, 512, _task_info.fps);

	_x264_handle = H264EncodeInit(_task_info.video_width, _task_info.video_height, 30, 1000, 30, bitrate, _task_info.fps);

	_last_video_packet_type = NAL_INVALID;
	_video_frame_length = 0;

	//读取BMP转YUV备用
	if(NULL == _bmp_yuv)
	{
		_bmp_yuv = new unsigned char[_task_info.video_width * _task_info.video_height * 3 / 2];
		memset(_bmp_yuv, 0, _task_info.video_width * _task_info.video_height * 3 / 2);

		SINGLETON(CScheduleServer).bmp_2_yuv420(_bmp_yuv, _task_info.video_width, _task_info.video_height);
	}

	return 0;
}

int CCaptureScreenTask::connect_device()
{
	SINGLETON(CScheduleServer).reg_ua(_task_info.ua_id, UA_MobilePhone);
	_ua = SINGLETON(CScheduleServer).fetch_ua(_task_info.ua_id);

	if(NULL == _ua) return -3;

	if(0 != init()) return -2;

	//memset(_video_frame, 0, 65536 * 100);//memset(_video_frame, 0, sizeof(_video_frame));
	_video_frame_length = 0;
	_last_video_type = NAL_INVALID;

	//memset(_frame, 0, 65536 * 100);//memset(_frame, 0, sizeof(_frame));
	_last_frame_type = NAL_INVALID;
	_frame_pos = 0;

	return 0;
}

int CCaptureScreenTask::disconnect_device()
{
	return 0;
}

bool CCaptureScreenTask::check_window()
{
	string caption = SINGLETON(CConfigBox).get_property("Window", "");

	if(false == caption.empty())
	{
		CWnd* pDesktopWnd = CWnd::GetDesktopWindow();

		//2.获得一个子窗口
		CWnd* pWnd = pDesktopWnd->GetWindow(GW_CHILD);
		//3.循环取得桌面下的所有子窗口
		while(pWnd != NULL)
		{
			//获得窗口类名
			CString strClassName = _T("");
			::GetClassName(pWnd->GetSafeHwnd(),strClassName.GetBuffer(256),256);

			//获得窗口标题
			CString strWindowText = _T("");
			::GetWindowText(pWnd->GetSafeHwnd(),strWindowText.GetBuffer(256),256);

			//AfxMessageBox(strWindowText);
			if(-1 != strWindowText.Find(caption.c_str()))
			{
				_task_info.window_caption = strWindowText;
				return true;
			}

			//继续下一个子窗口
			pWnd = pWnd->GetWindow(GW_HWNDNEXT);
		}

		_task_info.window_caption = caption;
		return false;
	}

	_task_info.window_caption = caption;
	return true;//desktop
}

SS_Error CCaptureScreenTask::run()
{
	if (SS_RecvData != CDataRecvTask::run()) return SS_NoErr;

	if (DataRecvTask_Done == _status) return SS_NoErr;

	if(!_next_fetch_video_frame_timestamp)
		_next_fetch_video_frame_timestamp = timeGetTime();

	if(_next_fetch_video_frame_timestamp > timeGetTime())
	{
		//TRACE("\n<Wait %d -> %d", _next_fetch_video_frame_timestamp, timeGetTime());
		return SS_NoErr;
	}

#if 1
	unsigned long t = timeGetTime();
	if(true == check_window())
	{
		if(false == _window_exist)
		{
			if(0 != start_capture()) return SS_InvalidTask;
		}

		_window_exist = true;
	}
	else
	{
		_window_exist = false;
	}

	TRACE("\nCAP: %d", timeGetTime() - t);
#endif

	//TRACE("\nRUN");

	_next_fetch_video_frame_timestamp += _frame_interval;

	unsigned long t1 = timeGetTime();
	//capture();
	if(true == _window_exist) capture();
	//unsigned long t2 = timeGetTime();
	//CSizescale::scale(_yuv2, _task_info.video_width, _task_info.video_height, _yuv, _screen_width, _screen_height);
	//TRACE("\nScreen: %d, %d", t2 - t1, timeGetTime() - t2);
	else//被捕获程序已退出
	{
		//SINGLETON(CScheduleServer).bmp_2_yuv420(_capture_yuv, _task_info.video_width, _task_info.video_height);
		if(NULL != _bmp_yuv) memcpy(_capture_yuv, _bmp_yuv, _task_info.video_width * _task_info.video_height * 3 / 2);
	}

	if(false)
	{
		FILE* f = fopen("screen.yuv", "ab+");
		fwrite(_capture_yuv, 1, _task_info.video_width * _task_info.video_height * 3 / 2, f);
		//fwrite(_yuv420p, 1, _task_info.video_width * _task_info.video_height * 3 / 2, f);
		fclose(f);
		return SS_NoErr;
	}

	memset(_stream_buf, 0, sizeof(_stream_buf));

	//encode
	int i_nal = H264EncodeFrame(_x264_handle, -1, _capture_yuv, _stream_buf, _nal_len);

	//TRACE("\nnal: %d", i_nal);
	//return SS_NoErr;

	if(0 >= i_nal) return SS_NoErr;

	unsigned char *p = _stream_buf;
	unsigned char *pp = _stream_buf;

	_last_video_packet_type = NAL_INVALID;
	_video_frame_length = 0;

	unsigned long timestamp = timeGetTime();//同一帧内数据包均采用同样的时戳

	for(int i = 0; i < i_nal; i++)
	{
		//RTSP
		if(true)
		{
			RTSPServerLib::CRTSPServer* _rtsp_server = SINGLETON(CScheduleServer).get_rtsp_server();
			
			if(NULL != _rtsp_server)
			{
				::memset(_rtsp_packet, 0, sizeof(_rtsp_packet));
				::memcpy(_rtsp_packet + sizeof(RTPHeader), pp, _nal_len[i]);

				//TRACE("\n<%d of %d / %d> ", i + 1, i_nal, _nal_len[i]);

				CRTPNATSession::add_rtp_header(_rtsp_packet, _nal_len[i] + sizeof(RTPHeader), 96, ((i == i_nal - 1) ? true : false), _sequence++, 0xFFFF & timestamp, 1);
				
				//RTPHeader* header = reinterpret_cast<RTPHeader*>(_rtsp_packet);
				//TRACE("\n<%d %d %d %d>", header->payloadtype, ntohl(header->ssrc), ntohs(header->sequencenumber), ntohl(header->timestamp));

				//_rtsp_server->input_stream_data(1, _rtsp_packet,  _nal_len[i] + sizeof(RTPHeader), RTSPServerLib::CRTSPServer::kVideoStream);
			
			}
			pp += _nal_len[i];
		}

		int offset = 0;
		NAL_TYPE type = CRTMPSession::get_video_packet_type(p, _nal_len[i], offset);

		if(_last_video_packet_type == type)
		{
			//::memcpy(_video_frame + _video_frame_length, p + offset, _nal_len[i] - offset);
			//_video_frame_length += _nal_len[i] - offset;
			::memcpy(_video_frame + _video_frame_length, p, _nal_len[i]);
			_video_frame_length += _nal_len[i];
		}
		else
		{
			if(_video_frame_length)
			{
				on_recv_frame(_video_frame, _video_frame_length, false);
			}

			::memcpy(_video_frame, p, _nal_len[i]);
			_video_frame_length = _nal_len[i];
		}

		_last_video_packet_type = type;

		p += _nal_len[i];
	}

	if(_video_frame_length)
	{
		on_recv_frame(_video_frame, _video_frame_length, false);
	}

	return SS_NoErr;
}

SS_Error CCaptureScreenTask::on_done()
{
	shutdown_capture();

	_task_info.clear();

	_is_done = true;

	delete _bmp_yuv;
	_bmp_yuv = NULL;

	return SS_NoErr;
}

int CCaptureScreenTask::start_capture()
{
	av_register_all();
	avformat_network_init();

	_format_ctx = avformat_alloc_context();

	avdevice_register_all();

	//Use gdigrab
	AVDictionary* options = NULL;
	//Set some options
	//grabbing frame rate
	//av_dict_set(&options,"framerate","5",0);
	//The distance from the left edge of the screen or desktop
	//av_dict_set(&options,"offset_x","20",0);
	//The distance from the top edge of the screen or desktop
	//av_dict_set(&options,"offset_y","40",0);
	//Video frame size. The default is to capture the full screen
	//av_dict_set(&options,"video_size","640x480",0);
	AVInputFormat *ifmt=av_find_input_format("gdigrab");

	//if(avformat_open_input(&_format_ctx,"desktop",ifmt,&options)!=0)
	//if(avformat_open_input(&_format_ctx,"title=FFmpeg_screen_cut - Microsoft Visual Studio",ifmt,&options)!=0)
	//if(0 != avformat_open_input(&_format_ctx,"title=D:\\FFmpeg", ifmt,&options))
	if(0 != avformat_open_input(&_format_ctx, (string("title=") + _task_info.window_caption).c_str(), ifmt,&options))
	{
		SINGLETON(CScheduleServer).show_log("Couldn't capture: " + _task_info.window_caption);

		if(0 != avformat_open_input(&_format_ctx,"desktop",ifmt,&options)!=0)
		{
			SINGLETON(CScheduleServer).show_log("Couldn't capture desktop!");
			return -1;
		}
		else
		{
			SINGLETON(CScheduleServer).show_log("Capturing: desktop");
		}
	}
	else
	{
		SINGLETON(CScheduleServer).show_log("Capturing: " + _task_info.window_caption);
	}
	

	if(avformat_find_stream_info(_format_ctx,NULL)<0)
	{
		printf("Couldn't find stream information.\n");
		return -2;
	}

	_video_index = -1;
	for(int i=0; i < _format_ctx->nb_streams; i++) 
	{
		if(AVMEDIA_TYPE_VIDEO == _format_ctx->streams[i]->codec->codec_type)
		{
			_video_index = i;
			break;
		}
	}
	
	if(-1 == _video_index)
	{
		printf("Didn't find a video stream.\n");
		return -3;
	}

	AVCodec* pCodec = NULL;

	_codec_ctx = _format_ctx->streams[_video_index]->codec;
	pCodec = avcodec_find_decoder(_codec_ctx->codec_id);

	if(NULL == pCodec)
	{
		printf("Codec not found.\n");
		return -5;
	}

	if(0 > avcodec_open2(_codec_ctx, pCodec, NULL))
	{
		printf("Could not open codec.\n");
		return -6;
	}

	_av_frame = av_frame_alloc();

	_packet = (AVPacket *)av_malloc(sizeof(AVPacket));

	_img_convert_ctx = sws_getContext(_codec_ctx->width, _codec_ctx->height, _codec_ctx->pix_fmt, _task_info.video_width, _task_info.video_height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	//_img_convert_ctx = sws_getContext(_codec_ctx->width, _codec_ctx->height, _codec_ctx->pix_fmt, _task_info.video_width, _task_info.video_height, PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);

	_frame_size = _task_info.video_width * _task_info.video_height;

	_capture_yuv = (unsigned char*)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, _task_info.video_width, _task_info.video_height));

	_yuv_size[0] = _task_info.video_width;
	_yuv_size[1] = _task_info.video_width / 2;
	_yuv_size[2] = _task_info.video_width / 2;
	_yuv_size[3] = 0;

	_yuv_buf[0] = (uint8_t*)av_malloc(_frame_size);
	_yuv_buf[1] = (uint8_t*)av_malloc(_frame_size >> 2);
	_yuv_buf[2] = (uint8_t*)av_malloc(_frame_size >> 2);
	_yuv_buf[3] = NULL;

	return 0;
}

void CCaptureScreenTask::shutdown_capture()
{
	avcodec_close(_codec_ctx);
	avformat_close_input(&_format_ctx);
	
	av_free_packet(_packet);
	av_free(_av_frame);
	
	av_free(_yuv_buf[0]);
	av_free(_yuv_buf[1]);
	av_free(_yuv_buf[2]);
	av_free(_capture_yuv);
}

int CCaptureScreenTask::capture()
{
	unsigned long t = clock();

	int got_picture = 0;

	//------------------------------
	if(0 <= av_read_frame(_format_ctx, _packet))
	{
		if(_video_index == _packet->stream_index)
		{
			if(0 > avcodec_decode_video2(_codec_ctx, _av_frame, &got_picture, _packet))
			{
				printf("Decode Error.\n");
				return -1;
			}
			
			if(got_picture)
			{
				clock_t t = clock();

				sws_scale(_img_convert_ctx,
						(const uint8_t* const*)_av_frame->data, _av_frame->linesize, 0, _codec_ctx->height,
						_yuv_buf, _yuv_size);

				memcpy(_capture_yuv, _yuv_buf[0], _frame_size);
				memcpy(_capture_yuv + _frame_size, _yuv_buf[1], _frame_size >> 2);
				memcpy(_capture_yuv + (_frame_size * 5 >> 2), _yuv_buf[2], _frame_size>>2);

				//TRACE("\ncapture: %d", clock() - t);

				//fwrite(out, 1, write_size, fp_yuv);
			}
		}
		
		av_free_packet(_packet);
	}
	
	//printf("\ncapture: %d", clock() - t);
}
