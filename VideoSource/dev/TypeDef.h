#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "Config.h"

namespace ScheduleServer
{
	//////////////////////////////////////////////////////////////////////////
	typedef struct tagLiveCastRequest
	{
		//string band;
		//string type;
		string url;

		tagLiveCastRequest() : /*band(""), type(""),*/ url("")
		{
		}

		~tagLiveCastRequest()
		{
		}

		bool operator < (tagLiveCastRequest const& _request) const
		{
			if(url < _request.url)
				return true;

			return false;
		}
	}
	LiveCastRequest;

	typedef struct tagLiveCastResponse
	{
		string num;
		string url;//hls
		string url2;//rtmp
		string expire;

		tagLiveCastResponse() : num(""), url(""), url2(""), expire("")
		{
		}

		~tagLiveCastResponse()
		{
		}

		bool operator < (tagLiveCastResponse const& _response) const
		{
			if(url < _response.url)
				return true;

			return false;
		}
	}
	LiveCastResponse;

	typedef struct tagVODRequest
	{
		string url;

		tagVODRequest() : url("")
		{
		}

		~tagVODRequest()
		{
		}

		bool operator < (tagVODRequest const& _request) const
		{
			if(url < _request.url)
				return true;

			return false;
		}
	}
	VODRequest;

	typedef struct tagVODResponse
	{
		string num;
		string url;//hls
		string url2;//rtmp
		string duration;

		tagVODResponse() : num(""), url(""), url2(""), duration("")
		{
		}

		~tagVODResponse()
		{
		}

		bool operator < (tagVODResponse const& _response) const
		{
			if(url < _response.url)
				return true;

			return false;
		}
	}
	VODResponse;
}

#endif // _TYPEDEF_H_
