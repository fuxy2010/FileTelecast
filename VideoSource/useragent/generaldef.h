// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2009-01 ~ 2009-03
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#ifndef _GENERAL_DEF_H_
#define _GENERAL_DEF_H_

//��������ΪVC�Ұ汾������VC6.0ʱ�����ļ��ڱ���ʱֻ������һ��
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "stdafx.h"

#ifdef _DEBUG
//#include "..\\utility\\VLD1.9H\\vld.h"
#endif

#include <string>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <set>

//#ifndef	PERFORMANCE_ANALYSIS
//#define	PERFORMANCE_ANALYSIS
//#endif

#ifndef _SENDING_DATA_BLOCK_SIZE_
#define _SENDING_DATA_BLOCK_SIZE_	1400
#endif

namespace ScheduleServer
{
	//�������������Ͷ���
	typedef std::map<std::string, std::string> SS_CTX;

	//�豸���Ͷ���
	typedef enum
	{
		SIPStack = 0,	//SIP Server
		Unknown			//δ֪
	}Device_Type;

	typedef struct tagTIME_INFO
	{
		unsigned long year;		//[1900,--]
		unsigned long month;	//[1, 12]
		unsigned long day;		//[1, 31]
		unsigned long hour;		//[0, 23]
		unsigned long minute;	//[0, 59]
		unsigned long second;	//[0, 59]

		tagTIME_INFO() : year(0), month(0), day(0), hour(0), minute(0), second(0)
		{
		}
	}
	TIME_INFO;

    //����ֵ���Ͷ���
	typedef enum
	{
		SS_NoErr = 0,						//�������óɹ�
		SS_RecvData,

		//����������
		SS_ServerStartFail,					//����������ʧ��

		//UA����
		SS_InsertUAFail,					//����UAʧ��
		SS_InsertMediaDataFail,				//����ý�����ݵ�����ʧ��

		//SIPStack����
		SS_ConnectSIPStackFail,				//����SIPStackʧ��
		SS_DisconnectSIPStackFail,				//�Ͽ�SIPStackʧ��

		//ϵͳ�����߳�
		SS_StartConferenceThreadFail,		//���������߳�ʧ��

		//�����߳�
		SS_StartTaskThreadFail,			//���������߳�ʧ��
		SS_AddTaskFail,					//�������ʧ��

		//�������
		SS_InvalidTask,					//��������

		//��ѯ����ִ�н��
		SS_QueryTaskResultFail,			//��ѯ����ִ�н��ʧ��

		//��������
		SS_SyncTaskFail,					//��������ʧ��

		//RTP����
		SS_RTPSendSessionUnAvailable,		//RTP����Session������
		SS_RTPRecvSessionUnAvailable,		//RTP����Session������
		SS_AddRTPHeaderFail,				//���RTP��ͷʧ��
		SS_SendPacketFail,					//����RTP��ʧ��

		SS_Unknown_Error					//δ֪����    
	}SS_Error;
}

#endif // _GENERAL_DEF_H_
