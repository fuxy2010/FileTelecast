// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2011-01 ~ 2011-11
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#ifndef _JRTP_SESSION_H_
#define _JRTP_SESSION_H_

#include <iostream>
#include <string>
#include <vector>

#include "GeneralDef.h"
//#include "rtpsession.h"

typedef struct tagRTPHeader//fym struct RTPHeader
{
#ifdef RTP_BIG_ENDIAN
	unsigned char version:2;
	unsigned char padding:1;
	unsigned char extension:1;
	unsigned char csrccount:4;

	unsigned char marker:1;
	unsigned char payloadtype:7;
#else // little endian
	unsigned char csrccount:4;
	unsigned char extension:1;
	unsigned char padding:1;
	unsigned char version:2;

	unsigned char payloadtype:7;
	unsigned char marker:1;
#endif // RTP_BIG_ENDIAN

	unsigned short sequencenumber;
	unsigned int timestamp;
	unsigned int ssrc;
}
RTPHeader, *RTPHeaderPtr;//fym

namespace ScheduleServer
{
	//RTP������
	typedef enum
	{
		AMRNBRTPPacket = 0,//��Ƶ��
		PCMRTPPacket,//��Ƶ��
		H264RTPPacket,//��Ƶ��
		UnknownRTPPacket//δ֪
	}
	RTP_PACKET_TYPE;

	class CRTPRecvSession//���ڽ�������Ƶ���ݼ��շ�SIP��Ϣ
	{
	public:
		CRTPRecvSession(unsigned short port);//portΪ�����������ݰ��Ķ˿�
		virtual ~CRTPRecvSession();

	public:
		/*void set_rtp_callback(RTPCallBackFunc func)
		{
		}

		void set_udp_callback(UDPCallBackFunc func)
		{
		}*/

		//����ģ��UA����SIP��Ϣ
		SS_Error send_udp_packet(const string& dest_ip, const unsigned short dest_port, const unsigned char* packet, const unsigned long& length)
		{
			return SS_NoErr;
		}

	private:
		bool _available;
		//RTPSession _rtp_session;

	};

	//////////////////////////////////////////////////////////////////////////
	class CRTPNATSession//�������������е�UA��������Ƶ����
	{
	public:
		CRTPNATSession(unsigned short port);//portΪsocket�󶨶˿�
		virtual ~CRTPNATSession();

	public:
		/*void set_udp_callback(UDPCallBackFunc func)
		{
		}*/

		//��data�Ŀ�ͷ����RTP��ͷ
		//data�����ڿ�ͷԤ��12���ֽ����ڴ��RTP��ͷ
		//lengthΪdata�ĳ��ȣ�����ͷ��
		static SS_Error add_rtp_header(unsigned char* data, const unsigned long& length,
			const unsigned char& payload_type, const bool& mark,
			const unsigned short& sequence, const unsigned long& timestamp, const unsigned long& ssrc);

		//����RTP������һ��ַ
		//packet�Ѱ���RTP��ͷ��len������ͷ����
		//����ǰlen����У�鳤�ȣ����ܳ���1612�ֽ�
		SS_Error send_rtp_packet(const struct sockaddr_in& dest_addr, const unsigned char* packet, const unsigned long& length);

		//����RTP���������ַ
		//packet�Ѱ���RTP��ͷ��len������ͷ����
		//����ǰlen����У�鳤�ȣ����ܳ���1612�ֽ�
		SS_Error send_rtp_packet(const vector<struct sockaddr_in>& dest_addr_vec,
			const unsigned char* packet, const unsigned long& length);

	private:
		bool _available;
		//RTPSession _rtp_session;

	};
}

#endif//_JRTP_SESSION_H_