#ifndef _DATETIME_H_        
#define _DATETIME_H_ 

#include "Config.h"
#include <time.h>
#include <sys\timeb.h>

namespace ScheduleServer
{
    // ����ʱ����Ϣ�ṹ����
    typedef struct _DATETIME_INFO
    {
        UINT year;                                                   // ��(1970-2100)
        UINT month;                                                  // ��(1-12)
        UINT day;                                                    // ��(1-31)
        UINT hour;                                                   // ʱ(0-23)
        UINT minute;                                                 // ��(0-59)
        UINT second;                                                 // ��(0-59)
    } DATETIME_INFO;

    namespace DateTime
    {
        // ��ʱ
        void sleep(ULONG millisec);

        // ����ʱ����Ϣ
        INT64 make_time(UINT year, UINT month, UINT day, UINT hour = 0, UINT minute = 0, UINT second = 0);

        // ת��ʱ��ֵ(��1970��1��1��0ʱ0��0�뵽��ʱ������)Ϊ����ʱ����Ϣ
        DATETIME_INFO convert_date(INT64 time);

        // ת������ʱ����ϢΪʱ��ֵ(��1970��1��1��0ʱ0��0�뵽��ʱ������)
        INT64 convert_time(const DATETIME_INFO& datetime_info);

        // ��ȡ��ǰʱ��ֵ(��1970��1��1��0ʱ0��0�뵽��ʱ������)
        INT64 get_current_time(void);

        // ��ȡ��ǰʱ��ֵ(��1970��1��1��0ʱ0��0�뵽��ʱ�ĺ�����)
        INT64 get_current_timestamp(void);
    }
}

using namespace ScheduleServer;

// ----------------------------------------------------------------------
// ������: DateTime::sleep
// ����: fuym
// ����: ��������ʱ
// ����ֵ: [void]
// ����: 
//   [INT millisec] ����ֵ
// ----------------------------------------------------------------------
inline void DateTime::sleep(ULONG millisec)
{
#ifdef _WIN32
    ::Sleep(millisec);
#else
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(millisec));
#endif
}

// ----------------------------------------------------------------------
// ������: DateTime::make_time
// ����: fuym
// ����: ����ʱ����Ϣ
// ����ֵ: [LONG] ���ʱ��ֵ(��1970��1��1��0ʱ0��0�뵽��ʱ������)
// ����: 
//   [INT year] ��
//   [INT month] ��
//   [INT day] ��
//   [INT hour] ʱ
//   [INT minute] ��
//   [INT second] ��
// ----------------------------------------------------------------------
inline INT64 DateTime::make_time(UINT year, UINT month, UINT day, UINT hour, UINT minute, UINT second)
{
    DATETIME_INFO dt_info;
    dt_info.year = year;
    dt_info.month = month;
    dt_info.day = day;
    dt_info.hour = hour;
    dt_info.minute = minute;
    dt_info.second = second;

    return convert_time(dt_info);
}

// ----------------------------------------------------------------------
// ������: DateTime::convert_date
// ����: fuym
// ����: ת��ʱ��ֵ(��1970��1��1��0ʱ0��0�뵽��ʱ������)Ϊ����ʱ����Ϣ
// ����ֵ: [DATETIME_INFO] �������ʱ����Ϣ
// ����: 
//   [LONG time] ����ʱ��ֵ(��)
// ----------------------------------------------------------------------
inline DATETIME_INFO DateTime::convert_date(INT64 time)
{
    DATETIME_INFO dt_info;

    time_t timep = (time_t)time;
    struct tm t;

#ifdef _WIN32
    localtime_s(&t, &timep);
#else
    struct tm tr;
    localtime_r(&time, &tr);
    t = tr;
#endif

    dt_info.year = t.tm_year + 1900;
    dt_info.month = t.tm_mon + 1;
    dt_info.day = t.tm_mday;
    dt_info.hour = t.tm_hour;
    dt_info.minute = t.tm_min;
    dt_info.second = t.tm_sec;

    return dt_info;
}

// ----------------------------------------------------------------------
// ������: DateTime::convert_time
// ����: fuym
// ����: ת������ʱ����ϢΪʱ��ֵ(��1970��1��1��0ʱ0��0�뵽��ʱ������)
// ����ֵ: [LONG] ���ʱ��ֵ(��)
// ����: 
//   [const DATETIME_INFO& datetime_info] ��������ʱ����Ϣ
// ----------------------------------------------------------------------
inline INT64 DateTime::convert_time(const DATETIME_INFO& datetime_info)
{
    struct tm t; 
    t.tm_year = datetime_info.year - 1900;
    t.tm_mon = datetime_info.month - 1; 
    t.tm_mday = datetime_info.day; 
    t.tm_hour = datetime_info.hour; 
    t.tm_min = datetime_info.minute; 
    t.tm_sec = datetime_info.second; 
    t.tm_isdst = 0; 

    return ::mktime(&t);
}

// ----------------------------------------------------------------------
// ������: DateTime::get_current_time
// ����: fuym
// ����: ��ȡ��ǰʱ��ֵ(��1970��1��1��0ʱ0��0�뵽��ʱ������)
// ����ֵ: [LONG] �����ǰʱ��ֵ(��)
// ����: 
//   [void]
// ----------------------------------------------------------------------
inline INT64 DateTime::get_current_time(void)
{
    return time(NULL);
}

// ----------------------------------------------------------------------
// ������: DateTime::get_current_timestamp
// ����: fuym
// ����: ��ȡ��ǰʱ��
// ����ֵ: [INT64] �����ǰʱ��ֵ(����)
// ����: 
//   [void]
// ----------------------------------------------------------------------
inline INT64 DateTime::get_current_timestamp(void)
{
    struct _timeb t;
    _ftime64_s(&t);
    return (t.time * 1000 + t.millitm);
}

#endif  // _DATETIME_H_         
