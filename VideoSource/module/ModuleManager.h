#ifndef _MODULE_MANAGER_H_
#define _MODULE_MANAGER_H_

#include "IModule.h"
#include <windows.h>

namespace ScheduleServer
{
	class CModuleManager
	{
	public:
		// ���캯��
		CModuleManager();

		// ��������
		~CModuleManager();

		// ���ز��, ���ز��ָ��
		IModule* load(const std::string& module_file_path);

		// ���ݲ����ж�ز��
		bool unload(const std::string& module_file_path);

		// ж�����в��
		void unload_all();

		// ���ݲ�������Ҳ��ָ��
		IModule* get_module(const std::string& module_file_path);

		// ���ݲ�������Ҳ�����
		HMODULE get_module_handle(const std::string& module_file_path);

		// ��ȡ���ز��ָ���б�
		void get_module_list(IModulePtrList& miptr_list);

	private:
		// ��������б�
		std::map<HMODULE, IModule*> _module_ptr_map;
	};
}

#endif // _MODULE_MANAGER_H_
