#ifndef _Load_Library_H_
#define _Load_Library_H_


#ifdef __cplusplus
extern "C" {
#endif

//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*  x264 编码   *=*=*=*=*=*=*=*=*=*=*/

/**
 * 初始化H.264编码器
 * @param width： 视频帧的宽度
 * @param height：视频帧的高度
 * @param qp：量化步长（在10~51之间取值）
 * @param slice_size：视频数据包的最大字节数目
 * @param IntraPeriod：关键帧的周期，即多少帧一个关键帧
 * @param bitrate：码率（单位：Kbit/s， 为0时关闭码率控制）
 * @param fps：帧率(同等码率下，帧率越高，质量越差，反之，帧率越低，质量越好)
 * @return：返回值为编码器的句柄
 */
long H264EncodeInit(int width, int height, int qp, int slice_size, int IntraPeriod, int bitrate, int fps);


/**
 * 编码一帧
 * @param handle： 编码器的句柄
 * @param frame_type：强制把当前帧编码为指定类型的帧（0：P帧  1:关键帧  -1:由编码器自动决策）
 * @param in：一帧YUV数据
 * @param out：一帧码流数据
 * @param nal_len：一帧分为多个片，该数组记录每个片的尺寸
 * @return：返回值为一帧码流所包含的片数目
 */
int H264EncodeFrame(long handle, int frame_type, unsigned char * in, unsigned char * out, int *nal_len);


/**
 * 关闭编码器
 * @param handle： 编码器的句柄
 */
void H264EncodeClose(long handle);





//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*  x264 解码   *=*=*=*=*=*=*=*=*=*=*/

/*
接口函数：H264DecodeInit
设    计：武汉大学计算机学院国家多媒体软件工程技术研究中心
功    能：获得一路解码器
返    回：此函数返回的是一路解码器
*/
long H264DecodeInit();


/*
接口函数：H264DecodeFrame
设    计：武汉大学计算机学院国家多媒体软件工程技术研究中心
功    能：解码一帧图像，一帧分多个Nal时，需要先拼帧再调用这个函数
参    数：
		  handle：    解码器
		  in:         待解码的输入数据
		  nallen：    nal包的长度
		  width:      返回解码后图像的宽度
		  height:     返回解码后图像的高度
返    回：  此函数的返回值大于0时表明解码成功并得到一帧图像
*/
int H264DecodeFrame(long handle, unsigned char * in, int nallen,int *width, int *height);


/*
接口函数：GetDecodePicture
设    计：武汉大学计算机学院国家多媒体软件工程技术研究中心
功    能：提取解码得到的一帧图像
参    数：
		  handle：    解码器
		  out:        存储解码得到的一帧图像
返    回：  此函数的返回值大于0时表明提取成功
*/
int GetDecodePicture(long handle, unsigned char * out);


/*
接口函数：H264DecodeClose
设    计：武汉大学计算机学院国家多媒体软件工程技术研究中心
功    能：关闭一路解码器
参    数：
		  handle：解码器
返    回：此函数的返回值为1，表示成功关闭解码器
*/
int H264DecodeClose(long handle);





//*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*  x264 拼帧   *=*=*=*=*=*=*=*=*=*=*/

//创建H264的拼帧器，返回拼帧器句柄，为NULL则创建失败
void* CreateH264Packer();


//功能：将若干个H264 的RTP包拼成一帧H264的视频流，便于解码和录像
//void *handle－－－输入参数，拼帧器句柄
//char *pPayload－－－输入参数，RTP包净载荷
//int payloadlen－－－输入参数，RTP载荷长度（字节单位）
//int bMark－－－输入参数，RTP包的mark标记，为1时标记一帧的最后一个包
//int pts－－－输入参数，RTP包时间戳，同一视频帧编码后的RTP包具有相同的时间戳
//unsigned short sequence－－－输入参数，RTP包序列号
//unsigned char *frmbuf－－－输入参数，拼成一帧的数据缓冲区。缓冲区应足够大，可以安全地缓存一帧数据,建议开64KB
//返回值：为0，则表明一帧数据未拼完；非0，则代表拼成功后一帧数据的长度（字节单位）
int PackH264Frame(void *handle,unsigned char *pPayload,int payloadlen,int bMark,unsigned int pts,
				  unsigned short sequence,unsigned char *frmbuf);


//从码流中解析H264的帧类型
//unsigned char *frmbuf－－－码流缓冲区
//int len－－－码流长度
//返回值：是I帧则返回1，否则返回0
int isH264IntraFrame(unsigned char *frmbuf,int len);


//销毁H264的拼帧器，输入拼帧器句柄
void DestroyH264Packer(void *handle);


#ifdef __cplusplus
}
#endif

#endif