// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2011-01 ~ 2011-11
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#ifndef _CAPTURE_SCREEN_TASK_H_
#define _CAPTURE_SCREEN_TASK_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "GeneralDef.h"
#include "DataRecvTask.h"
#include "time.h"
#include "ScheduleServer.h"

namespace ScheduleServer
{
	class CCaptureScreenTask : public CDataRecvTask
	{
	public:
		CCaptureScreenTask(SDK_RECV_TASK_INFO& task_info);

		virtual SS_Error run();

	protected:
		virtual int connect_device();
		virtual int disconnect_device();
		virtual SS_Error on_done();

	private:
		//static int32_t _sdk_handler;
		//static bool _sdk_initialized;
		//static CSSMutex _sdk_init_mutex;

		int init();

		unsigned char _frame[65536 * 10];
		NAL_TYPE _last_frame_type;
		int _frame_pos;

	private:
		//void on_recv_packet(unsigned char* data, int len);

	private:
		string _ip;
		string _port;
		string _username;
		string _password;
		string _camera_id;
		string _channel;

		int _seq;

	private:
		int _screen_height;
		int _screen_width;
		//FILE* _yuv_file;
		//bool _start_write;
		unsigned long _next_fetch_video_frame_timestamp;
		unsigned char* _yuv;//屏幕YUV
		unsigned char* _yuv2;//下采样YUV
		unsigned long _frame_interval;
		//bool _got_sps_pps;

	private:
		long _x264_handle;//定义一路编码器
		unsigned char _stream_buf[6553600];//编码后一帧码流的缓存
		int _nal_len[1024]; //定义数据包尺寸存储数组,注意数组的尺寸不能太小

		NAL_TYPE _last_video_packet_type;
		unsigned char _video_frame[65536 * 100];
		int _video_frame_length;

	private:
		void capture_screen();

	private:
		unsigned long _action_offset;

	public:
		void left();
		void right();
		void up();
		void down();
		void zoom_in();
		void zoom_out();
	};
}

#endif  //_CAPTURE_SCREEN_TASK_H_      
