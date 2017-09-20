// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2011-01 ~ 2011-11
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#ifndef _USER_AGENT_H_
#define _USER_AGENT_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#define SEND_NAT_PROBE_MSG WM_USER + 100

#include "GeneralDef.h"
#include "Locker.h"
#include "MediaData.h"
#include "WUACodec.h"
#include "JRTPSession.h"
#include "UAStatistics.h"
#include "RTMPSession.h"

namespace ScheduleServer
{
	//UA����
	typedef enum
	{
		UA_MobilePhone = 0,//�ֻ�
		UA_Radio,//350M��̨
		UA_FixedPhone,//�̻�
		UA_Unknown//δ֪
	}
	USER_AGENT_TYPE;

	//UA��Ϣ
	typedef struct tagUSER_AGENT_INFO
	{
		unsigned long		id;//UA��ID����UAΪ�ֻ���ΪSIM����IMSI��
		USER_AGENT_TYPE		type;//����

		tagUSER_AGENT_INFO() :
		id(0), type(UA_MobilePhone)
		{
		}

		~tagUSER_AGENT_INFO()
		{
		}

	}
	USER_AGENT_INFO, *USER_AGENT_INFO_PTR;

	class CUserAgent
	{
	public:
		CUserAgent(USER_AGENT_INFO& info);
		~CUserAgent();

	private:
		CAudioCodec* _audio_codec;
	
		unsigned char _aac_packet[1024];

	public:
		//�ɼ�ý������////////////////////////////////////////////////////////////////////////
		//���뱾�ز�������Ƶ֡
		SS_Error add_sample_audio_packet(const char* frame, const unsigned long& length);

		//ȡ�������Ƶ֡
		AUDIO_PACKET_PTR fetch_sample_audio_packet();

		//���뱾�ز�����Ƶ���ݰ�
		int input_video_frame(unsigned char* frame);
		SS_Error add_sample_video_packet(VIDEO_PACKET_PTR packet);
		
		//ȡ�������Ƶ���ݰ�
		VIDEO_PACKET_PTR fetch_sample_video_packet();

	private:
		//ɾ��������Ƶ���ݰ�
		SS_Error remove_all_audio_packet();
		//ɾ��������Ƶ���ݰ�
		SS_Error remove_all_video_packet();

	private:
		//��������֡����
		std::list<AUDIO_PACKET_PTR> _sample_audio_packet_list;
		CSSMutex _sample_audio_packet_list_mutex;

		//��Ƶ���ݰ�����
		std::list<VIDEO_PACKET_PTR> _sample_video_packet_list;
		CSSMutex _sample_video_packet_list_mutex;

	public:
		//UA��Ϣ
		USER_AGENT_INFO _info;
		//ȡ����_info���ʼ��� CSSMutex _info_mutex;

		volatile bool _start_hls;

		CUAStatistics* _audio_statistics;
		CUAStatistics* _video_statistics;

	protected:
		static const unsigned long _max_audio_packet_num;//��Ƶ���ݰ�������󳤶�
		static const unsigned long _max_video_packet_num;//��Ƶ���ݰ�������󳤶�

	//////////////////////////////////////////////////////////////////////////
	private:
		CRTMPSession _rtmp_session;

		bool _first_video_packet;
		unsigned long _first_video_timestamp;

		bool _first_audio_packet;
		unsigned long _first_audio_timestamp;

	public:
		int rtmp_connect(char* url)
		{
			return _rtmp_session.connect(url);
		}

		int disconnect()
		{
			return _rtmp_session.disconnect();
		}

		int send_audio();
		int send_video();

	private:
		static CSSMutex _rtmp_send_mutex;

	private://video
		//�������������
		long _x264_handle;  //����һ·������
		unsigned char _stream_buf[655360];  //�����һ֡�����Ļ���,ע������ĳߴ粻��̫С
		int _nal_len[1000]; //�������ݰ��ߴ�洢����,ע������ĳߴ粻��̫С
	};
}

#endif//_EVENT_DATA_H_