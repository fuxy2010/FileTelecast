// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2011-01 ~ 2011-11
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#include "GeneralDef.h"
#include "ScheduleServer.h"
#include "RegisterThread.h"
#include "TimeConsuming.h"
#include "resource.h"

using namespace ScheduleServer;

void CRegisterThread::entry()
{
	//unsigned long start = timeGetTime();//�����߳�ѭ��ִ����ʼʱ��

	//15���¼һ��UAͳ����Ϣ
	if(15000 <= (::timeGetTime() - _latest_ua_statistics_timestamp))
	{
		_latest_ua_statistics_timestamp = ::timeGetTime();

		SINGLETON(CScheduleServer).ua_statistics();
	}

	//5�����һ������Ƶͳ����ʾ
	/*if(5000 <= (::timeGetTime() - _latest_show_statistics_timestamp))
	{
		_latest_show_statistics_timestamp = ::timeGetTime();
		
		CUserAgent* ua = SINGLETON(CScheduleServer).fetch_ua(SINGLETON(CScheduleServer).get_playing_ua_id());

		if(NULL != ua)
		{
			//��Ƶͳ��
			string statistics = "��Ƶ������: " + MiscTools::parse_type_to_string<double>(ua->_audio_statistics->get_packet_lost_rate());
			statistics += "%, ���: " + MiscTools::parse_type_to_string<double>(ua->_audio_statistics->get_averate_packet_timestamp_interval());
			statistics += "ms, �����ʱ: " + MiscTools::parse_type_to_string<double>(ua->_audio_statistics->get_packet_delay());
			statistics += "ms, ����: " + MiscTools::parse_type_to_string<double>(ua->_audio_statistics->get_bitrate());
			statistics += "kbps";

			//AfxGetApp()->GetMainWnd()->GetDlgItem(IDC_AUDIO_STATISTICS)->SetWindowText(statistics.c_str());

			//��Ƶͳ��
			statistics += "\n\r��Ƶ������: " + MiscTools::parse_type_to_string<double>(ua->_video_statistics->get_packet_lost_rate());
			statistics += "%, ���: " + MiscTools::parse_type_to_string<double>(ua->_video_statistics->get_averate_packet_timestamp_interval());
			statistics += "ms, ����ӳ�: " + MiscTools::parse_type_to_string<double>(ua->_video_statistics->get_packet_delay());
			statistics += "ms, ����: " + MiscTools::parse_type_to_string<double>(ua->_video_statistics->get_bitrate());
			statistics += "kbps";

			AfxGetApp()->GetMainWnd()->GetDlgItem(IDC_SIP_STATISTICS)->SetWindowText(statistics.c_str());			
		}
	}*/

	Sleep(3000);
}

void CRegisterThread::on_start()
{
}

void CRegisterThread::on_close()
{
}
