// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2011-01 ~ 2011-11
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#ifndef _SPEAKER_AUDIO_MIX_TASK_H_
#define _SPEAKER_AUDIO_MIX_TASK_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "GeneralDef.h"
#include "Task.h"
#include "time.h"
#include "ScheduleServer.h"
#include "AudioCodec.h"
#include "TaskCounter.h"
#include "RTMPSession.h"

namespace ScheduleServer
{
	typedef enum
	{
		VUASendMediaTask_Begin = 0,//����ʼ
		VUASendMediaTask_SendMedia,//����
		VUASendMediaTask_Done//�������
	}
	VUA_SEND_MEDIA_TASK_STATUS;

	typedef struct tagVUA_SEND_MEDIA_TASK_INFO
	{
		unsigned long	task_id;//����ID
		unsigned long	ua_id;
		unsigned long	create_timestamp;//�����������ʱ��
		bool			send_audio;
		//std::vector<unsigned long> vua_id_vec;

		tagVUA_SEND_MEDIA_TASK_INFO() : task_id(0), ua_id(0), create_timestamp(::time(NULL)), send_audio(true)
		{
			//vua_id_vec.clear();
		}

		~tagVUA_SEND_MEDIA_TASK_INFO()
		{
			//vua_id_vec.clear();
		}
	}
	VUA_SEND_MEDIA_TASK_INFO;

	class CVUASendMediaTask : public CTask
	{
	public:
		CVUASendMediaTask(VUA_SEND_MEDIA_TASK_INFO& task_info);
		virtual ~CVUASendMediaTask();

		virtual SS_Error run();

		SS_Error close();

		virtual void shutdown() {}

	protected:
		virtual SS_Error on_done();

		virtual SS_Error on_exception();

	protected:
		volatile VUA_SEND_MEDIA_TASK_STATUS _status;

		VUA_SEND_MEDIA_TASK_INFO _task_info;

		CUserAgent* _ua;

		bool _to_be_ended;
	};
}

#endif  //_SPEAKER_AUDIO_MIX_TASK_H_      
