﻿
// TrafficMonitor.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "TrafficMonitor.h"
#include "TrafficMonitorDlg.h"
#include "crashtool.h"
#include "Test.h"
#include "WIC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CTrafficMonitorApp 构造
CTrafficMonitorApp* CTrafficMonitorApp::self = NULL;

CTrafficMonitorApp::CTrafficMonitorApp()
{
	self = this;
	// 支持重新启动管理器
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
	CRASHREPORT::StartCrashReport();
}

void CTrafficMonitorApp::LoadConfig()
{
	CIniHelper ini{ m_config_path };

	//常规设置
	m_general_data.language = static_cast<Language>(ini.GetInt(_T("general"), _T("language"), 0));
	m_general_data.show_all_interface = ini.GetBool(L"general", L"show_all_interface", false);
	//载入获取CPU利用率的方式，默认使用GetSystemTimes获取
	m_general_data.m_get_cpu_usage_by_cpu_times = ini.GetBool(L"general", L"get_cpu_usage_by_cpu_times", /*m_win_version.GetMajorVersion() < 10*/ true);
    m_general_data.monitor_time_span = ini.GetInt(L"general", L"monitor_time_span", 1000);
    if (m_general_data.monitor_time_span < MONITOR_TIME_SPAN_MIN || m_general_data.monitor_time_span > MONITOR_TIME_SPAN_MAX)
        m_general_data.monitor_time_span = 1000;

	//Windows10颜色模式设置
	bool is_windows10_light_theme = m_win_version.IsWindows10LightTheme();
	if (is_windows10_light_theme)
		CCommon::SetColorMode(ColorMode::Light);
	else
		CCommon::SetColorMode(ColorMode::Default);

	//主窗口设置
	m_cfg_data.m_show_notify_icon = ini.GetBool(_T("config"), _T("show_notify_icon"), false);
	m_cfg_data.m_show_task_bar_wnd = ini.GetBool(_T("config"), _T("show_task_bar_wnd"), true);
	m_cfg_data.m_position_x = ini.GetInt(_T("config"), _T("position_x"), -1);
	m_cfg_data.m_position_y = ini.GetInt(_T("config"), _T("position_y"), -1);
	m_cfg_data.m_auto_select = ini.GetBool(_T("connection"), _T("auto_select"), true);
	m_cfg_data.m_select_all = ini.GetBool(_T("connection"), _T("select_all"), false);
	m_cfg_data.m_connection_name = CCommon::UnicodeToStr(ini.GetString(L"connection", L"connection_name", L"").c_str());
	
	m_cfg_data.m_notify_icon_selected = ini.GetInt(_T("config"), _T("notify_icon_selected"), (m_win_version.IsWindows7() || m_win_version.IsWindows8Or8point1() ? 2 : m_cfg_data.m_dft_notify_icon));		//Win7/8/8.1默认使用蓝色通知区图标，因为隐藏通知区图标后白色图标会看不清，其他系统默认使用白色图标
    m_cfg_data.m_notify_icon_auto_adapt = ini.GetBool(_T("config"), _T("notify_icon_auto_adapt"), true);
    if(m_cfg_data.m_notify_icon_auto_adapt)
        AutoSelectNotifyIcon();
	FontInfo default_font{};
	default_font.name = CCommon::LoadText(IDS_DEFAULT_FONT);
	default_font.size = 10;


	m_general_data.traffic_tip_enable = ini.GetBool(L"notify_tip", L"traffic_tip_enable", false);
	m_general_data.traffic_tip_value = ini.GetInt(L"notify_tip", L"traffic_tip_value", 200);
	m_general_data.traffic_tip_unit = ini.GetInt(L"notify_tip", L"traffic_tip_unit", 0);
	m_general_data.memory_usage_tip_enable = ini.GetBool(L"notify_tip", L"memory_usage_tip_enable", false);
	m_general_data.memory_tip_value = ini.GetInt(L"notify_tip", L"memory_tip_value", 80);


	//任务栏窗口设置
	m_taskbar_data.back_color = ini.GetInt(_T("task_bar"), _T("task_bar_back_color"), m_taskbar_data.dft_back_color);
	m_taskbar_data.transparent_color = ini.GetInt(_T("task_bar"), _T("transparent_color"), m_taskbar_data.dft_transparent_color);
	if (CTaskbarDefaultStyle::IsTaskbarTransparent(m_taskbar_data))	//如果任务栏背景透明，则需要将颜色转换一下
	{
		CCommon::TransparentColorConvert(m_taskbar_data.back_color);
		CCommon::TransparentColorConvert(m_taskbar_data.transparent_color);
	}
	m_taskbar_data.status_bar_color = ini.GetInt(_T("task_bar"), _T("status_bar_color"), m_taskbar_data.dft_status_bar_color);
	ini.GetIntArray(_T("task_bar"), _T("task_bar_text_color"), (int*)m_taskbar_data.text_colors, TASKBAR_COLOR_NUM, m_taskbar_data.dft_text_colors);
	m_taskbar_data.specify_each_item_color = ini.GetBool(L"task_bar", L"specify_each_item_color", false);
	//m_cfg_data.m_tbar_show_cpu_memory = ini.GetBool(_T("task_bar"), _T("task_bar_show_cpu_memory"), false);
	m_cfg_data.m_tbar_display_item = ini.GetInt(L"task_bar", L"tbar_display_item", TDI_UP | TDI_DOWN);
	m_taskbar_data.swap_up_down = ini.GetBool(_T("task_bar"), _T("task_bar_swap_up_down"), false);

	if (m_taskbar_data.back_color == 0 && m_taskbar_data.text_colors[0] == 0)		//万一读取到的背景色和文本颜色都为0（黑色），则将文本色和背景色设置成默认颜色
	{
		m_taskbar_data.back_color = m_taskbar_data.dft_back_color;
		m_taskbar_data.text_colors[0] = m_taskbar_data.dft_text_colors;
	}
	default_font = FontInfo{};
	default_font.name = CCommon::LoadText(IDS_DEFAULT_FONT);
	default_font.size = 9;
	ini.LoadFontData(_T("task_bar"), m_taskbar_data.font, default_font);

	m_taskbar_data.disp_str.up = ini.GetString(L"task_bar", L"up_string", L"↑: $");
	m_taskbar_data.disp_str.down = ini.GetString(L"task_bar", L"down_string", L"↓: $");
	m_taskbar_data.disp_str.cpu = ini.GetString(L"task_bar", L"cpu_string", L"CPU: $");
	m_taskbar_data.disp_str.memory = ini.GetString(L"task_bar", L"memory_string", CCommon::LoadText(IDS_MEMORY_DISP, _T(": $")));

	m_taskbar_data.tbar_wnd_on_left = ini.GetBool(_T("task_bar"), _T("task_bar_wnd_on_left"), false);
	m_taskbar_data.speed_short_mode = ini.GetBool(_T("task_bar"), _T("task_bar_speed_short_mode"), false);
	m_taskbar_data.unit_byte = ini.GetBool(_T("task_bar"), _T("unit_byte"), true);
	m_taskbar_data.speed_unit = static_cast<SpeedUnit>(ini.GetInt(_T("task_bar"), _T("task_bar_speed_unit"), 0));
	m_taskbar_data.hide_unit = ini.GetBool(_T("task_bar"), _T("task_bar_hide_unit"), false);
	m_taskbar_data.hide_percent = ini.GetBool(_T("task_bar"), _T("task_bar_hide_percent"), false);
	m_taskbar_data.value_right_align = ini.GetBool(_T("task_bar"), _T("value_right_align"), false);
	m_taskbar_data.horizontal_arrange = ini.GetBool(_T("task_bar"), _T("horizontal_arrange"), false);
	m_taskbar_data.show_status_bar = ini.GetBool(_T("task_bar"), _T("show_status_bar"), false);
	m_taskbar_data.separate_value_unit_with_space = ini.GetBool(_T("task_bar"), _T("separate_value_unit_with_space"), true);
	m_taskbar_data.show_tool_tip = ini.GetBool(_T("task_bar"), _T("show_tool_tip"), false);
	m_taskbar_data.digits_number = ini.GetInt(_T("task_bar"), _T("digits_number"), 4);
	m_taskbar_data.double_click_action = static_cast<DoubleClickAction>(ini.GetInt(_T("task_bar"), _T("double_click_action"), 0));
	m_taskbar_data.double_click_exe = ini.GetString(L"task_bar", L"double_click_exe", (theApp.m_system_dir + L"\\Taskmgr.exe").c_str());
	m_taskbar_data.cm_graph_type = ini.GetBool(_T("task_bar"), _T("cm_graph_type"), false);

	if (m_win_version.IsWindows10OrLater())		//只有Win10才支持自动适应系统深色/浅色主题
		m_taskbar_data.auto_adapt_light_theme = ini.GetBool(L"task_bar", L"auto_adapt_light_theme", false);
	else
		m_taskbar_data.auto_adapt_light_theme = false;
	m_taskbar_data.dark_default_style = ini.GetInt(L"task_bar", L"dark_default_style", 0);
	m_taskbar_data.light_default_style = ini.GetInt(L"task_bar", L"light_default_style", TASKBAR_DEFAULT_LIGHT_STYLE_INDEX);

    if (m_win_version.IsWindows8OrLater())
        m_taskbar_data.auto_set_background_color = ini.GetBool(L"task_bar", L"auto_set_background_color", false);
    else
        m_taskbar_data.auto_set_background_color = false;

	//其他设置

	m_no_multistart_warning = ini.GetBool(_T("other"), _T("no_multistart_warning"), false);
	m_notify_interval = ini.GetInt(_T("other"), _T("notify_interval"), 60);
	m_exit_when_start_by_restart_manager = ini.GetBool(_T("other"), _T("exit_when_start_by_restart_manager"), true);
	m_debug_log = ini.GetBool(_T("other"), _T("debug_log"), false);
	//由于Win7系统中设置任务栏窗口透明色会导致任务栏窗口不可见，因此默认在Win7中禁用透明色的设定
	m_taksbar_transparent_color_enable = ini.GetBool(L"other", L"taksbar_transparent_color_enable", !m_win_version.IsWindows7());
	m_last_light_mode = ini.GetBool(L"other", L"last_light_mode", m_win_version.IsWindows10LightTheme());
}

void CTrafficMonitorApp::SaveConfig()
{
	CIniHelper ini{ m_config_path };

	//常规设置
	ini.WriteInt(_T("general"), _T("language"), static_cast<int>(m_general_data.language));
	ini.WriteBool(L"general", L"show_all_interface", m_general_data.show_all_interface);
	ini.WriteBool(L"general", L"get_cpu_usage_by_cpu_times", m_general_data.m_get_cpu_usage_by_cpu_times);
    ini.WriteInt(L"general", L"monitor_time_span", m_general_data.monitor_time_span);

	//主窗口设置
	ini.WriteBool(L"config", L"show_notify_icon", m_cfg_data.m_show_notify_icon);
	ini.WriteBool(L"config", L"show_task_bar_wnd", m_cfg_data.m_show_task_bar_wnd);
	ini.WriteInt(L"config", L"position_x", m_cfg_data.m_position_x);
	ini.WriteInt(L"config", L"position_y", m_cfg_data.m_position_y);
	ini.WriteBool(L"connection", L"auto_select", m_cfg_data.m_auto_select);
	ini.WriteBool(L"connection", L"select_all", m_cfg_data.m_select_all);
	ini.WriteString(L"connection", L"connection_name", CCommon::StrToUnicode(m_cfg_data.m_connection_name.c_str()).c_str());
	
	ini.WriteInt(L"config", L"notify_icon_selected", m_cfg_data.m_notify_icon_selected);
    ini.WriteBool(L"config", L"notify_icon_auto_adapt", m_cfg_data.m_notify_icon_auto_adapt);

	ini.WriteBool(L"notify_tip", L"traffic_tip_enable", m_general_data.traffic_tip_enable);
	ini.WriteInt(L"notify_tip", L"traffic_tip_value", m_general_data.traffic_tip_value);
	ini.WriteInt(L"notify_tip", L"traffic_tip_unit", m_general_data.traffic_tip_unit);
	ini.WriteBool(L"notify_tip", L"memory_usage_tip_enable", m_general_data.memory_usage_tip_enable);
	ini.WriteInt(L"notify_tip", L"memory_tip_value", m_general_data.memory_tip_value);


	//任务栏窗口设置
	ini.WriteInt(L"task_bar", L"task_bar_back_color", m_taskbar_data.back_color);
	ini.WriteInt(L"task_bar", L"transparent_color", m_taskbar_data.transparent_color);
	ini.WriteInt(L"task_bar", L"status_bar_color", m_taskbar_data.status_bar_color);
	ini.WriteIntArray(L"task_bar", L"task_bar_text_color", (int*)m_taskbar_data.text_colors, TASKBAR_COLOR_NUM);
	ini.WriteBool(L"task_bar", L"specify_each_item_color", m_taskbar_data.specify_each_item_color);
	//ini.WriteBool(L"task_bar", L"task_bar_show_cpu_memory", m_cfg_data.m_tbar_show_cpu_memory);
	ini.WriteInt(L"task_bar", L"tbar_display_item", m_cfg_data.m_tbar_display_item);
	ini.SaveFontData(L"task_bar", m_taskbar_data.font);
	ini.WriteBool(L"task_bar", L"task_bar_swap_up_down", m_taskbar_data.swap_up_down);

	ini.WriteString(_T("task_bar"), _T("up_string"), m_taskbar_data.disp_str.up);
	ini.WriteString(_T("task_bar"), _T("down_string"), m_taskbar_data.disp_str.down);
	ini.WriteString(_T("task_bar"), _T("cpu_string"), m_taskbar_data.disp_str.cpu);
	ini.WriteString(_T("task_bar"), _T("memory_string"), m_taskbar_data.disp_str.memory);

	ini.WriteBool(L"task_bar", L"task_bar_wnd_on_left", m_taskbar_data.tbar_wnd_on_left);
	ini.WriteBool(L"task_bar", L"task_bar_speed_short_mode", m_taskbar_data.speed_short_mode);
	ini.WriteBool(L"task_bar", L"unit_byte", m_taskbar_data.unit_byte);
	ini.WriteInt(L"task_bar", L"task_bar_speed_unit", static_cast<int>(m_taskbar_data.speed_unit));
	ini.WriteBool(L"task_bar", L"task_bar_hide_unit", m_taskbar_data.hide_unit);
	ini.WriteBool(L"task_bar", L"task_bar_hide_percent", m_taskbar_data.hide_percent);
	ini.WriteBool(L"task_bar", L"value_right_align", m_taskbar_data.value_right_align);
	ini.WriteBool(L"task_bar", L"horizontal_arrange", m_taskbar_data.horizontal_arrange);
	ini.WriteBool(L"task_bar", L"show_status_bar", m_taskbar_data.show_status_bar);
	ini.WriteBool(L"task_bar", L"separate_value_unit_with_space", m_taskbar_data.separate_value_unit_with_space);
	ini.WriteBool(L"task_bar", L"show_tool_tip", m_taskbar_data.show_tool_tip);
	ini.WriteInt(L"task_bar", L"digits_number", m_taskbar_data.digits_number);
	ini.WriteInt(L"task_bar", L"double_click_action", static_cast<int>(m_taskbar_data.double_click_action));
    ini.WriteString(L"task_bar", L"double_click_exe", m_taskbar_data.double_click_exe);
	ini.WriteBool(L"task_bar", L"cm_graph_type", m_taskbar_data.cm_graph_type);

	ini.WriteBool(L"task_bar", L"auto_adapt_light_theme", m_taskbar_data.auto_adapt_light_theme);
	ini.WriteInt(L"task_bar", L"dark_default_style", m_taskbar_data.dark_default_style);
	ini.WriteInt(L"task_bar", L"light_default_style", m_taskbar_data.light_default_style);
    ini.WriteBool(L"task_bar", L"auto_set_background_color", m_taskbar_data.auto_set_background_color);

	//其他设置

	ini.WriteBool(_T("other"), _T("no_multistart_warning"), m_no_multistart_warning);
	ini.WriteBool(_T("other"), _T("exit_when_start_by_restart_manager"), m_exit_when_start_by_restart_manager);
	ini.WriteBool(_T("other"), _T("debug_log"), m_debug_log);
	ini.WriteInt(_T("other"), _T("notify_interval"), m_notify_interval);
	ini.WriteBool(_T("other"), _T("taksbar_transparent_color_enable"), m_taksbar_transparent_color_enable);
	ini.WriteBool(_T("other"), _T("last_light_mode"), m_last_light_mode);
	ini.WriteString(L"app", L"version", VERSION);

	//检查是否保存成功
	if (!ini.Save())
	{
		if (m_cannot_save_config_warning)
		{
			CString info;
			info.LoadString(IDS_CONNOT_SAVE_CONFIG_WARNING);
			info.Replace(_T("<%file_path%>"), m_config_path.c_str());
			AfxMessageBox(info, MB_ICONWARNING);
		}
		m_cannot_save_config_warning = false;
		return;
	}
}

void CTrafficMonitorApp::LoadGlobalConfig()
{
	bool portable_mode_default{ false };
	wstring global_cfg_path{ m_module_dir + L"global_cfg.ini" };
	if (!CCommon::FileExist(global_cfg_path.c_str()))		//如果global_cfg.ini不存在，则根据AppData/Roaming/TrafficMonitor目录下是否存在config.ini来判断配置文件的保存位置
	{
		portable_mode_default = !CCommon::FileExist((m_appdata_dir + L"config.ini").c_str());
	}

	CIniHelper ini{ global_cfg_path };
	m_general_data.portable_mode = ini.GetBool(L"config", L"portable_mode", portable_mode_default);

    //执行一次保存操作，以检查当前目录是否有写入权限
    m_module_dir_writable = ini.Save();

    if (m_module_dir.find(CCommon::GetTemplateDir()) != wstring::npos)      //如果当前路径是在Temp目录下，则强制将数据保存到Appdata
    {
        m_module_dir_writable = false;
    }

    if (!m_module_dir_writable)              //如果当前目录没有写入权限，则设置配置保存到AppData目录
    {
        m_general_data.portable_mode = false;
    }
}

void CTrafficMonitorApp::SaveGlobalConfig()
{
	CIniHelper ini{ m_module_dir + L"global_cfg.ini" };
	ini.WriteBool(L"config", L"portable_mode", m_general_data.portable_mode);
}

int CTrafficMonitorApp::DPI(int pixel)
{
	return m_dpi * pixel / 96;
}

void CTrafficMonitorApp::DPI(CRect & rect)
{
	rect.left = DPI(rect.left);
	rect.right = DPI(rect.right);
	rect.top = DPI(rect.top);
	rect.bottom = DPI(rect.bottom);
}

void CTrafficMonitorApp::GetDPI(CWnd* pWnd)
{
	CWindowDC dc(pWnd);
	HDC hDC = dc.GetSafeHdc();
	m_dpi = GetDeviceCaps(hDC, LOGPIXELSY);
}


void CTrafficMonitorApp::SetAutoRun(bool auto_run)
{
	CRegKey key;
	//打开注册表项
	if (key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run")) != ERROR_SUCCESS)
	{
		AfxMessageBox(CCommon::LoadText(IDS_AUTORUN_FAILED_NO_KEY), MB_OK | MB_ICONWARNING);
		return;
	}
	if (auto_run)		//写入注册表项
	{
		if (key.SetStringValue(_T("TrafficMonitor"), m_module_path_reg.c_str()) != ERROR_SUCCESS)
		{
			AfxMessageBox(CCommon::LoadText(IDS_AUTORUN_FAILED_NO_ACCESS), MB_OK | MB_ICONWARNING);
			return;
		}
	}
	else		//删除注册表项
	{
		//删除前先检查注册表项是否存在，如果不存在，则直接返回
		wchar_t buff[256];
		ULONG size{ 256 };
		if (key.QueryStringValue(_T("TrafficMonitor"), buff, &size) != ERROR_SUCCESS)
			return;
		if (key.DeleteValue(_T("TrafficMonitor")) != ERROR_SUCCESS)
		{
			AfxMessageBox(CCommon::LoadText(IDS_AUTORUN_DELETE_FAILED), MB_OK | MB_ICONWARNING);
			return;
		}
	}
}

bool CTrafficMonitorApp::GetAutoRun()
{
	CRegKey key;
	if (key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run")) != ERROR_SUCCESS)
	{
		//打开注册表“Software\\Microsoft\\Windows\\CurrentVersion\\Run”失败，则返回false
		return false;
	}
	wchar_t buff[256];
	ULONG size{ 256 };
	if (key.QueryStringValue(_T("TrafficMonitor"), buff, &size) == ERROR_SUCCESS)		//如果找到了“TrafficMonitor”键
	{
		return (m_module_path_reg == buff);	//如果“TrafficMonitor”的值是当前程序的路径，就返回true，否则返回false
	}
	else
	{
		return false;		//没有找到“TrafficMonitor”键，返回false
	}
}

CString CTrafficMonitorApp::GetSystemInfoString()
{
	CString info;
	info += _T("System Info:\r\n");

	CString strTmp;
	strTmp.Format(_T("Windows Version: %d.%d build %d\r\n"), m_win_version.GetMajorVersion(),
		m_win_version.GetMinorVersion(), m_win_version.GetBuildNumber());
	info += strTmp;

	strTmp.Format(_T("DPI: %d"), m_dpi);
	info += strTmp;
	info += _T("\r\n");

	return info;
}


void CTrafficMonitorApp::InitMenuResourse()
{
    //载入菜单
    m_main_menu.LoadMenu(IDR_MENU1);
    m_taskbar_menu.LoadMenu(IDR_TASK_BAR_MENU);

    //为菜单项添加图标
    //主窗口右键菜单
    CMenuIcon::AddIconToMenuItem(m_main_menu.GetSubMenu(0)->GetSafeHmenu(), 0, TRUE, GetMenuIcon(IDI_CONNECTION));
    CMenuIcon::AddIconToMenuItem(m_main_menu.GetSubMenu(0)->GetSafeHmenu(), 12, TRUE, GetMenuIcon(IDI_FUNCTION));
    CMenuIcon::AddIconToMenuItem(m_main_menu.GetSafeHmenu(), ID_NETWORK_INFO, FALSE, GetMenuIcon(IDI_INFO));
    CMenuIcon::AddIconToMenuItem(m_main_menu.GetSafeHmenu(), ID_SHOW_NOTIFY_ICON, FALSE, GetMenuIcon(IDI_NOTIFY));
    CMenuIcon::AddIconToMenuItem(m_main_menu.GetSafeHmenu(), ID_SHOW_TASK_BAR_WND, FALSE, GetMenuIcon(IDI_TASKBAR_WINDOW));
    CMenuIcon::AddIconToMenuItem(m_main_menu.GetSafeHmenu(), ID_OPTIONS, FALSE, GetMenuIcon(IDI_SETTINGS));
    CMenuIcon::AddIconToMenuItem(m_main_menu.GetSafeHmenu(), ID_APP_EXIT, FALSE, GetMenuIcon(IDI_EXIT));

    //任务栏窗口右键菜单
    CMenuIcon::AddIconToMenuItem(m_taskbar_menu.GetSubMenu(0)->GetSafeHmenu(), 0, TRUE, GetMenuIcon(IDI_CONNECTION));
    CMenuIcon::AddIconToMenuItem(m_taskbar_menu.GetSafeHmenu(), ID_NETWORK_INFO, FALSE, GetMenuIcon(IDI_INFO));
    CMenuIcon::AddIconToMenuItem(m_taskbar_menu.GetSubMenu(0)->GetSafeHmenu(), 5, TRUE, GetMenuIcon(IDI_ITEM));
    CMenuIcon::AddIconToMenuItem(m_taskbar_menu.GetSafeHmenu(), ID_SHOW_NOTIFY_ICON, FALSE, GetMenuIcon(IDI_NOTIFY));
    CMenuIcon::AddIconToMenuItem(m_taskbar_menu.GetSafeHmenu(), ID_SHOW_TASK_BAR_WND, FALSE, GetMenuIcon(IDI_CLOSE));
    CMenuIcon::AddIconToMenuItem(m_taskbar_menu.GetSafeHmenu(), ID_OPTIONS2, FALSE, GetMenuIcon(IDI_SETTINGS));
    CMenuIcon::AddIconToMenuItem(m_taskbar_menu.GetSafeHmenu(), ID_APP_EXIT, FALSE, GetMenuIcon(IDI_EXIT));

}

HICON CTrafficMonitorApp::GetMenuIcon(UINT id)
{
    auto iter = m_menu_icons.find(id);
    if (iter != m_menu_icons.end())
    {
        return iter->second;
    }
    else
    {
        HICON hIcon = CCommon::LoadIconResource(id, DPI(16));
        m_menu_icons[id] = hIcon;
        return hIcon;
    }
}

void CTrafficMonitorApp::AutoSelectNotifyIcon()
{
    if (m_win_version.GetMajorVersion() >= 10)
    {
        bool light_mode = m_win_version.IsWindows10LightTheme();
        if (light_mode)     //浅色模式下，如果图标是白色，则改成黑色
        {
            if (m_cfg_data.m_notify_icon_selected == 0)
                m_cfg_data.m_notify_icon_selected = 4;
            if (m_cfg_data.m_notify_icon_selected == 1)
                m_cfg_data.m_notify_icon_selected = 5;
        }
        else     //深色模式下，如果图标是黑色，则改成白色
        {
            if (m_cfg_data.m_notify_icon_selected == 4)
                m_cfg_data.m_notify_icon_selected = 0;
            if (m_cfg_data.m_notify_icon_selected == 5)
                m_cfg_data.m_notify_icon_selected = 1;
        }
    }
}

// 唯一的一个 CTrafficMonitorApp 对象

CTrafficMonitorApp theApp;


// CTrafficMonitorApp 初始化

BOOL CTrafficMonitorApp::InitInstance()
{
	//CSimpleXML xml(L"C:\\Temp\\xmldoc_test1.xml");
	//wstring str;
	//str = xml.GetNode(L"panorama", L"face1");

	//设置配置文件的路径
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	m_module_path = path;
	if (m_module_path.find(L' ') != wstring::npos)
	{
		//如果路径中有空格，则在程序路径前后添加双引号
		m_module_path_reg = L'\"';
		m_module_path_reg += m_module_path;
		m_module_path_reg += L'\"';
	}
	else
	{
		m_module_path_reg = m_module_path;
	}
	m_module_dir = CCommon::GetModuleDir();
	m_system_dir = CCommon::GetSystemDir();
	m_appdata_dir = CCommon::GetAppDataConfigDir();

	LoadGlobalConfig();

#ifdef _DEBUG
	m_config_dir = L".\\";
	m_skin_path = L".\\skins";
#else
	if (m_general_data.portable_mode)
		m_config_dir = m_module_dir;
	else
		m_config_dir = m_appdata_dir;
	m_skin_path = m_module_dir + L"skins";
#endif
	//AppData里面的程序配置文件路径
	m_config_path = m_config_dir + L"config.ini";
	m_log_path = m_config_dir + L"error.log";

	bool is_windows10_fall_creator = m_win_version.IsWindows10FallCreatorOrLater();

	//从ini文件载入设置
	LoadConfig();

	//初始化界面语言
	CCommon::SetThreadLanguage(m_general_data.language);

#ifndef _DEBUG
	//wstring cmd_line{ m_lpCmdLine };
	//bool is_restart{ cmd_line.find(L"RestartByRestartManager") != wstring::npos };		//如果命令行参数中含有字符串“RestartByRestartManager”则说明程序是被Windows重新启动的
	////bool when_start{ CCommon::WhenStart(m_no_multistart_warning_time) };
	//if (m_exit_when_start_by_restart_manager && is_restart && is_windows10_fall_creator)		//当前Windows版本是秋季创意者更新时，如果程序被重新启动，则直接退出程序
	//{
	//	//AfxMessageBox(_T("调试信息：程序已被Windows的重启管理器重新启动。"));
	//	return FALSE;
	//}

	//检查是否已有实例正在运行
	HANDLE hMutex = ::CreateMutex(NULL, TRUE, _T("TrafficMonitor-1419J3XLKL1w8OZc"));
	if (hMutex != NULL)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			//char buff[128];
			//string cmd_line_str{ CCommon::UnicodeToStr(cmd_line.c_str()) };
			//sprintf_s(buff, "when_start=%d, m_no_multistart_warning=%d, cmd_line=%s", when_start, m_no_multistart_warning, cmd_line_str.c_str());
			//CCommon::WriteLog(buff, _T(".\\start.log"));
			if (!m_no_multistart_warning)
			{
				AfxMessageBox(CCommon::LoadText(IDS_AN_INSTANCE_RUNNING));
			}
			return FALSE;
		}
	}
#endif

	m_taskbar_default_style.LoadConfig();

	//SaveConfig();

	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));


    //执行测试代码
#ifdef _DEBUG
    CTest::Test();
#endif

	CTrafficMonitorDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

