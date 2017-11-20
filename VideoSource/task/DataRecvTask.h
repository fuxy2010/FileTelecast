// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2011-01 ~ 2011-11
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#ifndef _SDK_RECV_TASK_H_
#define _SDK_RECV_TASK_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "GeneralDef.h"
#include "Task.h"
#include "time.h"
#include "ScheduleServer.h"

namespace ScheduleServer
{
	typedef enum
	{
		DataRecvTaskBegin = 0,
		DataRecvTask_Connected,
		DataRecvTask_RECV,
		DataRecvTask_Done,
		DataRecvTask_Pause
	}
	SDK_RECV_TASK_STATUS;

	typedef struct tagSDK_RECV_TASK_INFO
	{
		unsigned long	task_id;//����ID
		unsigned long	ua_id;
		unsigned long	create_timestamp;
		string			data_url;
		unsigned short	fps;
		unsigned short	video_width;
		unsigned short	video_height;
		string			window_caption;

		tagSDK_RECV_TASK_INFO() : task_id(0), ua_id(0), create_timestamp(::time(NULL)), fps(25), video_width(1280), video_height(720)
		{
			data_url = "";
			window_caption = "";

		}

		~tagSDK_RECV_TASK_INFO()
		{
		}

		void clear()
		{

		}
	}
	SDK_RECV_TASK_INFO;

	class CDataRecvTask : public CTask
	{
	public:
		CDataRecvTask(SDK_RECV_TASK_INFO& task_info);
		virtual ~CDataRecvTask();

		virtual SS_Error run();

	protected:
		//virtual SS_Error on_done();

		virtual SS_Error on_exception();

	public:
		//int init();

	protected:
		/*volatile */SDK_RECV_TASK_STATUS _status;

		SDK_RECV_TASK_INFO _task_info;
		
	protected:
		bool _start_push;
		bool _start_recv;
		//unsigned long _last_frame_timestamp;

		unsigned long _last_packet_timestamp;

		unsigned char _video_frame[65536 * 10];
		NAL_TYPE _last_video_type;
		int _video_frame_length;

		void add_video_frame(unsigned char* frame, int length, unsigned long timestamp)
		{
			if(length && NULL != _ua)
			{
				int offset = 0;
				NAL_TYPE type = CRTMPSession::get_video_packet_type(frame, length, offset);
				if(NAL_INVALID == type)
					return;

				VIDEO_PACKET_PTR packet_ptr;

				CMemPool::malloc_video_packet(packet_ptr);

				packet_ptr.packet->ua_id = _task_info.ua_id;
				packet_ptr.packet->sequence = 0;
				packet_ptr.packet->timestamp = timestamp;
				packet_ptr.packet->payload_size = length;//���ɳ���
				packet_ptr.packet->mark = true;
				memcpy(packet_ptr.packet->payload, frame, length);

				//TRACE("\nP1 %d", _video_frame_length);
				if(SS_NoErr != _ua->add_sample_video_packet(packet_ptr))//for sps and pps
				{
					CMemPool::free_video_packet(packet_ptr);
				}
			}
		}

		CUserAgent* _ua;

	protected:
		virtual int connect_device() = 0;
		virtual int disconnect_device() = 0;
		//virtual void recv_frame() = 0;

	protected:
		bool _initialized;

	public:
		bool is_initialized() { return _initialized; }

		void shutdown() { _status = DataRecvTask_Done; }

		void pause();

		void restart(string& url);

		void on_recv_frame(unsigned char* data, int length, bool sps_pps_sei_idr);

	private:
		HANDLE _wait_for_pause_handle;

	private:
		//static map<DWORD, unsigned long> _latest_packet_timestamp;
		//static int rtsp_interrupt_callback(void* ctx);

	public:
		static bool _hik_platform_sdk_initialized;
	};
}

#endif  //_SDK_RECV_TASK_H_      
