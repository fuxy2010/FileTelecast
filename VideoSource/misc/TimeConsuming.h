// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2011-01 ~ 2011-11
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#ifndef _TIME_CONSUMING_H_       
#define _TIME_CONSUMING_H_

#include <iostream>

#if 0
//���뼶��ʱ����
class CTimeConsuming
{
public:
	CTimeConsuming(char c, double threshold)
	{
		_start = timeGetTime();

		_mark = c;

		_threshold = threshold;
	}

	~CTimeConsuming()
	{
		long consuming = timeGetTime() - start;

		if(_threshold < consuming)
			cout << " <" << _mark << consuming << "> ";
	}

private:
	unsigned long _start;
	char _mark;
	double _threshold;
};
#else
//΢�뼶��ʱ����
class CTimeConsuming
{
public:
	CTimeConsuming(char c, double threshold)
	{
		QueryPerformanceCounter(&_litmp);
		_start = _litmp.QuadPart;// ��ó�ʼֵ

		_mark = c;

		_threshold = threshold;
	}

	~CTimeConsuming()
	{
		if(0 == CTimeConsuming::_clock_frequency)
		{
			QueryPerformanceFrequency(&_litmp);
			CTimeConsuming::_clock_frequency = (double)_litmp.QuadPart;// ��ü�������ʱ��Ƶ��
		}

		QueryPerformanceCounter(&_litmp);

		double consuming = (double)(1000 * (_litmp.QuadPart - _start) /  CTimeConsuming::_clock_frequency);

		if(_threshold < consuming)
			std::cout << " <" << _mark << consuming << "> ";
	}

public:
	static double _clock_frequency;

private:
	LARGE_INTEGER _litmp;
	LONGLONG _start;
	double _freq;
	char _mark;
	double _threshold;
};
#endif
//A--SS_Error CRTSPRecvTask::run()
//B--
//C--int CAMRNBCodec::decode(unsigned char* bits, short* frame, int crc)
//D--void CScheduleServer::test_db_proxy()
//E--int CAMRNBCodec::encode(short* frame, unsigned char* bits)
//F--bool CConferenceTask::fetch_raw_audio_frame()
//G--void CScheduleServer::test_mars()
//H--
//I--void CConferenceTask::init_participants()
//J--
//K--
//L--
//M--void CConferenceTask::add_audio_mix_tasks()
//N--void CConferenceTask::notify_sponsor_participants_status()
//O--
//P--void CConferenceTask::update_participants()
//Q--
//R--SS_Error CConferenceTask::run()
//S--void CConferenceTask::update_participants_status()
//T--
//U--
//V--void CConferenceTask::add_video_deliver_task()
//W--
//X--void CVUAProcessThread::entry()
//Y--void CVUASendThread::entry()
//Z--

#endif