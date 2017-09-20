// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2011-01 ~ 2011-11
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#include "DataRecvTask.h"
#include "MiscTool.h"
#include "TimeConsuming.h"

/*#include "./dpsdk/DPSDK_Core.h"
#include "./dpsdk/DPSDK_Ext.h"
#include "./dpsdk/DPSDK_Core_Error.h"

// ��ƽ̨SDK
#pragma comment(lib, "../dev/dp/dpsdk/DPSDK_Core.lib")
#pragma comment(lib, "../dev/dp/dpsdk/DPSDK_Ext.lib")
#pragma comment(lib, "WS2_32.lib")*/
//#include "DPStreamAccessPlugin.h"

using namespace ScheduleServer;

bool CDataRecvTask::_hik_platform_sdk_initialized = false;

CDataRecvTask::CDataRecvTask(SDK_RECV_TASK_INFO& task_info) :
_status(DataRecvTaskBegin),
_start_push(false),
_start_recv(false),
_last_packet_timestamp(0),
_ua(NULL),
_initialized(false),
_wait_for_pause_handle(::CreateEvent(NULL, TRUE/*���Զ���λ*/, FALSE/*��ʼ״̬*/, NULL))
{
	_task_info = task_info;

	_status = DataRecvTaskBegin;
}

CDataRecvTask::~CDataRecvTask()
{
	_task_info.clear();

	::CloseHandle(_wait_for_pause_handle);
}

void CDataRecvTask::pause()
{
	::ResetEvent(_wait_for_pause_handle);

	_status = DataRecvTask_Pause;
}

void CDataRecvTask::restart(string& url)
{
	::WaitForSingleObject(_wait_for_pause_handle, 5000);

	_start_push = false;
	_start_recv = false;
	_initialized = false;

	disconnect_device();

	_task_info.data_url = url;

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
}

void CDataRecvTask::on_recv_frame(unsigned char* data, int length, bool sps_pps_sei_idr)
{
	int offset = 0;
	//TRACE("\n<P %d %d>", CRTMPSession::get_video_packet_type(data, length, offset), length);
	//_last_packet_timestamp = timeGetTime();
	//return;

	if(false)
	{
		FILE* f = fopen("h264.dat", "ab+");
		fwrite(data, 1, length, f);
		fclose(f);
		return;
	}

	if(true == sps_pps_sei_idr)
	{
		unsigned char* dat = data;
		//int length = packet->size;

		int offset = 0;
		NAL_TYPE type =  CRTMPSession::get_video_packet_type(dat, length, offset);
		unsigned long timestamp = timeGetTime();

		if(NAL_SPS == type)
		{
			timestamp = timeGetTime();			

			//sps
			int sps_pos = 0;
			int pps_pos = 0;
			int sei_pos = 0;
			int idr_pos = 0;

			CRTMPSession::find_frame_header4(dat + sps_pos + 4, length - sps_pos - 4, offset);
			pps_pos = sps_pos + offset + 4;

			CRTMPSession::find_frame_header4(dat + pps_pos + 4, length - pps_pos- 4, offset);
			sei_pos = pps_pos + offset + 4;

			CRTMPSession::find_frame_header4(dat + sei_pos + 4, length - sei_pos- 4, offset);
			idr_pos = sei_pos + offset + 4;

			//unsigned long t = timeGetTime();

			//sps
			_video_frame_length = pps_pos - sps_pos;
			memcpy(_video_frame, dat + sps_pos, _video_frame_length);
			add_video_frame(_video_frame, _video_frame_length, timestamp);
			//TRACE("\n<B %d %d", type, ua->_video_frame_length);
			_video_frame_length = 0;

			//pps
			_video_frame_length = sei_pos - pps_pos;
			memcpy(_video_frame, dat + pps_pos, _video_frame_length);
			add_video_frame(_video_frame, _video_frame_length, timestamp);
			//TRACE("\n<C %d %d", type, ua->_video_frame_length);
			_video_frame_length = 0;

			//sei
			_video_frame_length = idr_pos - sei_pos;
			memcpy(_video_frame, dat + sei_pos, _video_frame_length);
			add_video_frame(_video_frame, _video_frame_length, timestamp);
			//TRACE("\n<D %d %d", type, ua->_video_frame_length);
			_video_frame_length = 0;

			timestamp = timeGetTime();

			//idr
			_video_frame_length = length - idr_pos;
			memcpy(_video_frame, dat + idr_pos, _video_frame_length);
			add_video_frame(_video_frame, _video_frame_length, timestamp);
			//TRACE("\n<E %d %d", type, ua->_video_frame_length);
			_video_frame_length = 0;

			_last_video_type = NAL_SLICE_IDR;

		}
		else if(NAL_SLICE == type)
		{
			//p
			_video_frame_length = length;
			memcpy(_video_frame, dat, _video_frame_length);
			add_video_frame(_video_frame, _video_frame_length, timestamp);
			//TRACE("\n<G %d %d", type, ua->_video_frame_length);
			_video_frame_length = 0;

			_last_video_type = NAL_SLICE;
		}
	}
	else
	{
		unsigned char* dat = data;
		//int length = packet->size;

		int offset = 0;
		NAL_TYPE type =  CRTMPSession::get_video_packet_type(dat, length, offset);
		unsigned long timestamp = timeGetTime();

		if(NAL_INVALID != type)
		{
			_video_frame_length = length;
			memcpy(_video_frame, dat, _video_frame_length);
			add_video_frame(_video_frame, _video_frame_length, timestamp);
			_video_frame_length = 0;

			_last_video_type = type;
		}
	}
}

SS_Error CDataRecvTask::run()
{
	CTimeConsuming tc('A', 10.0);

	//TRACE("\nUS<%d %d> ", timeGetTime() - _start_time, _expire);

	if(_last_packet_timestamp && 10 * CLOCKS_PER_SEC < (timeGetTime() - _last_packet_timestamp))
	{
		//SINGLETON(CScheduleServer).shutdown_cast(_task_info.ua_id);
	}

	if (DataRecvTask_Done == _status) return SS_RecvData;

	switch(_status)
	{
	case DataRecvTaskBegin:
		_status = (!connect_device()) ? DataRecvTask_Connected : DataRecvTask_Connected;
		break;

	case DataRecvTask_Connected:
		_status = DataRecvTask_RECV;
		break;

	case DataRecvTask_RECV:
		return SS_RecvData;
		break;

	case DataRecvTask_Pause:
		::SetEvent(_wait_for_pause_handle);
		break;

	case DataRecvTask_Done:
		on_done();
		break;
	}

	return SS_NoErr;

}

SS_Error CDataRecvTask::on_exception()
{
	return SS_NoErr;
}
