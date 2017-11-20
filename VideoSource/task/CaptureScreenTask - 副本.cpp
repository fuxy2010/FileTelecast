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

using namespace ScheduleServer;

//int32_t CCaptureScreenTask::_sdk_handler = -1;
//bool CCaptureScreenTask::_sdk_initialized = false;
//CSSMutex CCaptureScreenTask::_sdk_init_mutex;

CCaptureScreenTask::CCaptureScreenTask(SDK_RECV_TASK_INFO& task_info):
CDataRecvTask(task_info),
_action_offset(0)
{
	if(!connect_device())
	{
		_status = DataRecvTask_Connected;
		_initialized = true;
	}
	else
	{
		_status = DataRecvTask_Done;
		_initialized = false;
	}

	//_got_sps_pps = false;
}

int CCaptureScreenTask::init()
{
	_screen_width = GetSystemMetrics(SM_CXSCREEN);
	_screen_height = GetSystemMetrics(SM_CYSCREEN);

	_screen_width -= _screen_width % 8;
	_screen_height -= _screen_height % 8;

	//创建编码器
	::memset(_stream_buf, 0, sizeof(_stream_buf));
	::memset(_nal_len, 0, sizeof(_nal_len));

	_yuv = new unsigned char[_screen_width * _screen_height * 3 / 2];
	_yuv2 = new unsigned char[_task_info.video_width * _task_info.video_height * 3 / 2];

	_next_fetch_video_frame_timestamp = 0;

	_frame_interval = CLOCKS_PER_SEC / _task_info.fps;

	//_x264_handle = H264EncodeInit(_task_info.video_width, _task_info.video_height, 30, 1000, 30, 1024, _task_info.fps);
	_x264_handle = H264EncodeInit(_task_info.video_width, _task_info.video_height, 30, 1000, 30, 2048, _task_info.fps);

	_last_video_packet_type = NAL_INVALID;
	_video_frame_length = 0;

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

void CCaptureScreenTask::left()
{
	_action_offset = 30;
}

void CCaptureScreenTask::right()
{
	_action_offset = 50;
}

void CCaptureScreenTask::up()
{
	_action_offset = 40;
}

void CCaptureScreenTask::down()
{
	_action_offset = 30;
}

void CCaptureScreenTask::zoom_in()
{
	_action_offset = 30;
}

void CCaptureScreenTask::zoom_out()
{
	_action_offset = 30;
}

SS_Error CCaptureScreenTask::run()
{
	if (SS_RecvData != CDataRecvTask::run()) return SS_NoErr;

	if(!_next_fetch_video_frame_timestamp)
		_next_fetch_video_frame_timestamp = timeGetTime();

	if(_next_fetch_video_frame_timestamp > timeGetTime())
	{
		//TRACE("\n<Wait %d -> %d", _next_fetch_video_frame_timestamp, timeGetTime());
		return SS_NoErr;
	}

	//TRACE("\nRUN");

	_next_fetch_video_frame_timestamp += _frame_interval;


	unsigned long t1 = timeGetTime();
	capture_screen();
	unsigned long t2 = timeGetTime();
	CSizescale::scale(_yuv2, _task_info.video_width, _task_info.video_height, _yuv, _screen_width, _screen_height);
	TRACE("\nScreen: %d, %d", t2 - t1, timeGetTime() - t2);

	if(false)
	{
		FILE* f = fopen("screen.yuv", "ab+");
		fwrite(_yuv2, 1, _task_info.video_width * _task_info.video_height * 3 / 2, f);
		fclose(f);
	}

	memset(_stream_buf, 0, sizeof(_stream_buf));

	//encode
	int i_nal = H264EncodeFrame(_x264_handle, -1, _yuv2, _stream_buf, _nal_len);

	//TRACE("\nnal: %d", i_nal);
	//return SS_NoErr;

	if(0 >= i_nal) return SS_NoErr;

	unsigned char *p = _stream_buf;

	_last_video_packet_type = NAL_INVALID;
	_video_frame_length = 0;

	for(int i = 0; i < i_nal; i++)
	{
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
	_task_info.clear();

	_is_done = true;

	return SS_NoErr;
}

void CCaptureScreenTask::capture_screen()
{
	unsigned long t = timeGetTime();
	BITMAPINFO pbi;
	PRGBTRIPLE scan0;
	int stride;
	int nScreenWidth = _screen_width;//GetSystemMetrics(SM_CXSCREEN);
	int nScreenHeight = _screen_height;//GetSystemMetrics(SM_CYSCREEN);
	HWND hDesktopWnd = ::GetDesktopWindow();
	HDC hDesktopDC = ::GetDC(hDesktopWnd);
	HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);

	//TRACE("\nCAP0: %d", timeGetTime() - t);

	HBITMAP hCaptureBitmap = CreateCompatibleBitmap(hDesktopDC,nScreenWidth, nScreenHeight);
	SelectObject(hCaptureDC,hCaptureBitmap);
	BitBlt(hCaptureDC,0,0,nScreenWidth,nScreenHeight,hDesktopDC,0,0,SRCCOPY|CAPTUREBLT);

	//TRACE("\nCAP1: %d", timeGetTime() - t);
	
	//	SaveCapturedBitmap(hCaptureBitmap);
	DeleteDC(hCaptureDC);
	pbi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbi.bmiHeader.biWidth = nScreenWidth;
	pbi.bmiHeader.biHeight = nScreenHeight;
	pbi.bmiHeader.biPlanes = 1;
	pbi.bmiHeader.biBitCount = 24;
	pbi.bmiHeader.biCompression = BI_RGB;
	stride = ((nScreenWidth * 24 + 31) & 0xffffffe0) / 8;  // 24位图像扫描线宽度
	scan0 = (PRGBTRIPLE)malloc(stride * nScreenHeight);    // 图像数据缓冲区，应释放
	GetDIBits(hDesktopDC, hCaptureBitmap, 0, nScreenHeight, scan0, &pbi, DIB_RGB_COLORS);
	::ReleaseDC(hDesktopWnd, hDesktopDC);
	DeleteObject(hCaptureBitmap);

	//TRACE("\nCAP: %d", timeGetTime() - t);

	//BMP翻转
	t = timeGetTime();
	{
		int half_width = _screen_width >> 1;
		PRGBTRIPLE row_head = scan0;
		BYTE blue;
		BYTE green;
		BYTE red;
		for(int i = 0; i < _screen_height; ++i)
		{
			PRGBTRIPLE temp = row_head;

			for(int j = 0; j < half_width; ++j)
			{
				//*(temp + j) = *(row_head + _video_width - j - 1);
				blue = (temp + j)->rgbtBlue;
				green = (temp + j)->rgbtGreen;
				red = (temp + j)->rgbtRed;

				(temp + j)->rgbtBlue = (row_head + _screen_width - j - 1)->rgbtBlue;
				(temp + j)->rgbtGreen = (row_head + _screen_width - j - 1)->rgbtGreen;
				(temp + j)->rgbtRed = (row_head + _screen_width - j - 1)->rgbtRed;

				(row_head + _screen_width - j - 1)->rgbtBlue = blue;
				(row_head + _screen_width - j - 1)->rgbtGreen = green;
				(row_head + _screen_width - j - 1)->rgbtRed = red;
			}

			row_head += _screen_width;
		}
	}
	//TRACE("\nBMP: %d", timeGetTime() - t);

	//BMP->YUV420
	t = timeGetTime();
	{
		unsigned char *y = _yuv;
		unsigned char *u = _yuv + _screen_width * _screen_height;
		unsigned char *v = _yuv + _screen_width * _screen_height * 5 / 4;
		CTranscoding::RGBtoYUV(_screen_width, _screen_height, reinterpret_cast<unsigned char*>(scan0), y, u, v);
		CTranscoding::matrxCovert(_screen_width * _screen_height,   y);
		CTranscoding::matrxCovert(_screen_width * _screen_height/4, u);
		CTranscoding::matrxCovert(_screen_width * _screen_height/4, v);

		if(false)
		{
			FILE* f = fopen("screen.yuv", "ab+");
			fwrite(_yuv, 1, _screen_width * _screen_height * 3 / 2, f);
			fclose(f);
		}
#if 0
		//video encoding
		//编码
		int i_nal = H264EncodeFrame(_x264_handle, -1, _yuv_buf, _frame_buf, _nal_len);
		if(0 >= i_nal)
		{
			free(scan0);
			return;
		}

		//input into rtsp server
		unsigned long timestamp = timeGetTime();//同一帧内数据包均采用同样的时戳
		unsigned char *p = _frame_buf;
		for(int i = 0; i < i_nal; i++)
		{
			::memset(_packet, 0, sizeof(_packet));
			::memcpy(_packet + sizeof(RTPHeader), p, _nal_len[i]);

			//printf("<%d of %d / %d> ", i + 1, i_nal, _nal_len[i]);

			add_rtp_header(_packet, _nal_len[i] + sizeof(RTPHeader), 96, ((i == i_nal - 1) ? true : false), _sequence++, 0xFFFF & timestamp, 1);
			_rtsp_server->input_stream_data(1, _packet,  _nal_len[i] + sizeof(RTPHeader), CRTSPServer::kVideoStream);

			p += _nal_len[i];
		}
#endif
	}
	//TRACE("\nYUV: %d", timeGetTime() - t);
	free(scan0);
}

