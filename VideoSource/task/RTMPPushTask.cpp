// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2011-01 ~ 2011-11
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#include "RTMPPushTask.h"
#include "MiscTool.h"
#include "TimeConsuming.h"

using namespace ScheduleServer;

CRTMPPushTask::CRTMPPushTask(RTMP_PUSH_TASK_INFO& task_info) :
_status(RTMPPushTask_Begin),
_ua(NULL),
//_rtsp_server(NULL),
_initialized(false)
{
	_task_info = task_info;

	if(!init_rtmp())
	{
		_status = RTMPPushTask_Connected;
		_initialized = true;
	}
	else
	{
		_status = RTMPPushTask_Done;
		_initialized = false;
	}

}

CRTMPPushTask::~CRTMPPushTask()
{
	_task_info.clear();
}

int CRTMPPushTask::init_rtmp()
{
	int ret = -1;

	ret = _rtmp_session.connect(const_cast<char*>(_task_info.rtmp_url.c_str()));
	//ret = _rtmp_session.connect("rtmp://121.41.15.6:2935/hls/test");
	//ret = _rtmp_session.connect("rtmp://127.0.0.1/hls/test");
	if(0 != ret)
	{
		return ret;
	}

	SINGLETON(CScheduleServer).reg_ua(_task_info.ua_id, UA_MobilePhone);
	_ua = SINGLETON(CScheduleServer).fetch_ua(_task_info.ua_id);

	if(NULL == _ua)
		ret = -2;

	return ret;
}

unsigned long t = 0;
void CRTMPPushTask::push()
{
	//if(3000 > timeGetTime() - _start_time) return;

	if(NULL == _ua)
		return;

	VIDEO_PACKET_PTR packet_ptr = _ua->fetch_sample_video_packet();

	if(NULL == packet_ptr.packet || !packet_ptr.packet->payload_size)
	{
		CMemPool::free_video_packet(packet_ptr);
		return;
	}

	int ret = -1;
	{
		ret = _rtmp_session.input_video_packet(packet_ptr.packet->payload, packet_ptr.packet->payload_size, packet_ptr.packet->timestamp);

		if(404 != ret)
		{
			TRACE("\nV<%d %d %d> @ %d", packet_ptr.packet->payload_size, ret, timeGetTime() - t, packet_ptr.packet->timestamp);//CRTMPSession::get_packet_type(p, _nal_len[i], seprator_len));
			t = timeGetTime();
		}
		else
		{
			TRACE("\nE<%d %d> ---------------------------", packet_ptr.packet->payload_size, ret);//CRTMPSession::get_packet_type(p, _nal_len[i], seprator_len));
			
			_rtmp_session.disconnect();

			Sleep(60);

			_rtmp_session.connect(const_cast<char*>(_task_info.rtmp_url.c_str()));
		}

	}

	CMemPool::free_video_packet(packet_ptr);

	//return ret;
}

SS_Error CRTMPPushTask::run()
{
	CTimeConsuming tc('A', 10.0);

	//TRACE("\nUR<%d %d> ", timeGetTime() - _start_time, _expire);

	if(RTMPPushTask_Done == _status)
	{
		on_done();
		return SS_NoErr;
	}

	switch(_status)
	{
	case RTMPPushTask_Begin:
		_status = (!init_rtmp()) ? RTMPPushTask_Connected : RTMPPushTask_Done;
		break;

	case RTMPPushTask_Connected:
		_status = RTMPPushTask_PUSH;
		break;

	case RTMPPushTask_PUSH:
		push();
		break;

	case RTMPPushTask_Done:
		on_done();
		break;
	}

	return SS_NoErr;

}

SS_Error CRTMPPushTask::on_done()
{
	_task_info.clear();

	_rtmp_session.disconnect();

	_is_done = true;

	return SS_NoErr;
}

SS_Error CRTMPPushTask::on_exception()
{
	return SS_NoErr;
}
