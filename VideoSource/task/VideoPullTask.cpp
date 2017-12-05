// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2011-01 ~ 2011-11
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#include "VideoPullTask.h"
#include "MiscTool.h"
#include "TimeConsuming.h"

using namespace ScheduleServer;

//int32_t CVideoPullTask::_sdk_handler = -1;
//bool CVideoPullTask::_sdk_initialized = false;
//CSSMutex CVideoPullTask::_sdk_init_mutex;

CVideoPullTask::CVideoPullTask(SDK_RECV_TASK_INFO& task_info):
CDataRecvTask(task_info),
_action_offset(0)
{
	_yuv_file = NULL;

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

int CVideoPullTask::init()
{
	_yuv_file = fopen(_task_info.data_url.c_str(), "rb");

	if(NULL == _yuv_file) return -1;

	fseek(_yuv_file, 0, SEEK_SET);
	
	_next_fetch_video_frame_timestamp = 0;

	_frame_interval = CLOCKS_PER_SEC / _task_info.fps;

	int bitrate = SINGLETON(CConfigBox).get_property_as_int("VideoBitrate", 1024);
	//_x264_handle = H264EncodeInit(_task_info.video_width, _task_info.video_height, 30, 1000, 30, 1024, _task_info.fps);
	_x264_handle = H264EncodeInit(_task_info.video_width, _task_info.video_height, 30, 1000, 30, bitrate, _task_info.fps);

	_last_video_packet_type = NAL_INVALID;
	_video_frame_length = 0;

	return 0;
}

int CVideoPullTask::connect_device()
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

int CVideoPullTask::disconnect_device()
{
	return 0;
}

void CVideoPullTask::left()
{
	_action_offset = 30;
}

void CVideoPullTask::right()
{
	_action_offset = 50;
}

void CVideoPullTask::up()
{
	_action_offset = 40;
}

void CVideoPullTask::down()
{
	_action_offset = 30;
}

void CVideoPullTask::zoom_in()
{
	_action_offset = 30;
}

void CVideoPullTask::zoom_out()
{
	_action_offset = 30;
}

SS_Error CVideoPullTask::run()
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

	if(_action_offset)
	{
		//fseek(_yuv_file, _action_offset * _task_info.video_width * _task_info.video_height * 3 / 2, SEEK_CUR);
		if(fseek(_yuv_file, 150 * _task_info.video_width * _task_info.video_height * 3 / 2, SEEK_CUR))
		{
			fseek(_yuv_file, 0, SEEK_SET);
			return SS_NoErr;
		}
		_action_offset = 0;
	}	

	if(feof(_yuv_file))
	{
		fseek(_yuv_file, 0, SEEK_SET);
		return SS_NoErr;
	}

	int length = _task_info.video_width * _task_info.video_height * 3 / 2;

	::memset(_yuv, 0, sizeof(_yuv));

	if(length != fread(_yuv, sizeof(unsigned char), length, _yuv_file))
	{
		fseek(_yuv_file, 0, SEEK_SET);
		return SS_NoErr;
	}

	memset(_stream_buf, 0, sizeof(_stream_buf));

	//encode
	int i_nal = H264EncodeFrame(_x264_handle, -1, _yuv, _stream_buf, _nal_len);

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

SS_Error CVideoPullTask::on_done()
{
	_task_info.clear();

	_is_done = true;

	return SS_NoErr;
}

