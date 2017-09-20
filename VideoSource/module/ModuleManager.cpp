#include "ModuleManager.h"
#include <iostream>

using namespace ScheduleServer;

CModuleManager::CModuleManager()
{
    _module_ptr_map.clear();
}

CModuleManager::~CModuleManager()
{
    unload_all();
}

IModule* CModuleManager::load(const std::string& module_file_path)
{
    HMODULE module = ::LoadLibrary(module_file_path.c_str());
    
	if(NULL == module)//���ز��ʧ��
    {
		LPVOID lpMsgBuf; 

        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
            NULL, 
            GetLastError(), 
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language 
            (LPTSTR)&lpMsgBuf, 
            0, 
            NULL 
            );

		std::cout << "\nFail in LoadLibrary! file: " << module_file_path << std::endl
			<< (LPCTSTR)lpMsgBuf << ", error code: " << GetLastError() << std::endl;

        LocalFree(lpMsgBuf);

        return NULL;
    }

	//��ӳ���в��Ҳ����ָ��
	std::map<HMODULE, IModule*>::iterator iter = _module_ptr_map.find(module);

	//����ҵ�ֱ�ӷ���
	if(_module_ptr_map.end() != iter)
		return iter->second;

	//���û�ҵ�
	IModule* module_ptr = NULL;

	//��ȡ�豸����ģ�����Ľӿں���ָ��
	FARPROC far_proc = ::GetProcAddress(module, DLL_EXPORT_FUNCTION_NAME);

	if(far_proc)
	{
		PLUG_IN_MODULE_API module_api = (PLUG_IN_MODULE_API)far_proc;

		module_ptr = module_api();//�����豸����ģ������Ӧ�������

		if(NULL == module_ptr)
		{
			::FreeLibrary(module);
			return NULL;
		}
	}
	else
	{
		::FreeLibrary(module);
		return NULL;
	}

	//�������󴴽��ɹ�

	// ���ݲ����ж���Ѽ��صĲ��
	unload(module_ptr->get_module_description().file_path);

	// ��������̬���ӿ��ļ�
	module_ptr->get_module_description().file_path = module_file_path;

	// �����ʼ��
	module_ptr->start();

	//�������б�
	_module_ptr_map[module] = module_ptr;//��ʱmodule_ptrһ����ΪNULL

	return module_ptr;
}

void CModuleManager::unload_all()
{
	for(std::map<HMODULE, IModule*>::iterator iter = _module_ptr_map.begin();
		iter != _module_ptr_map.end();
		++iter)
	{
		IModule* module_ptr = iter->second;

		if(NULL  != module_ptr)
		{
			module_ptr->shutdown();
			delete module_ptr;
			module_ptr = NULL;
		}

		::FreeLibrary(iter->first);
	}

	_module_ptr_map.clear();

}

bool CModuleManager::unload(const std::string& module_file_path)
{
	for(std::map<HMODULE, IModule*>::iterator iter = _module_ptr_map.begin();
		iter != _module_ptr_map.end();
		++iter)
	{
		IModule* module_ptr = iter->second;

		if(NULL == module_ptr)
		{
			::FreeLibrary(iter->first);

			_module_ptr_map.erase(iter);

			iter = _module_ptr_map.begin();//�����¿�ʼ��

			continue;
		}

		if(module_file_path == module_ptr->get_module_description().file_path)
		{
			module_ptr->shutdown();
			delete module_ptr;
			module_ptr = NULL;

			::FreeLibrary(iter->first);

			_module_ptr_map.erase(iter);

			return true;
		}
	}

    return false;
}

IModule* CModuleManager::get_module(const std::string& module_file_path)
{
	for(std::map<HMODULE, IModule*>::iterator iter = _module_ptr_map.begin();
		iter != _module_ptr_map.end();
		++iter)
	{
		IModule* module_ptr = iter->second;

		if(NULL == module_ptr)
		{
			::FreeLibrary(iter->first);

			_module_ptr_map.erase(iter);

			iter = _module_ptr_map.begin();//�����¿�ʼ��

			continue;
		}

		if(module_file_path == module_ptr->get_module_description().file_path)
			return module_ptr;
	}

	return NULL;
}

HMODULE CModuleManager::get_module_handle(const std::string& module_file_path)
{
	for(std::map<HMODULE, IModule*>::iterator iter = _module_ptr_map.begin();
		iter != _module_ptr_map.end();
		++iter)
	{
		IModule* module_ptr = iter->second;

		if(NULL == module_ptr)
		{
			::FreeLibrary(iter->first);

			_module_ptr_map.erase(iter);

			iter = _module_ptr_map.begin();//�����¿�ʼ��

			continue;
		}

		if(module_file_path == module_ptr->get_module_description().file_path)
			return iter->first;
	}

    return NULL;
}

void CModuleManager::get_module_list(IModulePtrList& miptr_list)
{
	std::map<HMODULE, IModulePtr>::iterator iter;
    for (iter = _module_ptr_map.begin(); iter != _module_ptr_map.end(); iter++)
    {
        miptr_list.push_back(iter->second);
    }
}

