// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2011-01 ~ 2011-11
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#ifndef _BASE_THREAD_H_     
#define _BASE_THREAD_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "GeneralDef.h"
#include "Locker.h"

#ifndef _THREAD_STATISTIC_
//#include "MiscTool.h"
//#define _THREAD_STATISTIC_
//#define _DURANCE_THRESHOLD		1
#endif

namespace ScheduleServer
{
	// �̻߳���
	class CBaseThread
	{
	public:
		static void initialize();

		CBaseThread();
		virtual ~CBaseThread();

		SS_Error start(void* parent);
		SS_Error shutdown();

	protected:
		void join();

		virtual void on_start() = 0;

		virtual void on_close() = 0;

		//�߳���ں���
		static unsigned int WINAPI _entry(LPVOID inThread);
		//�߳�ʵ����ں�����������
		virtual void entry() = 0;

	protected:
		//static DWORD _thread_storage_index;//�ֲ߳̾��洢����

		void* _parent_ptr;//�߳�������ָ��

		bool _joined;//�߳��Ƿ񼴽��ر�

		HANDLE _thread_handle;

		//void* _thread_data_ptr;

		//static void* _main_thread_data_ptr;

		CSSMutex _mutex; //��������ź���

	protected:
#ifdef _THREAD_STATISTIC_
		LARGE_INTEGER _litmp;
		LONGLONG _enter_timestamp;
		LONGLONG _leave_timestamp;
		LONGLONG _now;
		double _clock_freq;
		bool _should_printf_statistic_info;//����whileѭ�����Ƿ��ӡ�߳�ִ��ͳ����Ϣ
		FILE* _log;
		char _log_str[32];
#endif

	protected:
		void statistic_enter(const char& flag)//ͳ�Ʋ���ӡ�������ν����߳�while��ʱ����
		{
#ifdef _THREAD_STATISTIC_
			QueryPerformanceCounter(&_litmp);
			_now = _litmp.QuadPart;

			if(true == _should_printf_statistic_info)
			{
				//cout << " " << (double)(1000 * (_now - _enter_timestamp) / _clock_freq) << "] ";
				//cout << "[" << flag << " " << (double)(1000 * (_leave_timestamp - _enter_timestamp) / _clock_freq) << " " << (double)(1000 * (_now - _enter_timestamp) / _clock_freq) << "] ";
				//cout << "[" << flag << " " << (double)(1000 * (_now - _leave_timestamp) / _clock_freq) << "]";
				if(_log != NULL)
				{
					::memset(_log_str, 0, sizeof(_log_str));
					sprintf(_log_str, "[%c %f] ", flag, (double)(1000 * (_now - _leave_timestamp) / _clock_freq));
					fwrite(_log_str, 1, strlen(_log_str), _log);
				}
			}

			_enter_timestamp = _now;
#endif
		}

		void statistic_leave()//ͳ�Ʋ���ӡ�߳�һ��whileѭ���ĺ�ʱ����sleep��䣩
		{
#ifdef _THREAD_STATISTIC_
			QueryPerformanceCounter(&_litmp);
			_leave_timestamp = _litmp.QuadPart;

			_should_printf_statistic_info = (_leave_timestamp >= _enter_timestamp + ((_clock_freq * _DURANCE_THRESHOLD) / 1000)) ? true : false;

			if(true == _should_printf_statistic_info)
			{
				//cout << "[" << flag << " " << (double)(1000 * (_leave_timestamp - _enter_timestamp) / _clock_freq);
			}

			if(NULL == _log)
			{
				_log = fopen(string(MiscTools::parse_type_to_string<unsigned long>(reinterpret_cast<unsigned long>(this)) + ".log").c_str(), "ab+");
			}
#endif
		}
	};
}

#endif  // _BASE_THREAD_H_      
