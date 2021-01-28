
// TrafficMonitor.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������
#include "Common.h"
#include "IniHelper.h"
#include "WinVersionHelper.h"
#include "SimpleXML.h"
#include "TaskbarDefaultStyle.h"
#include <map>

// CTrafficMonitorApp: 
// �йش����ʵ�֣������ TrafficMonitor.cpp
//


class CTrafficMonitorApp : public CWinApp
{
public:
	//����·��
	static CTrafficMonitorApp* self;
	wstring m_module_dir;		//����exe�ļ���Ŀ¼
	wstring m_appdata_dir;
	wstring m_module_path;		//����exe�ļ���·��
	wstring m_module_path_reg;	//������Ϊд��ע������������exe�ļ���·�������·�����пո񣬼������ţ�
	wstring m_config_path;
	wstring m_log_path;
	wstring m_skin_path;
	wstring m_system_dir;
	wstring m_config_dir;

	//�������ݶ���ΪApp���еĹ�����Ա���Ա��������Ի���������������ж��ܷ���
	unsigned int m_in_speed{};		//�����ٶ�
	unsigned int m_out_speed{};		//�ϴ��ٶ�
	int m_cpu_usage{};		//CPU������
	int m_memory_usage{};	//�ڴ�������
	int m_used_memory{};	//���������ڴ棨��λΪKB��
	int m_total_memory{};	//�����ڴ���������λΪKB��

	__int64 m_today_up_traffic{};	//������ʹ�õ��ϴ�����
	__int64 m_today_down_traffic{};	//������ʹ�õ���������

	bool m_cannot_save_config_warning{ true };	//ָʾ�Ƿ�����޷���������ʱ������ʾ��
	bool m_cannot_save_global_config_warning{ true };	//ָʾ�Ƿ�����޷���������ʱ������ʾ��

    bool m_module_dir_writable{ true };         //ָʾ��������Ŀ¼�Ƿ��д

	//ѡ����������
	MainWndSettingData m_main_wnd_data;
	TaskBarSettingData m_taskbar_data;
	GeneralSettingData m_general_data;
	//������������
	MainConfigData m_cfg_data;
	int m_notify_interval;		//����֪ͨ��Ϣ��ʱ����
	bool m_debug_log{};
	bool m_taksbar_transparent_color_enable{};
	bool m_last_light_mode{};
	bool m_show_mouse_panetrate_tip{};	//�Ƿ���ʾ��������괩͸��ʱ����ʾ��Ϣ��

	//bool m_is_windows10_fall_creator;
	CWinVersionHelper m_win_version;		//��ǰWindows�İ汾

	HICON m_notify_icons[MAX_NOTIFY_ICON];

	CTaskbarDefaultStyle m_taskbar_default_style;

	HWND m_option_dlg{};		//ѡ�����öԻ���ľ��


    CMenu m_main_menu;
    CMenu m_taskbar_menu;

public:
	CTrafficMonitorApp();

	void LoadConfig();
	void SaveConfig();

	void LoadGlobalConfig();
	void SaveGlobalConfig();

	int DPI(int pixel);
	void DPI(CRect& rect);
	void GetDPI(CWnd* pWnd);

	void SetAutoRun(bool auto_run);
	bool GetAutoRun();

	//��ȡϵͳ��Ϣ�ı�
	CString GetSystemInfoString();

    void InitMenuResourse();

    //��ȡһ��ͼ����Դ�������Դ��δ���أ����Զ����ء�
    //���ڱ�������ʹ����CTrafficMonitorApp::DPI��������˱���������ȷ����CTrafficMonitorApp::GetDPI֮�����
    HICON GetMenuIcon(UINT id);

    void AutoSelectNotifyIcon();

private:
	//int m_no_multistart_warning_time{};		//���������ڿ�����೤ʱ���ڲ��������Ѿ���һ�������������С��ľ�����ʾ
	bool m_no_multistart_warning{};			//���Ϊfalse������Զ�����ᵯ�����Ѿ���һ�������������С��ľ�����ʾ
	bool m_exit_when_start_by_restart_manager{ true };		//�������Windows�����������������������˳�����
	int m_dpi{ 96 };
	CWinThread* m_pUpdateThread;			//�����µ��߳�

    std::map<UINT, HICON> m_menu_icons;      //�˵�ͼ����Դ��key��ͼ����Դ��ID��vlaue��ͼ��ľ��

// ��д
public:
	virtual BOOL InitInstance();

};

extern CTrafficMonitorApp theApp;