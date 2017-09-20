// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2011-01 ~ 2011-11
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#include "UserAgent.h"
#include "MiscTool.h"
#include "JRTPSession.h"
#include "ScheduleServer.h"
#include "resource.h"
#include "winbase.h"

using namespace ScheduleServer;

const unsigned long CUserAgent::_max_audio_packet_num = 100;//��Ƶ���ݰ�������󳤶ȣ���Ƶ������ʱ�䲻���ȹʼӴ��ֵ
const unsigned long CUserAgent::_max_video_packet_num = 100;//��Ƶ���ݰ�������󳤶�

int CUserAgent::_VIDEO_WIDTH = 640;
int CUserAgent::_VIDEO_HEIGHT = 480;
int CUserAgent::_VIDEO_SAMPLE_INTERVAL = 15;

CSSMutex CUserAgent::_rtmp_send_mutex;

CUserAgent::CUserAgent(USER_AGENT_INFO& info) :
_start_hls(false),
_audio_codec(NULL),
_first_video_packet(true),
_first_video_timestamp(0),
_first_audio_packet(true),
_first_audio_timestamp(0),
_next_fetch_video_frame_timestamp(0)
{
	_info = info;

	remove_all_audio_packet();
	remove_all_video_packet();

	_audio_codec = new CWUACodec();

	//�������������´�������Ϊ�����豸����Ƶ�ɼ��ķֱ��ʿ��ܻ�ı�
	_x264_handle = 0;//H264EncodeInit(_VIDEO_WIDTH, _VIDEO_HEIGHT, 30, 1000, 30, 384, 1000 / _VIDEO_SAMPLE_INTERVAL);

	if(1 > _x264_handle)
	{
		//MessageBox("����������ʧ��","��ʾ",MB_ICONQUESTION|MB_OK);
		//return;
	}

	//_last_video_packet_type = NAL_INVALID;
	//_video_frame_length = 0;

#ifdef PUSH_VIDEO_FILE
	_push_video_file = fopen(VIDEO_FILE, "rb");

	if(NULL != _push_video_file)
		fseek(_push_video_file, 0, SEEK_SET);
#endif

}

CUserAgent::~CUserAgent()
{
	//H264EncodeClose(_x264_handle);

	delete _audio_codec;
	_audio_codec = NULL;

	remove_all_audio_packet();
	remove_all_video_packet();
}

SS_Error CUserAgent::remove_all_audio_packet()
{
	{
		CSSLocker lock(&_sample_audio_packet_list_mutex);

		for(std::list<AUDIO_PACKET_PTR>::iterator iter = _sample_audio_packet_list.begin();
			iter != _sample_audio_packet_list.end();
			++iter)
		{
			CMemPool::free_audio_packet(*iter);
		}

		_sample_audio_packet_list.clear();
	}

	return SS_NoErr;
}

SS_Error CUserAgent::remove_all_video_packet()
{
	{
		CSSLocker lock(&_sample_video_packet_list_mutex);

		for(std::list<VIDEO_PACKET_PTR>::iterator iter = _sample_video_packet_list.begin();
			iter != _sample_video_packet_list.end();
			++iter)
		{
			CMemPool::free_video_packet(*iter);
		}

		_sample_video_packet_list.clear();
	}

	return SS_NoErr;
}

SS_Error CUserAgent::add_sample_audio_packet(const char* frame, const unsigned long& length)
{
	int packet_len = _audio_codec->encode(reinterpret_cast<short*>(const_cast<char*>(frame)), _aac_packet);

	//TRACE("\n%d[%x, %x, %x, %x, %x, %x, %x, %x] > ", packet_len, _aac_packet[0], _aac_packet[1], _aac_packet[2], _aac_packet[3], _aac_packet[4], _aac_packet[5], _aac_packet[6], _aac_packet[7]);
	//return SS_NoErr;

	if(0 >= packet_len)
		return SS_NoErr;

	AUDIO_PACKET_PTR packet_ptr;
	CMemPool::malloc_audio_packet(packet_ptr);

	packet_ptr.packet->energy = 1;

	if(true == _first_audio_packet)
	{
		_first_audio_timestamp = timeGetTime();
		_first_audio_packet = false;

		packet_ptr.packet->sequence = 0xffff;
	}
	else
	{
		packet_ptr.packet->sequence = 0;
	}

	packet_ptr.packet->timestamp = ::timeGetTime() - _first_audio_timestamp;
	packet_ptr.packet->ua_id = 1;
	::memcpy(packet_ptr.packet->payload, _aac_packet, packet_len);
	packet_ptr.packet->payload_size = packet_len;

	{
		CSSLocker lock(&_sample_audio_packet_list_mutex);

		if(_max_audio_packet_num < _sample_audio_packet_list.size())
		{
			CMemPool::free_audio_packet(_sample_audio_packet_list.front());
			_sample_audio_packet_list.pop_front();
		}

		_sample_audio_packet_list.push_back(packet_ptr);
	}

	return SS_NoErr;
}

AUDIO_PACKET_PTR CUserAgent::fetch_sample_audio_packet()
{
	{
		CSSLocker lock(&_sample_audio_packet_list_mutex);

		//Ϊƽ���ƶ�������3�����ϵİ���ȡ if(false == _raw_audio_frame_list.empty())
		if(1 <= _sample_audio_packet_list.size())
		{
			AUDIO_PACKET_PTR packet_ptr = _sample_audio_packet_list.front();//*(_audio_packet_list.begin());
			_sample_audio_packet_list.pop_front();

			//TRACE("<FA 3> ");

			return packet_ptr;
		}
	}

	AUDIO_PACKET_PTR packet_ptr;
	
	return packet_ptr;
}

/*int CUserAgent::input_video_packet(unsigned char* packet, unsigned long length)
{
	if(true == _first_video_packet)
	{
		_first_video_timestamp = timeGetTime();
		_first_video_packet = false;
	}

	unsigned long timestamp = timeGetTime() - _first_video_timestamp;//ͬһ֡�����ݰ�������ͬ����ʱ��

	if(length)
	{
		VIDEO_PACKET_PTR packet_ptr;

		CMemPool::malloc_video_packet(packet_ptr);

		packet_ptr.packet->ua_id = _info.id;
		packet_ptr.packet->sequence = 0;
		packet_ptr.packet->timestamp = timestamp;
		packet_ptr.packet->payload_size = length;//���ɳ���
		packet_ptr.packet->mark = true;
		memcpy(packet_ptr.packet->payload, packet, length);

		//TRACE("\nP1 %d", _video_frame_length);
		if(SS_NoErr != add_sample_video_packet(packet_ptr))//for sps and pps
		{
			CMemPool::free_video_packet(packet_ptr);
		}
	}

	return 0;
}*/

SS_Error CUserAgent::add_sample_video_packet(VIDEO_PACKET_PTR packet)
{
	//�������ǰ�������жϷ������򱨴�!!!
	if(NULL == packet.packet)
		return SS_InsertMediaDataFail;

	{
		CSSLocker lock(&_sample_video_packet_list_mutex);

		if(false)//_max_video_packet_num < _sample_video_packet_list.size())
		{
			CMemPool::free_video_packet(_sample_video_packet_list.front());
			_sample_video_packet_list.pop_front();

			TRACE("~");
		}

		_sample_video_packet_list.push_back(packet);
	}

	return SS_NoErr;
}

//ȡ�������Ƶ���ݰ�
//unsigned long tt = 0;
VIDEO_PACKET_PTR CUserAgent::fetch_sample_video_packet()
{
	{
		CSSLocker lock(&_sample_video_packet_list_mutex);
		
		VIDEO_PACKET_PTR packet_ptr;

		if(true == _sample_video_packet_list.empty()) return packet_ptr;

		if(!_next_fetch_video_frame_timestamp) _next_fetch_video_frame_timestamp = timeGetTime();

		if(_next_fetch_video_frame_timestamp > timeGetTime())
			return packet_ptr;

		VIDEO_PACKET_PTR packet = _sample_video_packet_list.front();//*(_video_packet_list.begin());
		_sample_video_packet_list.pop_front();

		//if(100 < packet.packet->payload_size) _next_fetch_video_frame_timestamp += VIDEO_FRAME_INTERVAL;

		return packet;
	}

	//������пգ��򷵻�һ�������ݰ�
	VIDEO_PACKET_PTR packet;
	return packet;
}

int CUserAgent::send_audio()
{
	AUDIO_PACKET_PTR packet_ptr = fetch_sample_audio_packet();

	if(NULL == packet_ptr.packet || !(packet_ptr.packet->payload_size))
	{
		CMemPool::free_audio_packet(packet_ptr);
		return -1;
	}

	if(0xffff == packet_ptr.packet->sequence)
	{
		//_rtmp_session.send_aac_spec(1, 44100, 1);
	}

	int ret = -1;
	{
		CSSLocker lock(&_rtmp_send_mutex);

		ret = 1;//_rtmp_session.input_audio_packet(packet_ptr.packet->payload, packet_ptr.packet->payload_size, packet_ptr.packet->timestamp);
		//TRACE("\nA<%d %d> ", packet_ptr.packet->payload_size, ret);//CRTMPSession::get_packet_type(p, _nal_len[i], seprator_len));
	}

	CMemPool::free_audio_packet(packet_ptr);

	return ret;
}

int CUserAgent::send_video()
{
	VIDEO_PACKET_PTR packet_ptr = fetch_sample_video_packet();

	if(NULL == packet_ptr.packet || !packet_ptr.packet->payload_size)
	{
		CMemPool::free_video_packet(packet_ptr);
		return -1;
	}

	int ret = -1;
	{
		CSSLocker lock(&_rtmp_send_mutex);

		ret = 1;//_rtmp_session.input_video_packet(packet_ptr.packet->payload, packet_ptr.packet->payload_size, packet_ptr.packet->timestamp);

		if(404 != ret)
			TRACE("\nSV<%d %d> ", packet_ptr.packet->payload_size, ret);//CRTMPSession::get_packet_type(p, _nal_len[i], seprator_len));
		else
			TRACE("\nSE<%d %d> ", packet_ptr.packet->payload_size, ret);//CRTMPSession::get_packet_type(p, _nal_len[i], seprator_len));

	}

	CMemPool::free_video_packet(packet_ptr);

	return ret;
}

