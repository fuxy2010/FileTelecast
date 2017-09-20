#ifndef _Load_Library_H_
#define _Load_Library_H_


#ifdef __cplusplus
extern "C" {
#endif

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*  x264 ����   *=*=*=*=*=*=*=*=*=*=*/

/**
 * ��ʼ��H.264������
 * @param width�� ��Ƶ֡�Ŀ��
 * @param height����Ƶ֡�ĸ߶�
 * @param i_p_size��I֡��P֡��С�ı�����ȡֵ:1-30��ֵԽС������Խƽ������I֡����Խ�Ϊ1ʱI֡��֡һ����
 * @param slice_size����Ƶ���ݰ�������ֽ���Ŀ
 * @param IntraPeriod���ؼ�֡�����ڣ�������֡һ���ؼ�֡
 * @param bitrate�����ʣ���λ��Kbit/s�� Ϊ0ʱ�ر����ʿ��ƣ�
 * @param fps��֡��(ͬ�������£�֡��Խ�ߣ�����Խ���֮��֡��Խ�ͣ�����Խ��)
 * @return������ֵΪ�������ľ��
 */
long H264EncodeInit(int width, 
					int height, 
					int i_p_size, 
					int slice_size, 
					int IntraPeriod, 
					int bitrate, 
					int fps);


/**
 * ��ʼ��H.264������
 * @param width�� ��Ƶ֡�Ŀ��
 * @param height����Ƶ֡�ĸ߶�
 * @param i_p_size��I֡��P֡��С�ı�����ȡֵ:1-30��ֵԽС������Խƽ������I֡����Խ�Ϊ1ʱI֡��֡һ����
 * @param slice_size����Ƶ���ݰ�������ֽ���Ŀ
 * @param IntraPeriod���ؼ�֡�����ڣ�������֡һ���ؼ�֡
 * @param bitrate�����ʣ���λ��Kbit/s�� Ϊ0ʱ�ر����ʿ��ƣ�
 * @param fps��֡��(ͬ�������£�֡��Խ�ߣ�����Խ���֮��֡��Խ�ͣ�����Խ��)
 * @param b_sliced_threads�����м���0��֡�����У�1��Ƭ�����У���Ƭ��������ʱ�ӵ��ٶ��������2����֡������1~N֡ʱ�ӣ�N���߳������ٶ����N����  
 * @param threads��֡������ʱ���߳���Ŀ�������߳���������CPU������
 * @return������ֵΪ�������ľ��
 */
long H264EncodeInit2(int width, 
					int height, 
					int i_p_size, 
					int slice_size, 
					int IntraPeriod, 
					int bitrate, 
					int fps, 
					int b_sliced_threads, 
					int threads);


/**
 * ��ʼ��������
 * @param handle�� �������ľ��
 * @param frame_type��ǿ�ưѵ�ǰ֡����Ϊָ�����͵�֡��0��P֡  1:�ؼ�֡  -1:�ɱ������Զ����ߣ�
 * @param in��һ֡YUV����
 * @param out��һ֡��������
 * @param nal_len��һ֡��Ϊ���Ƭ���������¼ÿ��Ƭ�ĳߴ�
 * @return������ֵ������ʱΪһ֡������������Ƭ��Ŀ��С�ڵ�����ʱ˵��Ϊ���̣߳�ֻ�п�ʼ��N֡����ֵС���㣬NΪ�߳���Ŀ
 */
int H264EncodeFrame(long handle, int frame_type, unsigned char * in, unsigned char * out, int *nal_len);


/**
 * �رձ�����
 * @param handle�� �������ľ��
 */
void H264EncodeClose(long handle);

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*  x264 ����   *=*=*=*=*=*=*=*=*=*=*/

/*
�ӿں�����H264DecodeInit
��    �ƣ��人��ѧ�����ѧԺ���Ҷ�ý��������̼����о�����
��    �ܣ����һ·������
��    �أ��˺������ص���һ·������
*/
long H264DecodeInit();


/*
�ӿں�����H264DecodeFrame
��    �ƣ��人��ѧ�����ѧԺ���Ҷ�ý��������̼����о�����
��    �ܣ�����һ֡ͼ��һ֡�ֶ��Nalʱ����Ҫ��ƴ֡�ٵ����������
��    ����
		  handle��    ������
		  in:         ���������������
		  nallen��    nal���ĳ���
		  width:      ���ؽ����ͼ��Ŀ��
		  height:     ���ؽ����ͼ��ĸ߶�
��    �أ�  �˺����ķ���ֵ����0ʱ��������ɹ����õ�һ֡ͼ��
*/
int H264DecodeFrame(long handle, unsigned char * in, int nallen,int *width, int *height);


/*
�ӿں�����GetDecodePicture
��    �ƣ��人��ѧ�����ѧԺ���Ҷ�ý��������̼����о�����
��    �ܣ���ȡ����õ���һ֡ͼ��
��    ����
		  handle��    ������
		  out:        �洢����õ���һ֡ͼ��
��    �أ�  �˺����ķ���ֵ����0ʱ������ȡ�ɹ�
*/
int GetDecodePicture(long handle, unsigned char * out);


/*
�ӿں�����H264DecodeClose
��    �ƣ��人��ѧ�����ѧԺ���Ҷ�ý��������̼����о�����
��    �ܣ��ر�һ·������
��    ����
		  handle��������
��    �أ��˺����ķ���ֵΪ1����ʾ�ɹ��رս�����
*/
int H264DecodeClose(long handle);





//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*  x264 ƴ֡   *=*=*=*=*=*=*=*=*=*=*/

//����H264��ƴ֡��������ƴ֡�������ΪNULL�򴴽�ʧ��
void* CreateH264Packer();


//���ܣ������ɸ�H264 ��RTP��ƴ��һ֡H264����Ƶ�������ڽ����¼��
//void *handle���������������ƴ֡�����
//char *pPayload���������������RTP�����غ�
//int payloadlen���������������RTP�غɳ��ȣ��ֽڵ�λ��
//int bMark���������������RTP����mark��ǣ�Ϊ1ʱ���һ֡�����һ����
//int pts���������������RTP��ʱ�����ͬһ��Ƶ֡������RTP��������ͬ��ʱ���
//unsigned short sequence���������������RTP�����к�
//unsigned char *frmbuf���������������ƴ��һ֡�����ݻ�������������Ӧ�㹻�󣬿��԰�ȫ�ػ���һ֡����,���鿪64KB
//����ֵ��Ϊ0�������һ֡����δƴ�ꣻ��0�������ƴ�ɹ���һ֡���ݵĳ��ȣ��ֽڵ�λ��
int PackH264Frame(void *handle,unsigned char *pPayload,int payloadlen,int bMark,unsigned int pts,
				  unsigned short sequence,unsigned char *frmbuf);


//�������н���H264��֡����
//unsigned char *frmbuf����������������
//int len��������������
//����ֵ����I֡�򷵻�1�����򷵻�0
int isH264IntraFrame(unsigned char *frmbuf,int len);


//����H264��ƴ֡��������ƴ֡�����
void DestroyH264Packer(void *handle);

#ifdef __cplusplus
}
#endif

#endif