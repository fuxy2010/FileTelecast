// **********************************************************************
// ����: ������
// �汾: 1.0
// ����: 2011-01 ~ 2011-11
// �޸���ʷ��¼: 
// ����, ����, �������
// **********************************************************************
#ifndef _AUDIO_CODEC_H_
#define _AUDIO_CODEC_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "GeneralDef.h"
#include "MediaData.h"
#include "Locker.h"

#define PKT_SAMPLE_NUM	(AUDIO_FRAME_LENGTH * AUDIO_FRAME_PER_PACKET)
#define PKT_FRAME_NUM	AUDIO_FRAME_PER_PACKET

namespace ScheduleServer
{
	//�������
	class CAudioCodec
	{

	public:
		CAudioCodec() : _encoder(NULL), _decoder(NULL) {};
		virtual ~CAudioCodec() {};

		virtual int encode(short* frame, unsigned char* bits) = 0;//������֡ԭʼ���������ر���������ֽڳ���

		virtual int decode(unsigned char* bits, short* frame, int crc = 0) = 0;//����������֡ԭʼ�����������õ������ĳ���

		virtual int calculate_energy(unsigned char* bits, int len) = 0;//��ȡһ���������Ƶ���ݰ�������,len ���ֽ�Ϊ��λ

		
		//����һ·
		//mix_buffer-- ����buffer�����һ·ʱӦ������
		//frame��������������
		//frame_len�������������ȣ����ֽ�Ϊ��λ
		//fscale������Ȩ����,��·���仯��Ĭ��1.0
		//energy---������������,��ѡ,Ĭ��0
		static int mix(short* mix_buffer, short *frame, int frame_len, double fscale, int energy)
		{
#if 1
			return 0;//mix_audio(mix_buffer, frame, frame_len * sizeof(char) / sizeof(short), fscale, energy);
#else
			//::memcpy(mix_buffer, frame, frame_len);
			//return frame_len;
#endif
		}


	protected:
		void* _encoder;
		void* _decoder;

	protected:
		CSSMutex _encode_mutext;//���뻥����
		CSSMutex _decode_mutext;//���뻥����

	};
}

#endif  // _AUDIO_CODEC_H_      
