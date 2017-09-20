// RecodeSound.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "RecodeSound.h"
#include "scheduleserver.h"

using namespace ScheduleServer;

IMPLEMENT_DYNCREATE(CRecodeSound, CWinThread)

CRecodeSound::CRecodeSound()//(CDialog *pDlg /* = NULL */)
{
	//m_pDlg = pDlg;
	//m_RecodeLog.Open(TEXT("recording.log"), CFile::modeCreate | CFile::modeWrite);
	//m_RecodeLog.WriteString(TEXT("In the Recordsound Constructor\n"));

	m_IsRecoding = FALSE; //��ʼ��δ��ʼ¼��
	m_IsAllocated = 0;//��ʼ��δ����buffer

	PreCreateHeader();  //����buffer

	memset(&m_WaveFormatEx, 0, sizeof(m_WaveFormatEx));
	m_WaveFormatEx.wFormatTag = WAVE_FORMAT_PCM;//������ʽΪPCM		
	m_WaveFormatEx.nChannels = 1;	//���������������ڵ�������Ƶ����Ϊ1������������Ϊ2
	m_WaveFormatEx.wBitsPerSample = 16;//��������  8bits/��
	m_WaveFormatEx.cbSize = 0;//һ��Ϊ0
	m_WaveFormatEx.nSamplesPerSec = 44100; //������ 16000 ��/��
	m_WaveFormatEx.nBlockAlign = 2; //һ����Ĵ�С������bit���ֽ�������������
	m_WaveFormatEx.nAvgBytesPerSec = 88200; //ÿ��������ʣ�����ÿ���ܲɼ������ֽڵ�����
}
CRecodeSound::~CRecodeSound()
{
}

BOOL CRecodeSound::InitInstance()
{
	// TODO: �ڴ�ִ���������̳߳�ʼ��
	return TRUE;
}

int CRecodeSound::ExitInstance()
{
	// TODO: �ڴ�ִ���������߳�����
	return CWinThread::ExitInstance();
}


void CRecodeSound::PreCreateHeader()
{
	for(int i=0;i<MAXRECBUFFER;i++)
		m_RecHead[i]=CreateWaveHeader();
	m_IsAllocated = 1;
}

LPWAVEHDR  CRecodeSound::CreateWaveHeader()
{
	LPWAVEHDR lpHdr = new WAVEHDR;

	if(lpHdr==NULL)
	{
		//m_RecodeLog.WriteString(TEXT("\n Unable to allocate the memory"));
		return NULL;
	}

	ZeroMemory(lpHdr, sizeof(WAVEHDR));
	char* lpByte = new char[RECBUFFER];//m_WaveFormatEx.nBlockAlign*SOUNDSAMPLES)];

	if(lpByte==NULL)
	{
		//m_RecodeLog.WriteString(TEXT("\n Unable to allocate the memory"));
		return NULL;
	}
	lpHdr->lpData =  lpByte;
	lpHdr->dwBufferLength =RECBUFFER;   // (m_WaveFormatEx.nBlockAlign*SOUNDSAMPLES);
	return lpHdr;

}
void CRecodeSound::OnStartRecording(WPARAM wParam, LPARAM lParam)
{
	//m_RecodeLog.WriteString(TEXT("In OnStartrecording\n"));

	if(m_IsRecoding) //����Ѿ������ɼ�����ֱ�ӷ���
		return ;//FALSE;

	//������Ƶ�ɼ�
	MMRESULT mmReturn = ::waveInOpen( &m_hRecord, WAVE_MAPPER, &m_WaveFormatEx, ::GetCurrentThreadId(), 0, CALLBACK_THREAD);

	//Error has occured while opening device

	if(mmReturn != MMSYSERR_NOERROR ) //�򿪲ɼ�ʧ��
	{
		displayError(mmReturn,"Open");
		return ;//FALSE;
	}		

	if(mmReturn == MMSYSERR_NOERROR )
	{
		//��׼���õ�buffer�ṩ����Ƶ�����豸
		for(int i=0; i < MAXRECBUFFER ; i++)
		{
			//׼��һ��bufrer�������豸
			mmReturn = ::waveInPrepareHeader(m_hRecord,m_RecHead[i], sizeof(WAVEHDR));
			//����һ��buffer��ָ���������豸����buffer��������֪ͨ����
			mmReturn = ::waveInAddBuffer(m_hRecord, m_RecHead[i], sizeof(WAVEHDR));
		}
		//����ָ��������ɼ��豸
		mmReturn = ::waveInStart(m_hRecord);

		if(mmReturn!=MMSYSERR_NOERROR )  //��ʼ�ɼ�ʧ��
			displayError(mmReturn,"Start");
		else
			m_IsRecoding = TRUE;
	}
}

void CRecodeSound::OnStopRecording(WPARAM wParam, LPARAM lParam)
{

	//m_RecodeLog.WriteString(TEXT("\nIn the onstop recording"));
	MMRESULT mmReturn = 0;

	if(!m_IsRecoding) //FALSE
		return ;

	//ֹͣ��Ƶ�ɼ�
	mmReturn = ::waveInStop(m_hRecord);

	//To get the remaining sound data from buffer
	//Reset will call OnSoundData function	

	if(!mmReturn) //ֹͣ�ɼ��ɹ������������豸,�����豸���ᵼ�����е�buffer����������
	{
		m_IsRecoding = FALSE;
		mmReturn = ::waveInReset(m_hRecord);  //�����豸
	}

	Sleep(500); //�ȴ�һ��ʱ�䣬ʹbuffer�������

	if(!mmReturn) //�����豸�ɹ��������ر��豸
		mmReturn = ::waveInClose(m_hRecord); //�ر��豸

	return ;//mmReturn;

}

unsigned long time1 = 0;
unsigned long time2 = 0;
unsigned long duration = 0;
unsigned long frames = 0;
void CRecodeSound::OnSoundData(WPARAM wParam, LPARAM lParam)
{
	//m_RecodeLog.WriteString(TEXT("\nIn the onsound data"));

	if(m_IsRecoding==FALSE) //�����ǰ���ڲɼ�״̬
		return ;//FALSE;

	LPWAVEHDR lpHdr = (LPWAVEHDR)lParam;

	if(lpHdr->dwBytesRecorded==0 || lpHdr==NULL)
		return ;//ERROR_SUCCESS;

	//ʹ�ɼ����̣�֪����buffer�Ѿ�մ�������������
	::waveInUnprepareHeader(m_hRecord, lpHdr, sizeof(WAVEHDR));

	{
		time2 = timeGetTime();

		duration += time2 - time1;
		frames += 1;

		if(100 == frames)
		{
			//TRACE("\nAvg %d", duration / 100);

			duration = 0;
			frames = 0;
		}
		time1 = time2;
	}

	/*//���ɼ������������͸������߳�
	if(((CVideoPlayDlg *)m_pDlg)->m_pPlaySound != NULL)
		((CVideoPlayDlg *)m_pDlg)->m_pPlaySound->PostThreadMessage(WM_PLAYSOUND_PLAYBLOCK, lpHdr->dwBytesRecorded, (LPARAM)lpHdr->lpData);*/
	
	// Send recorded audio to remote host...
	/*
	if(lpHdr->lpData!=NULL )
		( (CVideoNetDlg*) dlg )->daudio.SendAudioData((unsigned char *)lpHdr->lpData,lpHdr->dwBytesRecorded);
	*/
	if(RECBUFFER == lpHdr->dwBytesRecorded)
	{
		//�����������ݵ�������
		CUserAgent* ua = SINGLETON(CScheduleServer).fetch_ua(123);

		if(NULL != ua && true == ua->_start_hls)
		{
			ua->add_sample_audio_packet(lpHdr->lpData, lpHdr->dwBytesRecorded);
		}
	}

	if(m_IsRecoding)
	{
		//���½�buffer�ָ���׼�����״̬
		::waveInPrepareHeader(m_hRecord, lpHdr, sizeof(WAVEHDR));
		::waveInAddBuffer(m_hRecord, lpHdr, sizeof(WAVEHDR));
	}
}

void CRecodeSound::OnEndThread(WPARAM wParam, LPARAM lParam)
{
	//m_RecodeLog.WriteString(TEXT("\nIN the onendthread"));

	if(m_IsRecoding)
		OnStopRecording(0,0);

	::PostQuitMessage(0);
	return ;//TRUE;
}

void CRecodeSound::displayError(int mmReturn,char errmsg[])
{
	TCHAR errorbuffer[MAX_PATH];
	TCHAR errorbuffer1[MAX_PATH];

	waveInGetErrorText(mmReturn, errorbuffer, MAX_PATH);
	wsprintf(errorbuffer1, TEXT("RECORD: %s : %x : %s") ,errmsg, mmReturn, errorbuffer);
	AfxMessageBox(errorbuffer1);  
}

BEGIN_MESSAGE_MAP(CRecodeSound, CWinThread)
	ON_THREAD_MESSAGE(MM_WIM_DATA, OnSoundData)
	ON_THREAD_MESSAGE(WM_RECORDSOUND_STARTRECORDING, OnStartRecording)
	ON_THREAD_MESSAGE(WM_RECORDSOUND_STOPRECORDING, OnStopRecording)
	ON_THREAD_MESSAGE(WM_RECORDSOUND_ENDTHREAD, OnEndThread)
END_MESSAGE_MAP()


// CRecodeSound ��Ϣ�������
