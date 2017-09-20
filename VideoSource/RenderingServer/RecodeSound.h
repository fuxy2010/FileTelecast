#pragma once

#define WM_RECORDSOUND_STARTRECORDING	WM_USER+500
#define WM_RECORDSOUND_STOPRECORDING		WM_USER+501
#define WM_RECORDSOUND_ENDTHREAD			WM_USER+502

//#define SAMPLERSEC 8000
#define MAXRECBUFFER 12
#define RECBUFFER  2048		

//#include<mmsystem.h>
//#include<mmreg.h>
//#pragma  comment(lib, "winmm.lib")

// CRecodeSound
class CRecodeSound : public CWinThread
{
	DECLARE_DYNCREATE(CRecodeSound)

public:
	CRecodeSound();//(CDialog *pDlg = NULL);                            // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CRecodeSound();
	virtual BOOL InitInstance();
	virtual int ExitInstance();

private:
	void PreCreateHeader();
	LPWAVEHDR CreateWaveHeader();
	void displayError(int mmReturn,char errmsg[]);

	//CDialog*			m_pDlg;
	//CStdioFile		m_RecodeLog;	//¼��������־�ļ� 	 
	HWAVEIN			m_hRecord;		//��Ƶ�����豸
	WAVEFORMATEX		m_WaveFormatEx; //��Ƶ��ʽ
	LPWAVEHDR		m_RecHead[MAXRECBUFFER];
	BOOL				m_IsRecoding;	//��ʶ��ǰ�Ƿ���¼����Ƶ
	int				m_IsAllocated;	//��ʶ��ǰ�Ƿ��Ѿ�����buffer

protected:
	afx_msg void OnStartRecording(WPARAM wParam, LPARAM lParam);
	afx_msg void OnStopRecording(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEndThread(WPARAM wParam, LPARAM lParam);
	void OnSoundData(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};