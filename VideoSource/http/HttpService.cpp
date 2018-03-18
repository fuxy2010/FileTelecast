#include "StdAfx.h"
#include "HttpService.h"

using namespace HttpServer;

#define HTTP_SERVICE_STATUS_UNKNOW 0		// δ֪״̬
#define HTTP_SERVICE_STATUS_STARTING 1		// ������
#define HTTP_SERVICE_STATUS_RUNNING 2		// ������

class CCitmsErrorHandler : public Poco::ErrorHandler
{
public:
	CCitmsErrorHandler() {}
	/// Creates the ErrorHandler.

	virtual ~CCitmsErrorHandler() {}
	/// Destroys the ErrorHandler.

	virtual void exception(const Poco::Exception& exc) {
		// TODO
	}

	virtual void exception(const std::exception& exc) {
		// TODO
	}
};

#include "MiscTool.h"
#include "resource.h"
#include "Locker.h"
#include "TypeDef.h"
#include "json.h"
#include "scheduleserver.h"
#include "RTMPPushTask.h"
#include "VideoPullTask.h"

using namespace ScheduleServer;

static CCitmsErrorHandler * sErrorHandler = new CCitmsErrorHandler;

class CCitmsHttpRequestHandler : public Poco::Net::AbstractHTTPRequestHandler
{
public:
	CCitmsHttpRequestHandler()
	{
	}
	virtual ~CCitmsHttpRequestHandler() {}

private:
	static CSSMutex _http_mutex;

private:
	string return_response(int ErrCode, string ErrorDesc, string Data, unsigned long delayms = 0)
	{
		Json::Value json_value;
		json_value["ErrCode"] = Json::Value(ErrCode);
		json_value["ErrorDesc"] = Json::Value(ErrorDesc);
		json_value["Data"] = Json::Value(Data);

		if(!ErrCode)
			Sleep(delayms);

		Json::FastWriter fast_writer;
		return fast_writer.write(json_value);
	}

	string on_action(string url)
	{
		CSSLocker lock(&_http_mutex);
		//http://127.0.0.1:8080/action.html?id=123&action=left&arg=xxx

		{
			Poco::URI uri(url);

			/*std::string userInfo = uri.getUserInfo();
			//_username = userInfo.substr(0, userInfo.find(':'));

			if (userInfo.length() > _username.length())
			{
				_password = userInfo.substr(_username.length() + 1);
			}

			_ip = uri.getHost();
			_port = MiscTools::parse_string_to_type<unsigned short>(Poco::NumberFormatter::format(uri.getPort()));*/

			unsigned long id = 0;
			string action = "";
			string arg = "";

			auto paramsVector = uri.getQueryParameters();
			for (auto i = 0; i < paramsVector.size(); ++i)
			{
				std::string key = Poco::toLower(paramsVector[i].first);
				std::string val = paramsVector[i].second;

				if ("id" == key)
					id = MiscTools::parse_string_to_type<unsigned long>(val);
				else if ("action" == key)
					action  = val;
				else if ("arg" == key)
					arg = val;

				

				TRACE("\nKV: %s, %s", key.c_str(), val.c_str());
			}

			SINGLETON(CScheduleServer).on_mouse_action(action, arg);

			/*if(SINGLETON(CScheduleServer)._pull_task_map.end() != SINGLETON(CScheduleServer)._pull_task_map.find(id))
			{
				CVideoPullTask* task = dynamic_cast<CVideoPullTask*>(SINGLETON(CScheduleServer)._pull_task_map[id]);
			
				if(NULL != task) task->up();
			}*/
		}

		if(false)
		{
			return return_response(1002, "Invalid request.", "");
		}

		Json::Value json_value;
		json_value["Action"] = Json::Value("Done");

		Json::FastWriter fast_writer;
		return return_response(0, "success", fast_writer.write(json_value));
	}

protected:
	virtual void run()
	{
		{
			SINGLETON(HttpServer::CHttpService).query();
			//return;
		}

		string uri = "";
		Poco::URI::decode(request().getURI(), uri, false);

		SINGLETON(CScheduleServer).show_request(request().clientAddress().toString(),request().getMethod(), uri);

		if(request().getMethod() != Poco::Net::HTTPServerRequest::HTTP_GET)
		{
			sendErrorResponse(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST, "Please use GET method");
			return;
		}
		
		string res = "";

		res = on_action(uri);

		response().setStatusAndReason(Poco::Net::HTTPResponse::HTTP_OK);
		//Access-Control-Allow-Origin
		response().setAccessControl();
		response().setContentType("text/plain");		

		SINGLETON(CScheduleServer).show_response(res.data());

		response().sendBuffer(res.data(), res.size());
	}
};

CSSMutex CCitmsHttpRequestHandler::_http_mutex;

//map<unsigned long, CTask*> CCitmsHttpRequestHandler::_pull_task_map;
//map<unsigned long, CTask*> CCitmsHttpRequestHandler::_push_task_map;

class _CCitms_RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
	_CCitms_RequestHandlerFactory()
	{
	}

	Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request)
	{
		/*CStdString msg("Request from "
			+ request.clientAddress().toString()
			+ ": "
			+ request.getMethod()
			+ " "
			+ request.getURI()
			+ " "
			+ request.getVersion());
		*/
		return new CCitmsHttpRequestHandler;
	}

	virtual void handleRequest(Poco::Net::HTTPServerRequest& req, Poco::Net::HTTPServerResponse& resp)
	{
		resp.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
		resp.setContentType("text/html");
		
		ostream & out = resp.send();
		out << "<h1>Hello World!</h1>";
	}
};

CHttpService & CHttpService::GetInstance()
{
	static CHttpService instance;
	return instance;
}

CHttpService::~CHttpService()
{
	if (m_server) {
		m_server->stopAll(true);
		delete m_server;
		m_server = NULL;
	}
	if (m_timer) {
		m_timer->cancel(true);
		delete m_timer;
		m_timer = nullptr;
	}
}

void CHttpService::Initialize()
{
	Poco::ErrorHandler::set(sErrorHandler);
	m_state = HTTP_SERVICE_STATUS_UNKNOW;
	if (!m_timer)
		m_timer = new Poco::Util::Timer;

	Poco::Util::TimerTask::Ptr pTask = new Poco::Util::TimerTaskAdapter<CHttpService>(*this, &CHttpService::DoStart);
	m_timer->schedule(pTask, 2000, 5000);	// 5���ӳ���һ��
}

void CHttpService::Uninitialize()
{
	if (m_timer) {
		m_timer->cancel(true);
		delete m_timer;
		m_timer = nullptr;
	}
}

void CHttpService::Start(const std::string & ipAddr, int port)
{
	CSSLocker lock(&m_srvMutex);
	if ((m_ipAddr == ipAddr) && (m_port == port) || (m_server == NULL)) {
		// ������ȫһ�£��������ӻ��Ѿ��������ǲ������¿���
		if (m_state != HTTP_SERVICE_STATUS_UNKNOW) {
			return;
		}
	}
	m_ipAddr = ipAddr;
	m_port = port;

	// TODO log "���ڳ�����HTTP���񡭡�"
	// �ر����еķ���
	if (m_server) {
		m_server->stopAll(true);
		delete m_server;
		m_server = NULL;
	}

	m_state = HTTP_SERVICE_STATUS_STARTING;
}

void CHttpService::Stop()
{
	CSSLocker lock(&m_srvMutex);
	if (m_server) {
		m_server->stopAll(true);
		delete m_server;
		m_server = NULL;
	}
	// TODO log "��ֹͣHTTP����"
	m_state = HTTP_SERVICE_STATUS_UNKNOW;
}

CHttpService::CHttpService()
{

}

void CHttpService::DoStart(Poco::Util::TimerTask& task)
{
	CSSLocker lock(&m_srvMutex);
	if (m_state == HTTP_SERVICE_STATUS_STARTING) {
		try
		{
			Poco::Net::SocketAddress saddr(m_ipAddr, m_port);
			Poco::Net::ServerSocket svs(saddr);
			Poco::Net::HTTPServerParams * params_ptr = new Poco::Net::HTTPServerParams;
			params_ptr->setMaxQueued(500);
			params_ptr->setMaxThreads(10);
			m_server = new Poco::Net::HTTPServer(new _CCitms_RequestHandlerFactory, svs, params_ptr);
			m_server->start();
			m_state = HTTP_SERVICE_STATUS_RUNNING;
			//TODO log "�ɹ���HTTP����"
		}
		catch (const Poco::Exception& e) {
			//TODO log "HTTP������ʧ��: "
			if (m_server) {
				m_server->stopAll(true);
				delete m_server;
				m_server = NULL;
			}
		}
		catch (...) {
			if (m_server) {
				m_server->stopAll(true);
				delete m_server;
				m_server = NULL;
			}
		}
	}
}

void CHttpService::query()
{
	char status[256];
	memset(status, 0, sizeof(status));
	sprintf_s(status, "%d, %d, %d, %d, %d, %d", m_server->currentConnections(), m_server->maxConcurrentConnections(), m_server->currentThreads(), m_server->maxThreads(), m_server->queuedConnections(), m_server->refusedConnections());
	SINGLETON(CScheduleServer).show_log(status);
}
