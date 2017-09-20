// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2011-01 ~ 2011-11
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#ifndef _MEDIA_DATA_H_
#define _MEDIA_DATA_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

//����USE_BOOST_POOL��ʹ��boost�ڴ�أ�����ʹ�õ���Debug���������ڴ�й©���
#ifndef	USE_BOOST_POOL
//#define USE_BOOST_POOL
#endif

#ifdef	USE_BOOST_POOL
#include <boost/pool/singleton_pool.hpp>
#endif

//ÿ����ƵRTP����������֡����
#define AUDIO_FRAME_PER_PACKET			3
//����֡����(��λshort��
#define AUDIO_FRAME_LENGTH				160

//RTP��ͷ����
#define RTP_HEADER_LENGTH				12

//��Ƶ���ݱ����RTP����󳤶�(��RTP��ͷ����12��,��λ�ֽ�
//#define MAX_AUDIO_PACKET_SIZE_BYTE		(13 * AUDIO_FRAME_PER_PACKET + RTP_HEADER_LENGTH)
//Ϊ������̨���̻�����RTP��(������ԭʼ����������󳤶Ȳ��䣩
//#define MAX_AUDIO_PACKET_SIZE_BYTE		(AUDIO_FRAME_LENGTH * AUDIO_FRAME_PER_PACKET * sizeof(WORD) + RTP_HEADER_LENGTH)
//һ�η��䵽λ
#define MAX_AUDIO_PACKET_SIZE_BYTE		1024

//��ƵRTP�������ԭʼ����֡(��AUDIO_FRAME_PER_PACKET��)���ݰ�����,��λ��
//#define MAX_RAW_AUDIO_FRAME_SIZE_WORD	(AUDIO_FRAME_LENGTH * AUDIO_FRAME_PER_PACKET)
//һ�η��䵽λ
#define MAX_RAW_AUDIO_FRAME_SIZE_WORD	512

//��ƵRTP���ݰ���󳤶�(��RTP��ͷ����12��,��λ�ֽ�
#define MAX_VIDEO_PACKET_SIZE_BYTE		(65536 * 10)

//SIP��Ϣ���ݰ���󳤶ȵ�λ�ֽ�
#define MAX_SIP_MESSAGE_SIZE_BYTE		1400

namespace ScheduleServer
{
	//////////////////////////////////////////////////////////////////////////
	//��ƵRTP���ݰ�
	typedef struct tagAUDIO_PACKET
    {
		unsigned long	ua_id;//����ԴUA ID
		unsigned short	payload_size;//���ݰ�ʵ�ʳ��ȣ���RTP��ͷ��
		unsigned long	sequence;//RTP�����к�
		unsigned long	timestamp;//������ʱ��
		int				energy;//��������

		//��ƵRTP���ݰ�����RTP��ͷ��
		unsigned char	payload[MAX_AUDIO_PACKET_SIZE_BYTE];

		tagAUDIO_PACKET() : ua_id(0), payload_size(0), sequence(0), timestamp(0), energy(0)
		{
			//����boost�ڴ�ط�ʽ���÷����������ã�����malloc�г�ʼ��
			::memset(payload, 0, sizeof(payload));
		}

		~tagAUDIO_PACKET()
		{
		}

		bool operator < (tagAUDIO_PACKET const& _packet) const
		{
			if(timestamp < _packet.timestamp)
				return true;
			else if(timestamp == _packet.timestamp)
				return sequence < _packet.sequence;

			return false;
		}
    }
	AUDIO_PACKET;
	
	typedef struct tagAUDIO_PACKET_PTR
	{
		AUDIO_PACKET* packet;

		tagAUDIO_PACKET_PTR() : packet(NULL)
		{
			//�򲻿ɽ�����packet_ptr���ڴ��� cout << "\ncreate AUDIO_PACKET_PTR";
		}

		~tagAUDIO_PACKET_PTR()
		{
			//cout << "\ndelete AUDIO_PACKET_PTR";
		}

		bool operator < (tagAUDIO_PACKET_PTR const& _packet_ptr) const
		{
			if(NULL == packet)
				return true;

			if(NULL == _packet_ptr.packet)
				return false;

			return (*packet < *(_packet_ptr.packet));
		}
	}
	AUDIO_PACKET_PTR;

	//////////////////////////////////////////////////////////////////////////
	//ԭʼ����֡
	typedef struct tagRAW_AUDIO_FRAME
	{
		unsigned long	ua_id;//����ԴUA ID
		int				energy;//����֡����
		short			payload[MAX_RAW_AUDIO_FRAME_SIZE_WORD];//����֡�洢��
		bool			available;
		unsigned long	sequence;//RTP�����к�
		unsigned long	timestamp;//������ʱ��

		tagRAW_AUDIO_FRAME() : ua_id(0), energy(0), available(false)
		{
			//����boost�ڴ�ط�ʽ���÷����������ã�����malloc�г�ʼ��
			::memset(payload, 0, sizeof(payload));
		}

		~tagRAW_AUDIO_FRAME()
		{
		}

		bool operator < (tagRAW_AUDIO_FRAME const& _frame) const
		{
			if(timestamp < _frame.timestamp)
				return true;
			else if(timestamp == _frame.timestamp)
				return sequence < _frame.sequence;

			return false;
		}
	}
	RAW_AUDIO_FRAME;//, *RAW_AUDIO_FRAME_PTR;

	typedef struct tagRAW_AUDIO_FRAME_PTR
	{
		RAW_AUDIO_FRAME* frame;

		tagRAW_AUDIO_FRAME_PTR() : frame(NULL)
		{
		}

		~tagRAW_AUDIO_FRAME_PTR()
		{
		}

		bool operator < (tagRAW_AUDIO_FRAME_PTR const& _frame_ptr) const
		{
			if(NULL == frame)
				return true;

			if(NULL == _frame_ptr.frame)
				return false;

			return (*frame < *(_frame_ptr.frame));
		}
	}
	RAW_AUDIO_FRAME_PTR;

	//////////////////////////////////////////////////////////////////////////
	//��ƵRTP���ݰ���Ϣ
	typedef struct tagVIDEO_PACKET
	{
		DWORD			ua_id;//����ԴUA ID
		//WORD			payload_size;//���ݰ�ʵ�ʴ�С����RTP��ͷ)
		DWORD			payload_size;//���ݰ�ʵ�ʴ�С����RTP��ͷ)
		DWORD			sequence;//RTP�����к�
		unsigned long	timestamp;//RTP��ʱ��
		bool			mark;
		unsigned short	frame;//֡����0-δ֪, 1-I֡��2-��I֡

		//��ƵRTP���ݰ�����RTP��ͷ��
		BYTE payload[MAX_VIDEO_PACKET_SIZE_BYTE];

		tagVIDEO_PACKET() : ua_id(0), payload_size(0), sequence(0), timestamp(0), mark(false), frame(0)
		{
			//����boost�ڴ�ط�ʽ���÷����������ã�����malloc�г�ʼ��
			::memset(payload, 0, sizeof(payload));
		}

		~tagVIDEO_PACKET()
		{
		}

		bool operator < (tagVIDEO_PACKET const& _packet) const
		{
			if(timestamp < _packet.timestamp)
				return true;
			else if(timestamp == _packet.timestamp)
			{
				return sequence < _packet.sequence;
			}

			return false;
		}
	}
	VIDEO_PACKET;

	//��ƵRTP���ݰ�
	typedef struct tagVIDEO_PACKET_PTR
	{
		VIDEO_PACKET* packet;

		tagVIDEO_PACKET_PTR() : packet(NULL)
		{
		}

		~tagVIDEO_PACKET_PTR()
		{
		}

		bool operator < (tagVIDEO_PACKET_PTR const& _packet_ptr) const
		{
			if(NULL == packet)
				return true;

			if(NULL == _packet_ptr.packet)
				return false;

			return (*packet < *(_packet_ptr.packet));
		}
	}
	VIDEO_PACKET_PTR;

	//////////////////////////////////////////////////////////////////////////
	//SIP��Ϣ���ݰ���Ϣ
	typedef struct tagSIP_MESSAGE
	{
		unsigned long	message_size;//���ݰ�ʵ�ʴ�С
		unsigned long	timestamp;//RTP��ʱ��
		unsigned long	interval;//��Ϊ�����͵�SIP��Ϣ��Ϊ���ͺ�Sleep�ļ�������룩����Ϊ�յ���SIP��Ϣ��Ϊ0

		//SIP��Ϣ���ݰ�
		BYTE payload[MAX_SIP_MESSAGE_SIZE_BYTE];

		tagSIP_MESSAGE() : message_size(0), timestamp(0), interval(0)
		{
			//����boost�ڴ�ط�ʽ���÷����������ã�����malloc�г�ʼ��
			::memset(payload, 0, sizeof(payload));
		}

		~tagSIP_MESSAGE()
		{
		}

		bool operator < (tagSIP_MESSAGE const& _message) const
		{
			return (timestamp < _message.timestamp);
		}
	}
	SIP_MESSAGE;

	//SIP��Ϣ���ݰ�
	typedef struct tagSIP_MESSAGE_PTR
	{
		SIP_MESSAGE* message;

		tagSIP_MESSAGE_PTR() : message(NULL)
		{
		}

		~tagSIP_MESSAGE_PTR()
		{
		}

		bool operator < (tagSIP_MESSAGE_PTR const& _message_ptr) const
		{
			if(NULL == message)
				return true;

			if(NULL == _message_ptr.message)
				return false;

			return (*message < *(_message_ptr.message));
		}
	}
	SIP_MESSAGE_PTR;

#ifdef USE_BOOST_POOL

	//////////////////////////////////////////////////////////////////////////
	//��ƵRTP���ݰ��ڴ��
	typedef struct tagSINGLETON_AUDIO_PACKET_POOL {} SINGLETON_AUDIO_PACKET_POOL;
	typedef boost::singleton_pool<SINGLETON_AUDIO_PACKET_POOL, sizeof(AUDIO_PACKET)> audio_packet_pool;

	//ԭʼ����֡�ڴ��
	typedef struct tagSINGLETON_RAW_AUDIO_FRAME_POOL {} SINGLETON_RAW_AUDIO_FRAME_POOL;
	typedef boost::singleton_pool<SINGLETON_RAW_AUDIO_FRAME_POOL, sizeof(RAW_AUDIO_FRAME)> raw_audio_frame_pool;

	//��ƵRTP���ݰ��ڴ��
	typedef struct tagSINGLETON_VIDEO_PACKET_POOL {} SINGLETON_VIDEO_PACKET_POOL;
	typedef boost::singleton_pool<SINGLETON_VIDEO_PACKET_POOL, sizeof(VIDEO_PACKET)> video_packet_pool;

	//SIP��Ϣ���ݰ��ڴ��
	typedef struct tagSINGLETON_SIP_MESSAGE_POOL {} SINGLETON_SIP_MESSAGE_POOL;
	typedef boost::singleton_pool<SINGLETON_SIP_MESSAGE_POOL, sizeof(SIP_MESSAGE)> sip_message_pool;

	class CMemPool
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		static void malloc_audio_packet(AUDIO_PACKET_PTR& packet_ptr)
		{
			if(NULL != packet_ptr.packet)
				audio_packet_pool::free(packet_ptr.packet);

			packet_ptr.packet = (AUDIO_PACKET*)audio_packet_pool::malloc();

			::memset(packet_ptr.packet, 0, sizeof(AUDIO_PACKET));
		}

		static void free_audio_packet(AUDIO_PACKET_PTR& packet_ptr)
		{
			if(NULL == packet_ptr.packet)
				return;

			audio_packet_pool::free(packet_ptr.packet);
			packet_ptr.packet = NULL;
		}

		//////////////////////////////////////////////////////////////////////////
		static void malloc_raw_audio_frame(RAW_AUDIO_FRAME_PTR& frame_ptr)
		{
			if(NULL != frame_ptr.frame)
				raw_audio_frame_pool::free(frame_ptr.frame);

			frame_ptr.frame = (RAW_AUDIO_FRAME*)raw_audio_frame_pool::malloc();

			::memset(frame_ptr.frame, 0, sizeof(RAW_AUDIO_FRAME));

			frame_ptr.frame->available = false;
		}

		static void free_raw_audio_frame(RAW_AUDIO_FRAME_PTR& frame_ptr)
		{
			if(NULL == frame_ptr.frame)
				return;

			raw_audio_frame_pool::free(frame_ptr.frame);
			frame_ptr.frame = NULL;
		}

		//////////////////////////////////////////////////////////////////////////
		static void malloc_video_packet(VIDEO_PACKET_PTR& packet_ptr)
		{
			if(NULL != packet_ptr.packet)
				video_packet_pool::free(packet_ptr.packet);

			packet_ptr.packet = (VIDEO_PACKET*)video_packet_pool::malloc();

			::memset(packet_ptr.packet, 0, sizeof(VIDEO_PACKET));
		}

		static void free_video_packet(VIDEO_PACKET_PTR& packet_ptr)
		{
			if(NULL == packet_ptr.packet)
				return;

			video_packet_pool::free(packet_ptr.packet);
			packet_ptr.packet = NULL;
		}

		//////////////////////////////////////////////////////////////////////////
		static void malloc_sip_message(SIP_MESSAGE_PTR& message_ptr)
		{
			if(NULL != message_ptr.message)
				sip_message_pool::free(message_ptr.message);

			message_ptr.message = (SIP_MESSAGE*)sip_message_pool::malloc();

			::memset(message_ptr.message, 0, sizeof(SIP_MESSAGE));
		}

		static void free_sip_message(SIP_MESSAGE_PTR& message_ptr)
		{
			if(NULL == message_ptr.message)
				return;

			sip_message_pool::free(message_ptr.message);
			message_ptr.message = NULL;
		}
	};

#else//#ifdef USE_BOOST_POOL

#ifdef _DEBUG
	/*for audio sampler
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>

	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif*/
#endif //_DEBUG

	class CMemPool
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		static void malloc_audio_packet(AUDIO_PACKET_PTR& packet_ptr)
		{
			if(NULL != packet_ptr.packet)
				delete packet_ptr.packet;

			packet_ptr.packet = new AUDIO_PACKET;

			::memset(packet_ptr.packet, 0, sizeof(AUDIO_PACKET));
		}

		static void free_audio_packet(AUDIO_PACKET_PTR& packet_ptr)
		{
			if(NULL == packet_ptr.packet)
				return;

			delete packet_ptr.packet;
			packet_ptr.packet = NULL;
		}

		//////////////////////////////////////////////////////////////////////////
		static void malloc_raw_audio_frame(RAW_AUDIO_FRAME_PTR& frame_ptr)
		{
			if(NULL != frame_ptr.frame)
				delete frame_ptr.frame;

			frame_ptr.frame = new RAW_AUDIO_FRAME;

			::memset(frame_ptr.frame, 0, sizeof(RAW_AUDIO_FRAME));
		}

		static void free_raw_audio_frame(RAW_AUDIO_FRAME_PTR& frame_ptr)
		{
			if(NULL == frame_ptr.frame)
				return;

			delete frame_ptr.frame;
			frame_ptr.frame = NULL;
		}

		//////////////////////////////////////////////////////////////////////////
		static void malloc_video_packet(VIDEO_PACKET_PTR& packet_ptr)
		{
			if(NULL != packet_ptr.packet)
				delete packet_ptr.packet;

			packet_ptr.packet = new VIDEO_PACKET;

			::memset(packet_ptr.packet, 0, sizeof(VIDEO_PACKET));
		}

		static void free_video_packet(VIDEO_PACKET_PTR& packet_ptr)
		{
			if(NULL == packet_ptr.packet)
				return;

			delete packet_ptr.packet;
			packet_ptr.packet = NULL;
		}

		//////////////////////////////////////////////////////////////////////////
		static void malloc_sip_message(SIP_MESSAGE_PTR& message_ptr)
		{
			if(NULL != message_ptr.message)
				delete message_ptr.message;

			message_ptr.message = new SIP_MESSAGE;

			::memset(message_ptr.message, 0, sizeof(SIP_MESSAGE));
		}

		static void free_sip_message(SIP_MESSAGE_PTR& message_ptr)
		{
			if(NULL == message_ptr.message)
				return;

			delete message_ptr.message;
			message_ptr.message = NULL;
		}
	};
#endif
}

//�������ݲ�����������룩
#ifndef	AUDIO_SAMPLING_RATE
#define	AUDIO_SAMPLING_RATE	60
#endif

#endif//_MEDIA_DATA_H_

/*
��RTPRecvSession�Ļص��д�����malloc AUDIO_PACKET_PTR��AUDIO_PACKET_PTR������RTP����
�ٽ�AUDIO_PACKET_PTR��VIDEO_PACKET_PTR���Ƶ���Ӧ��CUserAgent�Ĵ洢����ʱ������г������򵯳���������ݰ���free��

�����У���ÿ��CUserAgent����Ƶ���ݶ�����ȡ��һ��AUDIO_PACKET_PTR������input_audio_packet�������Ƹ�PARTICIPANT��audio_packet_ptr
ע��input_audio_packet�л�����freeһ��audio_packet_ptr�����ϴθ��Ƹ�audio_packet_ptr��AUDIO_PACKET_PTR
���ڻ����������ͻ������������עfree��CUserAgentȡ�������ݰ�
*/