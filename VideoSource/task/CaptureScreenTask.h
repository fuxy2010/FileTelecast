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

#ifdef __cplusplus
extern "C"
{
#endif
#include "libavformat\avformat.h"
#include "libavdevice\avdevice.h"
#include "libswscale\swscale.h"
//#include "SDL/SDL.h"
#ifdef __cplusplus
};
#endif

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
		//unsigned char* _yuv;//屏幕YUV
		//unsigned char* _yuv2;//下采样YUV
		unsigned long _next_fetch_video_frame_timestamp;
		unsigned long _frame_interval;

	private:
		long _x264_handle;//定义一路编码器
		unsigned char _stream_buf[6553600];//编码后一帧码流的缓存
		int _nal_len[1024]; //定义数据包尺寸存储数组,注意数组的尺寸不能太小

		NAL_TYPE _last_video_packet_type;
		unsigned char _video_frame[65536 * 100];
		int _video_frame_length;

	private:
		void capture_screen_by_win();

	private:
		AVFormatContext*	_format_ctx;
		AVCodecContext*		_codec_ctx;
		int					_video_index;
		AVFrame*			_av_frame;
		AVPacket*			_packet;
		struct SwsContext*	_img_convert_ctx;
		unsigned char*		_capture_yuv;

		uint8_t*	_yuv_buf[4];
		int			_yuv_size[4];
		size_t		_frame_size;

		void capture_screen_by_ffmpeg();
		void screen_to_bgr(int left, int top, int right, int bottom, unsigned char* bgr);
		//void screen_to_yuv();

		int start_capture();
		void shutdown_capture();
		int capture();

	private:
		unsigned char* _bmp_yuv;

	public:
		//virtual void shutdown() { _status = RTMPPushTask_Done; }

	public:
		/*void left();
		void right();
		void up();
		void down();
		void zoom_in();
		void zoom_out();*/

	private:
		unsigned char _rtsp_packet[1536];
		unsigned short _sequence;

	private:
		bool _window_exist;
		bool check_window();
	};
}

#endif  //_CAPTURE_SCREEN_TASK_H_      
