
// TrafficMonitorDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TrafficMonitor.h"
#include "TrafficMonitorDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CTrafficMonitorDlg 对话框

//静态成员初始化
unsigned int CTrafficMonitorDlg::m_WM_TASKBARCREATED{ ::RegisterWindowMessage(_T("TaskbarCreated")) };	//注册任务栏建立的消息

CTrafficMonitorDlg::CTrafficMonitorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TRAFFICMONITOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_desktop_dc = ::GetDC(NULL);
}

CTrafficMonitorDlg::~CTrafficMonitorDlg()
{
	free(m_pIfTable);

	if (m_tBarDlg != nullptr)
	{
		delete m_tBarDlg;
		m_tBarDlg = nullptr;
	}

    ::ReleaseDC(NULL, m_desktop_dc);
}

void CTrafficMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_UP, m_disp_up);
	DDX_Control(pDX, IDC_STATIC_DOWN, m_disp_down);
	DDX_Control(pDX, IDC_STATIC_CPU, m_disp_cpu);
	DDX_Control(pDX, IDC_STATIC_MEMORY, m_disp_memory);
}

BEGIN_MESSAGE_MAP(CTrafficMonitorDlg, CDialogEx)
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_NETWORK_INFO, &CTrafficMonitorDlg::OnNetworkInfo)
	ON_WM_INITMENUPOPUP()
	ON_WM_CLOSE()
	ON_WM_INITMENU()
	ON_COMMAND(ID_LOCK_WINDOW_POS, &CTrafficMonitorDlg::OnLockWindowPos)
	ON_UPDATE_COMMAND_UI(ID_LOCK_WINDOW_POS, &CTrafficMonitorDlg::OnUpdateLockWindowPos)
	ON_WM_MOVE()
	ON_MESSAGE(MY_WM_NOTIFYICON, &CTrafficMonitorDlg::OnNotifyIcon)
	ON_COMMAND(ID_SHOW_NOTIFY_ICON, &CTrafficMonitorDlg::OnShowNotifyIcon)
	ON_UPDATE_COMMAND_UI(ID_SHOW_NOTIFY_ICON, &CTrafficMonitorDlg::OnUpdateShowNotifyIcon)
	ON_WM_DESTROY()
	ON_COMMAND(ID_SHOW_CPU_MEMORY, &CTrafficMonitorDlg::OnShowCpuMemory)
	ON_UPDATE_COMMAND_UI(ID_SHOW_CPU_MEMORY, &CTrafficMonitorDlg::OnUpdateShowCpuMemory)
	ON_COMMAND(ID_MOUSE_PENETRATE, &CTrafficMonitorDlg::OnMousePenetrate)
	ON_COMMAND(ID_SHOW_TASK_BAR_WND, &CTrafficMonitorDlg::OnShowTaskBarWnd)
	ON_COMMAND(ID_SHOW_CPU_MEMORY2, &CTrafficMonitorDlg::OnShowCpuMemory2)
	ON_REGISTERED_MESSAGE(m_WM_TASKBARCREATED, &CTrafficMonitorDlg::OnTaskBarCreated)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_OPTIONS, &CTrafficMonitorDlg::OnOptions)
	ON_COMMAND(ID_OPTIONS2, &CTrafficMonitorDlg::OnOptions2)
	ON_MESSAGE(WM_EXITMENULOOP, &CTrafficMonitorDlg::OnExitmenuloop)
	ON_MESSAGE(WM_TASKBAR_MENU_POPED_UP, &CTrafficMonitorDlg::OnTaskbarMenuPopedUp)
	ON_COMMAND(ID_SHOW_NET_SPEED, &CTrafficMonitorDlg::OnShowNetSpeed)
	ON_WM_QUERYENDSESSION()
END_MESSAGE_MAP()


void CTrafficMonitorDlg::ShowInfo()
{
	CString str;
	CString in_speed = CCommon::DataSizeToString(theApp.m_in_speed, theApp.m_main_wnd_data);
	CString out_speed = CCommon::DataSizeToString(theApp.m_out_speed, theApp.m_main_wnd_data);

	CString format_str;
	if (theApp.m_main_wnd_data.hide_unit && theApp.m_main_wnd_data.speed_unit != SpeedUnit::AUTO)
		format_str = _T("%s%s");
	else
		format_str = _T("%s%s/s");
	if (!theApp.m_main_wnd_data.swap_up_down)
	{
		str.Format(format_str, (m_layout_data.no_text ? _T("") : theApp.m_main_wnd_data.disp_str.up.c_str()), out_speed.GetString());
		m_disp_up.SetWindowTextEx(str, (theApp.m_cfg_data.m_show_more_info ? m_layout_data.up_align_l : m_layout_data.up_align_s));
		str.Format(format_str, (m_layout_data.no_text ? _T("") : theApp.m_main_wnd_data.disp_str.down.c_str()), in_speed.GetString());
		m_disp_down.SetWindowTextEx(str, (theApp.m_cfg_data.m_show_more_info ? m_layout_data.down_align_l : m_layout_data.down_align_s));
	}
	else		//交换上传和下载位置
	{
		str.Format(format_str, (m_layout_data.no_text ? _T("") : theApp.m_main_wnd_data.disp_str.down.c_str()), in_speed.GetString());
		m_disp_up.SetWindowTextEx(str, (theApp.m_cfg_data.m_show_more_info ? m_layout_data.up_align_l : m_layout_data.up_align_s));
		str.Format(format_str, (m_layout_data.no_text ? _T("") : theApp.m_main_wnd_data.disp_str.up.c_str()), out_speed.GetString());
		m_disp_down.SetWindowTextEx(str, (theApp.m_cfg_data.m_show_more_info ? m_layout_data.down_align_l : m_layout_data.down_align_s));
	}
	if (theApp.m_main_wnd_data.hide_percent)
		format_str = _T("%s%d");
    else if(theApp.m_main_wnd_data.separate_value_unit_with_space)
        format_str = _T("%s%d %%");
	else
		format_str = _T("%s%d%%");
	str.Format(format_str, (m_layout_data.no_text ? _T("") : theApp.m_main_wnd_data.disp_str.cpu.c_str()), theApp.m_cpu_usage);
	m_disp_cpu.SetWindowTextEx(str, (theApp.m_cfg_data.m_show_more_info ? m_layout_data.cpu_align_l : m_layout_data.cpu_align_s));
	str.Format(format_str, (m_layout_data.no_text ? _T("") : theApp.m_main_wnd_data.disp_str.memory.c_str()), theApp.m_memory_usage);
	m_disp_memory.SetWindowTextEx(str, (theApp.m_cfg_data.m_show_more_info ? m_layout_data.memory_align_l : m_layout_data.memory_align_s));
	//设置要显示的项目
	if (theApp.m_cfg_data.m_show_more_info)
	{
		m_disp_up.ShowWindow(m_layout_data.show_up_l ? SW_SHOW : SW_HIDE);
		m_disp_down.ShowWindow(m_layout_data.show_down_l ? SW_SHOW : SW_HIDE);
		m_disp_cpu.ShowWindow(m_layout_data.show_cpu_l ? SW_SHOW : SW_HIDE);
		m_disp_memory.ShowWindow(m_layout_data.show_memory_l ? SW_SHOW : SW_HIDE);
	}
	else
	{
		m_disp_up.ShowWindow(m_layout_data.show_up_s ? SW_SHOW : SW_HIDE);
		m_disp_down.ShowWindow(m_layout_data.show_down_s ? SW_SHOW : SW_HIDE);
		m_disp_cpu.ShowWindow(m_layout_data.show_cpu_s ? SW_SHOW : SW_HIDE);
		m_disp_memory.ShowWindow(m_layout_data.show_memory_s ? SW_SHOW : SW_HIDE);
	}
}

CString CTrafficMonitorDlg::GetMouseTipsInfo()
{
	CString tip_info;
	CString temp;
	temp.Format(_T("%s: %s (%s: %s/%s: %s)"), CCommon::LoadText(IDS_TRAFFIC_USED_TODAY),
		CCommon::KBytesToString(static_cast<unsigned int>((theApp.m_today_up_traffic + theApp.m_today_down_traffic)/ 1024)),
		CCommon::LoadText(IDS_UPLOAD), CCommon::KBytesToString(static_cast<unsigned int>(theApp.m_today_up_traffic / 1024)),
		CCommon::LoadText(IDS_DOWNLOAD), CCommon::KBytesToString(static_cast<unsigned int>(theApp.m_today_down_traffic / 1024))
		);
	tip_info += temp;
	if (theApp.m_cfg_data.m_show_more_info)
	{
		if (!m_layout_data.show_up_l)		//如果主窗口中没有显示上传速度，则在提示信息中显示上传速度
		{
			temp.Format(_T("\r\n%s: %s/s"), CCommon::LoadText(IDS_UPLOAD),
				CCommon::DataSizeToString(theApp.m_out_speed, theApp.m_main_wnd_data));
			tip_info += temp;
		}
		if (!m_layout_data.show_down_l)
		{
			temp.Format(_T("\r\n%s: %s/s"), CCommon::LoadText(IDS_DOWNLOAD),
				CCommon::DataSizeToString(theApp.m_in_speed, theApp.m_main_wnd_data));
			tip_info += temp;
		}
		if (!m_layout_data.show_cpu_l)
		{
			temp.Format(_T("\r\n%s: %d%%"), CCommon::LoadText(IDS_CPU_USAGE), theApp.m_cpu_usage);
			tip_info += temp;
		}
		if (!m_layout_data.show_memory_l)
		{
			temp.Format(_T("\r\n%s: %s/%s (%d%%)"), CCommon::LoadText(IDS_MEMORY_USAGE),
				CCommon::KBytesToString(theApp.m_used_memory),
				CCommon::KBytesToString(theApp.m_total_memory), theApp.m_memory_usage);
			tip_info += temp;
		}
		else
		{
			temp.Format(_T("\r\n%s: %s/%s"), CCommon::LoadText(IDS_MEMORY_USAGE),
				CCommon::KBytesToString(theApp.m_used_memory),
				CCommon::KBytesToString(theApp.m_total_memory));
			tip_info += temp;
		}
	}
	else
	{
		if (!m_layout_data.show_up_s)		//如果主窗口中没有显示上传速度，则在提示信息中显示上传速度
		{
			temp.Format(_T("\r\n%s: %s/s"), CCommon::LoadText(IDS_UPLOAD),
				CCommon::DataSizeToString(theApp.m_out_speed, theApp.m_main_wnd_data));
			tip_info += temp;
		}
		if (!m_layout_data.show_down_s)
		{
			temp.Format(_T("\r\n%s: %s/s"), CCommon::LoadText(IDS_DOWNLOAD),
				CCommon::DataSizeToString(theApp.m_in_speed, theApp.m_main_wnd_data));
			tip_info += temp;
		}
		if (!m_layout_data.show_cpu_s)
		{
			temp.Format(_T("\r\n%s: %d%%"), CCommon::LoadText(IDS_CPU_USAGE), theApp.m_cpu_usage);
			tip_info += temp;
		}
		if (!m_layout_data.show_memory_s)
		{
			temp.Format(_T("\r\n%s: %s/%s (%d%%)"), CCommon::LoadText(IDS_MEMORY_USAGE),
				CCommon::KBytesToString(theApp.m_used_memory),
				CCommon::KBytesToString(theApp.m_total_memory), theApp.m_memory_usage);
			tip_info += temp;
		}
		else
		{
			temp.Format(_T("\r\n%s: %s/%s"), CCommon::LoadText(IDS_MEMORY_USAGE),
				CCommon::KBytesToString(theApp.m_used_memory),
				CCommon::KBytesToString(theApp.m_total_memory));
			tip_info += temp;
		}
	}
	return tip_info;
}


void CTrafficMonitorDlg::GetScreenSize()
{
    m_screen_size.cx = GetSystemMetrics(SM_CXSCREEN);
    m_screen_size.cy = GetSystemMetrics(SM_CYSCREEN);

	::SystemParametersInfo(SPI_GETWORKAREA, 0, &m_screen_rect, 0);   // 获得工作区大小
}


void CTrafficMonitorDlg::AutoSelect()
{
	unsigned int max_in_out_bytes{};
	unsigned int in_out_bytes;
	//m_connection_selected = m_connections[0].index;
	m_connection_selected = 0;
	//自动选择连接时，查找已发送和已接收字节数之和最多的那个连接，并将其设置为当前查看的连接
	for (size_t i{}; i<m_connections.size(); i++)
	{
		if (m_pIfTable->table[m_connections[i].index].dwOperStatus == IF_OPER_STATUS_OPERATIONAL)		//只选择网络状态为正常的连接
		{
			in_out_bytes = m_pIfTable->table[m_connections[i].index].dwInOctets + m_pIfTable->table[m_connections[i].index].dwOutOctets;
			if (in_out_bytes > max_in_out_bytes)
			{ 
				max_in_out_bytes = in_out_bytes;
				m_connection_selected = i;
			}
		}
	}
	theApp.m_cfg_data.m_connection_name = m_connections[m_connection_selected].description_2;
	m_connection_change_flag = true;
}

void CTrafficMonitorDlg::IniConnection()
{
	//为m_pIfTable开辟所需大小的内存
	free(m_pIfTable);
	m_dwSize = sizeof(MIB_IFTABLE);
	m_pIfTable = (MIB_IFTABLE *)malloc(m_dwSize);
	int rtn;
	rtn = GetIfTable(m_pIfTable, &m_dwSize, FALSE);
	if (rtn == ERROR_INSUFFICIENT_BUFFER)	//如果函数返回值为ERROR_INSUFFICIENT_BUFFER，说明m_pIfTable的大小不够
	{
		free(m_pIfTable);
		m_pIfTable = (MIB_IFTABLE *)malloc(m_dwSize);	//用新的大小重新开辟一块内存
	}
	//获取当前所有的连接，并保存到m_connections容器中
	GetIfTable(m_pIfTable, &m_dwSize, FALSE);
	if (!theApp.m_general_data.show_all_interface)
	{
		CAdapterCommon::GetAdapterInfo(m_connections);
		CAdapterCommon::GetIfTableInfo(m_connections, m_pIfTable);
	}
	else
	{
		CAdapterCommon::GetAllIfTableInfo(m_connections, m_pIfTable);
	}

	//如果在设置了“显示所有网络连接”时设置了“选择全部”，则改为“自动选择”
	if (theApp.m_general_data.show_all_interface && theApp.m_cfg_data.m_select_all)
	{
		theApp.m_cfg_data.m_select_all = false;
		theApp.m_cfg_data.m_auto_select = true;
	}

	//写入调试日志
	if (theApp.m_debug_log)
	{
		CString log_str;
		log_str += _T("正在初始化网络连接...\n");
		log_str += _T("连接列表：\n");
		for (size_t i{}; i < m_connections.size(); i++)
		{
			log_str += m_connections[i].description.c_str();
			log_str += _T(", ");
			log_str += CCommon::IntToString(m_connections[i].index);
			log_str += _T("\n");
		}
		log_str += _T("IfTable:\n");
		for (size_t i{}; i < m_pIfTable->dwNumEntries; i++)
		{
			log_str += CCommon::IntToString(i);
			log_str += _T(" ");
			log_str += (const char*)m_pIfTable->table[i].bDescr;
			log_str += _T("\n");
		}
		CCommon::WriteLog(log_str, (theApp.m_config_dir + L".\\connections.log").c_str());
	}

	//if (m_connection_selected < 0 || m_connection_selected >= m_connections.size() || m_auto_select)
	//	AutoSelect();
	//选择网络连接
	if (theApp.m_cfg_data.m_auto_select)	//自动选择
	{
		if (m_restart_cnt != -1)	//当m_restart_cnt不等于-1时，即不是第一次初始化时，需要延时5秒再重新初始化连接
		{
			KillTimer(DELAY_TIMER);
			SetTimer(DELAY_TIMER, 5000, NULL);
		}
		else
		{
			AutoSelect();
		}
	}
	else		//查找网络名为上次选择的连接
	{
		m_connection_selected = 0;
		for (size_t i{}; i < m_connections.size(); i++)
		{
			if (m_connections[i].description_2 == theApp.m_cfg_data.m_connection_name)
				m_connection_selected = i;
		}
	}
	if (m_connection_selected < 0 || m_connection_selected >= m_connections.size())
		m_connection_selected = 0;
	theApp.m_cfg_data.m_connection_name = m_connections[m_connection_selected].description_2;

	//根据已获取到的连接在菜单中添加相应项目
    CMenu* select_connection_menu = theApp.m_main_menu.GetSubMenu(0)->GetSubMenu(0);		//设置“选择网络连接”子菜单项
	IniConnectionMenu(select_connection_menu);		//向“选择网卡”子菜单项添加项目

	IniTaskBarConnectionMenu();		//初始化任务栏窗口中的“选择网络连接”子菜单项

	m_restart_cnt++;	//记录初始化次数
	m_connection_change_flag = true;
}

void CTrafficMonitorDlg::IniConnectionMenu(CMenu * pMenu)
{
    ASSERT(pMenu != nullptr);
    if (pMenu != nullptr)
    {
        //先将ID_SELECT_ALL_CONNECTION后面的所有菜单项删除
        int start_pos = CCommon::GetMenuItemPosition(pMenu, ID_SELECT_ALL_CONNECTION) + 1;
        while (pMenu->GetMenuItemCount() > start_pos)
        {
            pMenu->DeleteMenu(start_pos, MF_BYPOSITION);
        }

        CString connection_descr;
        for (size_t i{}; i < m_connections.size(); i++)
        {
            connection_descr = CCommon::StrToUnicode(m_connections[i].description.c_str()).c_str();
            pMenu->AppendMenu(MF_STRING | MF_ENABLED, ID_SELECT_ALL_CONNECTION + i + 1, connection_descr);
        }
    }
}

void CTrafficMonitorDlg::IniTaskBarConnectionMenu()
{
	CMenu* select_connection_menu = theApp.m_taskbar_menu.GetSubMenu(0)->GetSubMenu(0);		//设置“选择网络连接”子菜单项
	IniConnectionMenu(select_connection_menu);		//向“选择网卡”子菜单项添加项目
}

void CTrafficMonitorDlg::SetConnectionMenuState(CMenu * pMenu)
{
	if (theApp.m_cfg_data.m_select_all)
		pMenu->CheckMenuRadioItem(0, m_connections.size() + 1, 1, MF_BYPOSITION | MF_CHECKED);
	else if (theApp.m_cfg_data.m_auto_select)		//m_auto_select为true时为自动选择，选中菜单的第1项
		pMenu->CheckMenuRadioItem(0, m_connections.size() + 1, 0, MF_BYPOSITION | MF_CHECKED);
	else		//m_auto_select为false时非自动选择，根据m_connection_selected的值选择对应的项
		pMenu->CheckMenuRadioItem(0, m_connections.size() + 1, m_connection_selected + 2, MF_BYPOSITION | MF_CHECKED);
}

void CTrafficMonitorDlg::CloseTaskBarWnd()
{
	if (m_tBarDlg != nullptr)
	{
		HWND hParent = ::GetParent(theApp.m_option_dlg);
		if (hParent == m_tBarDlg->GetSafeHwnd())			//关闭任务栏窗口前，如果选项设置窗口已打开且父窗口是任务栏窗口，则将其关闭
		{
			::SendMessage(theApp.m_option_dlg, WM_COMMAND, IDCANCEL, 0);
		}

        if(IsTaskbarWndValid())
            m_tBarDlg->OnCancel();
		delete m_tBarDlg;
		m_tBarDlg = nullptr;
	}
}

void CTrafficMonitorDlg::OpenTaskBarWnd()
{
	m_tBarDlg = new CTaskBarDlg;
	m_tBarDlg->Create(IDD_TASK_BAR_DIALOG, this);
	m_tBarDlg->ShowWindow(SW_SHOW);
	//m_tBarDlg->ShowInfo();
	//IniTaskBarConnectionMenu();
}

void CTrafficMonitorDlg::AddNotifyIcon()
{
	if (theApp.m_cfg_data.m_show_task_bar_wnd)
		CloseTaskBarWnd();
	//添加通知栏图标
	::Shell_NotifyIcon(NIM_ADD, &m_ntIcon);
	if (theApp.m_cfg_data.m_show_task_bar_wnd)
		OpenTaskBarWnd();
}

void CTrafficMonitorDlg::DeleteNotifyIcon()
{
	if (theApp.m_cfg_data.m_show_task_bar_wnd)
		CloseTaskBarWnd();
	//删除通知栏图标
	::Shell_NotifyIcon(NIM_DELETE, &m_ntIcon);
	if (theApp.m_cfg_data.m_show_task_bar_wnd)
		OpenTaskBarWnd();
}

void CTrafficMonitorDlg::ShowNotifyTip(const wchar_t * title, const wchar_t * message)
{
	//要显示通知区提示，必须先将通知区图标显示出来
	if (!theApp.m_cfg_data.m_show_notify_icon)
	{
		//添加通知栏图标
		AddNotifyIcon();
		theApp.m_cfg_data.m_show_notify_icon = true;
	}
	//显示通知提示
	m_ntIcon.uFlags |= NIF_INFO;
	//wcscpy_s(m_ntIcon.szInfo, message ? message : _T(""));
	//wcscpy_s(m_ntIcon.szInfoTitle, title ? title : _T(""));
	CCommon::WStringCopy(m_ntIcon.szInfo, 256, message);
	CCommon::WStringCopy(m_ntIcon.szInfoTitle, 64, title);
	::Shell_NotifyIcon(NIM_MODIFY, &m_ntIcon);
	m_ntIcon.uFlags &= ~NIF_INFO;
}

void CTrafficMonitorDlg::UpdateNotifyIconTip()
{
	CString strTip;			//鼠标指向图标时显示的提示
#ifdef _DEBUG
	strTip = CCommon::LoadText(IDS_TRAFFICMONITOR, _T(" (Debug)"));
#else
	strTip = CCommon::LoadText(IDS_TRAFFICMONITOR);
#endif

	CString in_speed = CCommon::DataSizeToString(theApp.m_in_speed);
	CString out_speed = CCommon::DataSizeToString(theApp.m_out_speed);

	strTip += CCommon::StringFormat(_T("\r\n<%1%>: <%2%>/s"), { CCommon::LoadText(IDS_UPLOAD), out_speed });
	strTip += CCommon::StringFormat(_T("\r\n<%1%>: <%2%>/s"), { CCommon::LoadText(IDS_DOWNLOAD), in_speed });
	strTip += CCommon::StringFormat(_T("\r\nCPU: <%1%>%"), { theApp.m_cpu_usage });
	strTip += CCommon::StringFormat(_T("\r\n<%1%>: <%2%>%"), { CCommon::LoadText(IDS_MEMORY), theApp.m_memory_usage });

	CCommon::WStringCopy(m_ntIcon.szTip, 128, strTip);
	::Shell_NotifyIcon(NIM_MODIFY, &m_ntIcon);

}



void CTrafficMonitorDlg::_OnOptions(int tab)
{
	COptionsDlg optionsDlg(tab);
	//将选项设置数据传递给选项设置对话框
	//optionsDlg.m_tab1_dlg.m_data = theApp.m_main_wnd_data;
	optionsDlg.m_tab_taskbar_dlg.m_data = theApp.m_taskbar_data;
	optionsDlg.m_tab_general_dlg.m_data = theApp.m_general_data;
	//optionsDlg.m_tab1_dlg.m_text_disable = m_layout_data.no_text;

	if (optionsDlg.DoModal() == IDOK)
	{
	//	theApp.m_main_wnd_data = optionsDlg.m_tab1_dlg.m_data;
		theApp.m_taskbar_data = optionsDlg.m_tab_taskbar_dlg.m_data;
		theApp.m_general_data = optionsDlg.m_tab_general_dlg.m_data;

		ApplySettings();

		//CTaskBarDlg::SaveConfig();
		if (IsTaskbarWndValid())
		{
			m_tBarDlg->ApplySettings();
			//如果更改了任务栏窗口字体或显示的文本，则任务栏窗口可能要变化，于是关闭再打开任务栏窗口
			CloseTaskBarWnd();
			OpenTaskBarWnd();
		}

		if(optionsDlg.m_tab_general_dlg.IsAutoRunModified())
			theApp.SetAutoRun(theApp.m_general_data.auto_run);

		if (optionsDlg.m_tab_general_dlg.IsShowAllInterfaceModified())
			IniConnection();

        if (optionsDlg.m_tab_general_dlg.IsMonitorTimeSpanModified())      //如果监控时间间隔改变了，则重设定时器
        {
            KillTimer(MONITOR_TIMER);
            SetTimer(MONITOR_TIMER, theApp.m_general_data.monitor_time_span, NULL);
        }

		//设置获取CPU利用率的方式
		m_cpu_usage.SetUseCPUTimes(theApp.m_general_data.m_get_cpu_usage_by_cpu_times);

		theApp.SaveConfig();
		theApp.SaveGlobalConfig();
	}
}

void CTrafficMonitorDlg::SetItemPosition()
{
	if (theApp.m_cfg_data.m_show_more_info)
	{
		SetWindowPos(nullptr, 0, 0, m_layout_data.width_l, m_layout_data.height_l, SWP_NOMOVE | SWP_NOZORDER);
		m_disp_up.MoveWindow(m_layout_data.up_x_l, m_layout_data.up_y_l, m_layout_data.up_width_l, m_layout_data.text_height);
		m_disp_down.MoveWindow(m_layout_data.down_x_l, m_layout_data.down_y_l, m_layout_data.down_width_l, m_layout_data.text_height);
		m_disp_cpu.MoveWindow(m_layout_data.cpu_x_l, m_layout_data.cpu_y_l, m_layout_data.cpu_width_l, m_layout_data.text_height);
		m_disp_memory.MoveWindow(m_layout_data.memory_x_l, m_layout_data.memory_y_l, m_layout_data.memory_width_l, m_layout_data.text_height);
	}
	else
	{
		SetWindowPos(nullptr, 0, 0, m_layout_data.width_s, m_layout_data.height_s, SWP_NOMOVE | SWP_NOZORDER);
		m_disp_up.MoveWindow(m_layout_data.up_x_s, m_layout_data.up_y_s, m_layout_data.up_width_s, m_layout_data.text_height);
		m_disp_down.MoveWindow(m_layout_data.down_x_s, m_layout_data.down_y_s, m_layout_data.down_width_s, m_layout_data.text_height);
		m_disp_cpu.MoveWindow(m_layout_data.cpu_x_s, m_layout_data.cpu_y_s, m_layout_data.cpu_width_s, m_layout_data.text_height);
		m_disp_memory.MoveWindow(m_layout_data.memory_x_s, m_layout_data.memory_y_s, m_layout_data.memory_width_s, m_layout_data.text_height);
	}
}

void CTrafficMonitorDlg::SetTextColor()
{
	int text_colors[MAIN_WND_COLOR_NUM];
	if (theApp.m_main_wnd_data.specify_each_item_color)
	{
		for (int i{}; i < MAIN_WND_COLOR_NUM; i++)
			text_colors[i] = theApp.m_main_wnd_data.text_colors[i];
	}
	else
	{
		for (int i{}; i < MAIN_WND_COLOR_NUM; i++)
			text_colors[i] = theApp.m_main_wnd_data.text_colors[0];
	}

	m_disp_up.SetTextColor(text_colors[0]);
	m_disp_down.SetTextColor(text_colors[1]);
	m_disp_cpu.SetTextColor(text_colors[2]);
	m_disp_memory.SetTextColor(text_colors[3]);
}

void CTrafficMonitorDlg::SetTextFont()
{
	if (m_font.m_hObject)	//如果m_font已经关联了一个字体资源对象，则释放它
		m_font.DeleteObject();
	m_font.CreateFont(
		FONTSIZE_TO_LFHEIGHT(theApp.m_main_wnd_data.font.size), // nHeight
		0, // nWidth
		0, // nEscapement
		0, // nOrientation
		(theApp.m_main_wnd_data.font.bold ? FW_BOLD : FW_NORMAL), // nWeight
		theApp.m_main_wnd_data.font.italic, // bItalic
		theApp.m_main_wnd_data.font.underline, // bUnderline
		theApp.m_main_wnd_data.font.strike_out, // cStrikeOut
		DEFAULT_CHARSET, // nCharSet
		OUT_DEFAULT_PRECIS, // nOutPrecision
		CLIP_DEFAULT_PRECIS, // nClipPrecision
		DEFAULT_QUALITY, // nQuality
		DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily
		theApp.m_main_wnd_data.font.name);
	m_disp_cpu.SetFont(&m_font);
	m_disp_memory.SetFont(&m_font);
	m_disp_up.SetFont(&m_font);
	m_disp_down.SetFont(&m_font);
}

bool CTrafficMonitorDlg::IsTaskbarWndValid() const
{
    return m_tBarDlg != nullptr && ::IsWindow(m_tBarDlg->GetSafeHwnd());
}

void CTrafficMonitorDlg::ApplySettings()
{
	//应用文字颜色设置
	SetTextColor();
	//应用字体设置
	SetTextFont();
}

// CTrafficMonitorDlg 消息处理程序

BOOL CTrafficMonitorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	SetWindowText(_T("TrafficMonitor"));
	//设置隐藏任务栏图标
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);

	theApp.GetDPI(this);
	//获取屏幕大小
	GetScreenSize();

    theApp.InitMenuResourse();

	IniConnection();	//初始化连接

	//如果启动时没有显示任务栏窗口，则显示通知区图标
	if (!theApp.m_cfg_data.m_show_task_bar_wnd)
		theApp.m_cfg_data.m_show_notify_icon = true;

	//载入通知区图标
	theApp.m_notify_icons[0] = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_NOFITY_ICON), IMAGE_ICON, theApp.DPI(16), theApp.DPI(16), LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);
	theApp.m_notify_icons[1] = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_NOFITY_ICON2), IMAGE_ICON, theApp.DPI(16), theApp.DPI(16), LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);
	theApp.m_notify_icons[2] = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_NOFITY_ICON3), IMAGE_ICON, theApp.DPI(16), theApp.DPI(16), LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);
	theApp.m_notify_icons[3] = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, theApp.DPI(16), theApp.DPI(16), LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);
	theApp.m_notify_icons[4] = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_NOFITY_ICON4), IMAGE_ICON, theApp.DPI(16), theApp.DPI(16), LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);
	theApp.m_notify_icons[5] = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_NOTIFY_ICON5), IMAGE_ICON, theApp.DPI(16), theApp.DPI(16), LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);

	//设置通知区域图标
	m_ntIcon.cbSize = sizeof(NOTIFYICONDATA);	//该结构体变量的大小
	if (theApp.m_cfg_data.m_notify_icon_selected < 0 || theApp.m_cfg_data.m_notify_icon_selected >= MAX_NOTIFY_ICON)
		theApp.m_cfg_data.m_notify_icon_selected = 0;
	m_ntIcon.hIcon = theApp.m_notify_icons[theApp.m_cfg_data.m_notify_icon_selected];		//设置图标
	m_ntIcon.hWnd = this->m_hWnd;				//接收托盘图标通知消息的窗口句柄
	CString atip;			//鼠标指向图标时显示的提示
#ifdef _DEBUG
	atip = CCommon::LoadText(IDS_TRAFFICMONITOR, _T(" (Debug)"));
#else
	atip = CCommon::LoadText(IDS_TRAFFICMONITOR);
#endif
	//wcscpy_s(m_ntIcon.szTip, 128, strTip);
	CCommon::WStringCopy(m_ntIcon.szTip, 128, atip.GetString());
	m_ntIcon.uCallbackMessage = MY_WM_NOTIFYICON;	//应用程序定义的消息ID号
	m_ntIcon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;	//图标的属性：设置成员uCallbackMessage、hIcon、szTip有效
	if (theApp.m_cfg_data.m_show_notify_icon)
		::Shell_NotifyIcon(NIM_ADD, &m_ntIcon);	//在系统通知区域增加这个图标

	//设置1000毫秒触发的定时器
	SetTimer(MAIN_TIMER, 1000, NULL);

    SetTimer(MONITOR_TIMER, theApp.m_general_data.monitor_time_span, NULL);

	//初始化皮肤
	CCommon::GetFiles(theApp.m_skin_path.c_str(), m_skins);
	if (m_skins.empty())
		m_skins.push_back(L"");
	m_skin_selected = 0;
	for (unsigned int i{}; i<m_skins.size(); i++)
	{
		if (m_skins[i] == theApp.m_cfg_data.m_skin_name)
			m_skin_selected = i;
	}


	//初始化窗口位置
	SetItemPosition();
	if (theApp.m_cfg_data.m_position_x != -1 && theApp.m_cfg_data.m_position_y != -1)
		SetWindowPos(nullptr, theApp.m_cfg_data.m_position_x, theApp.m_cfg_data.m_position_y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	//设置字体
	SetTextFont();

	//设置文字颜色
	SetTextColor();

	//获取启动时的时间
	GetLocalTime(&m_start_time);

	//初始化鼠标提示
	m_tool_tips.Create(this, TTS_ALWAYSTIP);
	m_tool_tips.SetMaxTipWidth(600);
	m_tool_tips.AddTool(this, _T(""));

	//设置获取CPU利用率的方式
	m_cpu_usage.SetUseCPUTimes(theApp.m_general_data.m_get_cpu_usage_by_cpu_times);

	SetTimer(TASKBAR_TIMER, 100, NULL);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CTrafficMonitorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//计算指定秒数的时间内Monitor定时器会触发的次数
static int GetMonitorTimerCount(int second)
{
    return second * 1000 / theApp.m_general_data.monitor_time_span;
}

void CTrafficMonitorDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == MONITOR_TIMER)
	{
		//获取网络连接速度
		int rtn = GetIfTable(m_pIfTable, &m_dwSize, FALSE);
		if (!theApp.m_cfg_data.m_select_all)		//获取当前选中连接的网速
		{
			m_in_bytes = m_pIfTable->table[m_connections[m_connection_selected].index].dwInOctets;
			m_out_bytes = m_pIfTable->table[m_connections[m_connection_selected].index].dwOutOctets;
		}
		else		//获取全部连接的网速
		{
			m_in_bytes = 0;
			m_out_bytes = 0;
			for (size_t i{}; i<m_connections.size(); i++)
			{
				//if (i > 0 && m_pIfTable->table[m_connections[i].index].dwInOctets == m_pIfTable->table[m_connections[i - 1].index].dwInOctets
				//	&& m_pIfTable->table[m_connections[i].index].dwOutOctets == m_pIfTable->table[m_connections[i - 1].index].dwOutOctets)
				//	continue;		//连接列表中可能会有相同的连接，统计所有连接的网速时，忽略掉已发送和已接收字节数完全相同的连接
				m_in_bytes += m_pIfTable->table[m_connections[i].index].dwInOctets;
				m_out_bytes += m_pIfTable->table[m_connections[i].index].dwOutOctets;
			}
		}

        unsigned __int64 cur_in_speed{}, cur_out_speed{};       //本次监控时间间隔内的上传和下载速度

		//如果发送和接收的字节数为0或上次发送和接收的字节数为0或当前连接已改变时，网速无效
		if ((m_in_bytes == 0 && m_out_bytes == 0) || (m_last_in_bytes == 0 && m_last_out_bytes == 0) || m_connection_change_flag
            || m_last_in_bytes > m_in_bytes || m_last_out_bytes > m_out_bytes)
		{
			cur_in_speed = 0;
			cur_out_speed = 0;
		}
		else
		{
			cur_in_speed = m_in_bytes - m_last_in_bytes;
			cur_out_speed = m_out_bytes - m_last_out_bytes;
		}
		////如果大于1GB/s，说明可能产生了异常，网速无效
		//if (cur_in_speed > 1073741824)
		//	cur_in_speed = 0;
		//if (cur_out_speed > 1073741824)
		//	cur_out_speed = 0;

        //将当前监控时间间隔的流量转换成每秒时间间隔内的流量
        theApp.m_in_speed = static_cast<unsigned int>(cur_in_speed * 1000 / theApp.m_general_data.monitor_time_span);
        theApp.m_out_speed = static_cast<unsigned int>(cur_out_speed * 1000 / theApp.m_general_data.monitor_time_span);

		m_connection_change_flag = false;	//清除连接发生变化的标志

		m_last_in_bytes = m_in_bytes;
		m_last_out_bytes = m_out_bytes;

		//处于自动选择状态时，如果连续30秒没有网速，则可能自动选择的网络不对，此时执行一次自动选择
		if (theApp.m_cfg_data.m_auto_select)
		{
			if (cur_in_speed == 0 && cur_out_speed == 0)
				m_zero_speed_cnt++;
			else
				m_zero_speed_cnt = 0;
			if (m_zero_speed_cnt >= GetMonitorTimerCount(30))
			{
				AutoSelect();
				m_zero_speed_cnt = 0;
			}
		}

		if (rtn == ERROR_INSUFFICIENT_BUFFER)
		{
			IniConnection();
			CString info;
			info.LoadString(IDS_INSUFFICIENT_BUFFER);
			info.Replace(_T("<%cnt%>"), CCommon::IntToString(m_restart_cnt));
			CCommon::WriteLog(info, theApp.m_log_path.c_str());
		}

		
        if (m_monitor_time_cnt % GetMonitorTimerCount(3) == GetMonitorTimerCount(3) - 1)
		{
			//重新获取当前连接数量
			static DWORD last_interface_num = -1;
			DWORD interface_num;
			GetNumberOfInterfaces(&interface_num);
			if (last_interface_num != -1 && interface_num != last_interface_num)	//如果连接数发生变化，则重新初始化连接
			{
				CString info;
				info.LoadString(IDS_CONNECTION_NUM_CHANGED);
				info.Replace(_T("<%before%>"), CCommon::IntToString(last_interface_num));
				info.Replace(_T("<%after%>"), CCommon::IntToString(interface_num));
				info.Replace(_T("<%cnt%>"), CCommon::IntToString(m_restart_cnt + 1));
				IniConnection();
				CCommon::WriteLog(info, theApp.m_log_path.c_str());
			}
			last_interface_num = interface_num;

			string descr;
			descr = (const char*)m_pIfTable->table[m_connections[m_connection_selected].index].bDescr;
			if (descr != theApp.m_cfg_data.m_connection_name)
			{
				//写入额外的调试信息
				if (theApp.m_debug_log)
				{
					CString log_str;
					log_str = _T("连接名称不匹配：\r\n");
					log_str += _T("IfTable description: ");
					log_str += descr.c_str();
					log_str += _T("\r\nm_connection_name: ");
					log_str += theApp.m_cfg_data.m_connection_name.c_str();
					CCommon::WriteLog(log_str, (theApp.m_config_dir + L".\\connections.log").c_str());
				}

				IniConnection();
				CString info;
				info.LoadString(IDS_CONNECTION_NOT_MATCH);
				info.Replace(_T("<%cnt%>"), CCommon::IntToString(m_restart_cnt));
				CCommon::WriteLog(info, theApp.m_log_path.c_str());
			}
		}

		////只有主窗口和任务栏窗口至少有一个显示时才执行下面的处理
		//if (!theApp.m_cfg_data.m_hide_main_window || theApp.m_cfg_data.m_show_task_bar_wnd)
		//{
		//获取CPU使用率
		theApp.m_cpu_usage = m_cpu_usage.GetCPUUsage();

		//获取内存利用率
		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof(statex);
		GlobalMemoryStatusEx(&statex);
		theApp.m_memory_usage = statex.dwMemoryLoad;
		theApp.m_used_memory = static_cast<int>((statex.ullTotalPhys - statex.ullAvailPhys) / 1024);
		theApp.m_total_memory  = static_cast<int>(statex.ullTotalPhys / 1024);

		ShowInfo();		//刷新窗口信息
	
		//更新鼠标提示
        if (theApp.m_main_wnd_data.show_tool_tip)
        {
            CString tip_info;
            tip_info = GetMouseTipsInfo();
            m_tool_tips.UpdateTipText(tip_info, this);
        }
		//更新任务栏窗口鼠标提示
		if (IsTaskbarWndValid())
			m_tBarDlg->UpdateToolTips();

		//}
        m_monitor_time_cnt++;
	}

    if (nIDEvent == MAIN_TIMER)
    {
        if (m_first_start)		//这个if语句在程序启动后1秒执行
        {
            //将设置窗口置顶的处理放在这里是用于解决
            //放在初始化函数中可能会出现设置置顶无效的问题
         
			ShowWindow(SW_HIDE);//设置隐藏主窗口
            //打开任务栏窗口
            if (theApp.m_cfg_data.m_show_task_bar_wnd && m_tBarDlg == nullptr)
                OpenTaskBarWnd();

            //如果窗口的位置为(0, 0)，则在初始化时MoveWindow函数无效，此时再移动一次窗口
            if (theApp.m_cfg_data.m_position_x == 0 && theApp.m_cfg_data.m_position_y == 0)
            {
                SetWindowPos(nullptr, theApp.m_cfg_data.m_position_x, theApp.m_cfg_data.m_position_y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
            }

            m_first_start = false;
        }


        //只有主窗口和任务栏窗口至少有一个显示时才执行下面的处理
        if (theApp.m_cfg_data.m_show_task_bar_wnd)
        {
            //每隔10秒钟检测一次是否可以嵌入任务栏
            if (IsTaskbarWndValid() && m_timer_cnt % 10 == 1)
            {
                if (m_tBarDlg->GetCannotInsertToTaskBar() && m_insert_to_taskbar_cnt < MAX_INSERT_TO_TASKBAR_CNT)
                {
                    CloseTaskBarWnd();
                    OpenTaskBarWnd();
                    m_insert_to_taskbar_cnt++;
                    if (m_insert_to_taskbar_cnt == MAX_INSERT_TO_TASKBAR_CNT)
                    {
                        if (m_tBarDlg->GetCannotInsertToTaskBar() && m_cannot_intsert_to_task_bar_warning)		//确保提示信息只弹出一次
                        {
                            //写入错误日志
                            CString info;
                            info.LoadString(IDS_CONNOT_INSERT_TO_TASKBAR_ERROR_LOG);
                            info.Replace(_T("<%cnt%>"), CCommon::IntToString(m_insert_to_taskbar_cnt));
                            info.Replace(_T("<%error_code%>"), CCommon::IntToString(m_tBarDlg->GetErrorCode()));
                            CCommon::WriteLog(info, theApp.m_log_path.c_str());
                            //弹出错误信息
                            MessageBox(CCommon::LoadText(IDS_CONNOT_INSERT_TO_TASKBAR, CCommon::IntToString(m_tBarDlg->GetErrorCode())), NULL, MB_ICONWARNING);
                            m_cannot_intsert_to_task_bar_warning = false;
                        }
                    }
                }
                if (!m_tBarDlg->GetCannotInsertToTaskBar())
                {
                    m_insert_to_taskbar_cnt = 0;
                }
            }
        }

        //检查是否要弹出内存使用率超出提示
        if (theApp.m_general_data.memory_usage_tip_enable)
        {
            static int last_memory_usage;
            static int notify_time{ -theApp.m_notify_interval };		//记录上次弹出提示时的时间
            if (last_memory_usage < theApp.m_general_data.memory_tip_value && theApp.m_memory_usage >= theApp.m_general_data.memory_tip_value && (m_timer_cnt - notify_time > static_cast<unsigned int>(theApp.m_notify_interval)))
            {
                CString info;
                info.Format(CCommon::LoadText(IDS_MEMORY_UDAGE_EXCEED, _T(" %d%%!")), theApp.m_memory_usage);
                ShowNotifyTip(CCommon::LoadText(_T("TrafficMonitor "), IDS_NOTIFY), info.GetString());
                notify_time = m_timer_cnt;
            }
            last_memory_usage = theApp.m_memory_usage;
        }

        //检查是否要弹出流量使用超出提示
        if (theApp.m_general_data.traffic_tip_enable)
        {
            static __int64 last_today_traffic;
            __int64 traffic_tip_value;
            if (theApp.m_general_data.traffic_tip_unit == 0)
                traffic_tip_value = static_cast<__int64>(theApp.m_general_data.traffic_tip_value) * 1024 * 1024;
            else
                traffic_tip_value = static_cast<__int64>(theApp.m_general_data.traffic_tip_value) * 1024 * 1024 * 1024;

            __int64 today_traffic = theApp.m_today_up_traffic + theApp.m_today_down_traffic;
            if (last_today_traffic < traffic_tip_value && today_traffic >= traffic_tip_value)
            {
                CString info = CCommon::LoadText(IDS_TODAY_TRAFFIC_EXCEED, CCommon::DataSizeToString(today_traffic));
                ShowNotifyTip(CCommon::LoadText(_T("TrafficMonitor "), IDS_NOTIFY), info.GetString());
            }
            last_today_traffic = today_traffic;
        }

        theApp.m_win_version.CheckWindows10LightTheme();		//每隔1秒钟检查一下当前系统是否为白色主题

        //根据当前Win10颜色模式自动切换任务栏颜色
        bool light_mode = theApp.m_win_version.IsWindows10LightTheme();
        if (theApp.m_last_light_mode != light_mode)
        {
            theApp.m_last_light_mode = light_mode;
            bool restart_taskbar_dlg{ false };
            if (theApp.m_taskbar_data.auto_adapt_light_theme)
            {
                int style_index = theApp.m_win_version.IsWindows10LightTheme() ? theApp.m_taskbar_data.light_default_style : theApp.m_taskbar_data.dark_default_style;
                theApp.m_taskbar_default_style.ApplyDefaultStyle(style_index, theApp.m_taskbar_data);
                theApp.SaveConfig();
                restart_taskbar_dlg = true;
            }
            bool is_taskbar_transparent{ CTaskbarDefaultStyle::IsTaskbarTransparent(theApp.m_taskbar_data) };
            if (!is_taskbar_transparent)
            {
                CTaskbarDefaultStyle::SetTaskabrTransparent(false, theApp.m_taskbar_data);
                restart_taskbar_dlg = true;
            }
            if (restart_taskbar_dlg && IsTaskbarWndValid())
            {
                //m_tBarDlg->ApplyWindowTransparentColor();
                CloseTaskBarWnd();
                OpenTaskBarWnd();

                //写入调试日志
                if (theApp.m_debug_log)
                {
                    CString log_str;
                    log_str += _T("检测到 Windows10 深浅色变化。\n");
                    log_str += _T("IsWindows10LightTheme: ");
                    log_str += std::to_wstring(light_mode).c_str();
                    log_str += _T("\n");
                    log_str += _T("auto_adapt_light_theme: ");
                    log_str += std::to_wstring(theApp.m_taskbar_data.auto_adapt_light_theme).c_str();
                    log_str += _T("\n");
                    log_str += _T("is_taskbar_transparent: ");
                    log_str += std::to_wstring(is_taskbar_transparent).c_str();
                    log_str += _T("\n");
                    log_str += _T("taskbar_back_color: ");
                    log_str += std::to_wstring(theApp.m_taskbar_data.back_color).c_str();
                    log_str += _T("\n");
                    log_str += _T("taskbar_transparent_color: ");
                    log_str += std::to_wstring(theApp.m_taskbar_data.transparent_color).c_str();
                    log_str += _T("\n");
                    log_str += _T("taskbar_text_colors: ");
                    for (int i{}; i < TASKBAR_COLOR_NUM; i++)
                    {
                        log_str += std::to_wstring(theApp.m_taskbar_data.text_colors[i]).c_str();
                        log_str += _T(", ");
                    }
                    log_str += _T("\n");
                    CCommon::WriteLog(log_str, (theApp.m_config_dir + L".\\debug.log").c_str());
                }
            }

            //根据当前Win10颜色模式自动切换通知区图标
            if (theApp.m_cfg_data.m_notify_icon_auto_adapt)
            {
                int notify_icon_selected = theApp.m_cfg_data.m_notify_icon_selected;
                theApp.AutoSelectNotifyIcon();
                if (notify_icon_selected != theApp.m_cfg_data.m_notify_icon_selected)
                {
                    m_ntIcon.hIcon = theApp.m_notify_icons[theApp.m_cfg_data.m_notify_icon_selected];
                    if (theApp.m_cfg_data.m_show_notify_icon)
                    {
                        DeleteNotifyIcon();
                        AddNotifyIcon();
                    }
                }
            }

        }

        //根据任务栏颜色自动设置任务栏窗口背景色
        if (theApp.m_taskbar_data.auto_set_background_color && theApp.m_win_version.IsWindows8OrLater()
            && IsTaskbarWndValid() && theApp.m_taskbar_data.transparent_color != 0
            && !m_is_foreground_fullscreen)
        {
            CRect rect;
            ::GetWindowRect(m_tBarDlg->GetSafeHwnd(), rect);
            int pointx{ rect.left - 1 };
            if (theApp.m_taskbar_data.tbar_wnd_on_left && m_tBarDlg->IsTasksbarOnTopOrBottom())
                pointx = rect.right + 1;
            int pointy = rect.bottom;
            if (pointx < 0) pointx = 0;
            if (pointx >= m_screen_size.cx) pointx = m_screen_size.cx - 1;
            if (pointy < 0) pointy = 0;
            if (pointy >= m_screen_size.cy) pointy = m_screen_size.cy - 1;
            COLORREF color = ::GetPixel(m_desktop_dc, pointx, pointy);        //取任务栏窗口左侧1像素处的颜色作为背景色
            if (!CCommon::IsColorSimilar(color, theApp.m_taskbar_data.back_color) && (/*theApp.m_win_version.IsWindows10LightTheme() ||*/ color != 0))
            {
                bool is_taskbar_transparent{ CTaskbarDefaultStyle::IsTaskbarTransparent(theApp.m_taskbar_data) };
                theApp.m_taskbar_data.back_color = color;
                CTaskbarDefaultStyle::SetTaskabrTransparent(is_taskbar_transparent, theApp.m_taskbar_data);
                if(is_taskbar_transparent)
                    m_tBarDlg->ApplyWindowTransparentColor();
            }
        }

        //当检测到背景色和文字颜色都为黑色写入错误日志
        static bool erro_log_write{ false };
        if (theApp.m_taskbar_data.back_color == 0 && theApp.m_taskbar_data.text_colors[0] == 0)
        {
            if (!erro_log_write)
            {
                CString log_str;
                log_str.Format(_T("检查到背景色和文字颜色都为黑色。IsWindows10LightTheme: %d, 系统启动时间：%d/%.2d/%.2d %.2d:%.2d:%.2d"),
                    light_mode, m_start_time.wYear, m_start_time.wMonth, m_start_time.wDay, m_start_time.wHour, m_start_time.wMinute, m_start_time.wSecond);
                CCommon::WriteLog(log_str, theApp.m_log_path.c_str());
                erro_log_write = true;
            }
        }
        else
        {
            erro_log_write = false;
        }

        UpdateNotifyIconTip();

        m_timer_cnt++;
    }

	if (nIDEvent == DELAY_TIMER)
	{
		AutoSelect();
		KillTimer(DELAY_TIMER);
	}

	if (nIDEvent == TASKBAR_TIMER)
	{
		if (IsTaskbarWndValid())
		{
			m_tBarDlg->AdjustWindowPos();
			m_tBarDlg->Invalidate(FALSE);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CTrafficMonitorDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//设置点击鼠标右键弹出菜单
    CMenu* pContextMenu = theApp.m_main_menu.GetSubMenu(0); //获取第一个弹出菜单，所以第一个菜单必须有子菜单 
	CPoint point1;	//定义一个用于确定光标位置的位置  
	GetCursorPos(&point1);	//获取当前光标的位置，以便使得菜单可以跟随光标
	//设置默认菜单项
	switch (theApp.m_main_wnd_data.double_click_action)
	{
	case DoubleClickAction::CONNECTION_INFO:
		pContextMenu->SetDefaultItem(ID_NETWORK_INFO);
		break;
	case DoubleClickAction::OPTIONS:
		pContextMenu->SetDefaultItem(ID_OPTIONS);
		break;
	default:
		pContextMenu->SetDefaultItem(-1);
		break;
	}
	pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point1.x, point1.y, this); //在指定位置显示弹出菜单

	CDialogEx::OnRButtonUp(nFlags, point1);
}


void CTrafficMonitorDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//在未锁定窗口位置时允许通过点击窗口内部来拖动窗口
	if (!theApp.m_cfg_data.m_lock_window_pos)
		PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CTrafficMonitorDlg::OnNetworkInfo()
{
	// TODO: 在此添加命令处理程序代码
	//弹出“连接详情”对话框
	CNetworkInfoDlg aDlg(m_connections, m_pIfTable->table, m_connection_selected);
	////向CNetworkInfoDlg类传递自启动以来已发送和接收的字节数
	//aDlg.m_in_bytes = m_pIfTable->table[m_connections[m_connection_selected].index].dwInOctets - m_connections[m_connection_selected].in_bytes;
	//aDlg.m_out_bytes = m_pIfTable->table[m_connections[m_connection_selected].index].dwOutOctets - m_connections[m_connection_selected].out_bytes;
	aDlg.m_start_time = m_start_time;
	aDlg.DoModal();
	//SetAlwaysOnTop();	//由于在“连接详情”对话框内设置了取消窗口置顶，所有在对话框关闭后，重新设置窗口置顶
	if(m_tBarDlg!=nullptr)
		m_tBarDlg->m_tool_tips.SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);	//重新设置任务栏窗口的提示信息置顶
}


//此函数用于使得 OnUpdatexxxxx(CCmdUI *pCmdUI) 函数在基于对话框程序中有效
void CTrafficMonitorDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialogEx::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	// TODO: 在此处添加消息处理程序代码
	ASSERT(pPopupMenu != NULL);
	// Check the enabled state of various menu items.

	CCmdUI state;
	state.m_pMenu = pPopupMenu;
	ASSERT(state.m_pOther == NULL);
	ASSERT(state.m_pParentMenu == NULL);

	// Determine if menu is popup in top-level menu and set m_pOther to
	// it if so (m_pParentMenu == NULL indicates that it is secondary popup).
	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
		state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
	else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
	{
		CWnd* pParent = this;
		// Child Windows dont have menus--need to go to the top!
		if (pParent != NULL &&
			(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
		{
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
				if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
				{
					// When popup is found, m_pParentMenu is containing menu.
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
		}
	}

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if (state.m_nID == 0)
			continue; // Menu separator or invalid cmd - ignore it.

		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if (state.m_nID == (UINT)-1)
		{
			// Possibly a popup menu, route to first item of that popup.
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue;       // First item of popup cant be routed to.
			}
			state.DoUpdate(this, TRUE);   // Popups are never auto disabled.
		}
		else
		{
			// Normal menu item.
			// Auto enable/disable if frame window has m_bAutoMenuEnable
			// set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
		}

		// Adjust for menu deletions and additions.
		UINT nCount = pPopupMenu->GetMenuItemCount();
		if (nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
}




void CTrafficMonitorDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	theApp.m_cannot_save_config_warning = true;
	theApp.m_cannot_save_global_config_warning = true;
	theApp.SaveConfig();	//退出前保存设置到ini文件
	theApp.SaveGlobalConfig();

	if (IsTaskbarWndValid())
		m_tBarDlg->OnCancel();

	CDialogEx::OnClose();
}


BOOL CTrafficMonitorDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	UINT uMsg = LOWORD(wParam);
	if (uMsg == ID_SELECT_ALL_CONNECTION)
	{
		theApp.m_cfg_data.m_select_all = true;
		theApp.m_cfg_data.m_auto_select = false;
		m_connection_change_flag = true;
	}
	//选择了“选择网络连接”子菜单中项目时的处理
	if (uMsg == ID_SELETE_CONNECTION)	//选择了“自动选择”菜单项
	{
		AutoSelect();
		theApp.m_cfg_data.m_auto_select = true;
		theApp.m_cfg_data.m_select_all = false;
		theApp.SaveConfig();
		m_connection_change_flag = true;
	}
	if (uMsg > ID_SELECT_ALL_CONNECTION && uMsg <= ID_SELECT_ALL_CONNECTION + m_connections.size())	//选择了一个网络连接
	{
		m_connection_selected = uMsg - ID_SELECT_ALL_CONNECTION - 1;
		theApp.m_cfg_data.m_connection_name = m_connections[m_connection_selected].description_2;
		theApp.m_cfg_data.m_auto_select = false;
		theApp.m_cfg_data.m_select_all = false;
		theApp.SaveConfig();
		m_connection_change_flag = true;
	}

	return CDialogEx::OnCommand(wParam, lParam);
}


void CTrafficMonitorDlg::OnInitMenu(CMenu* pMenu)
{
	CDialogEx::OnInitMenu(pMenu);

	// TODO: 在此处添加消息处理程序代码
	m_menu_popuped = true;

	//设置“选择连接”子菜单项中各单选项的选择状态
    CMenu* select_connection_menu = theApp.m_main_menu.GetSubMenu(0)->GetSubMenu(0);
	SetConnectionMenuState(select_connection_menu);

	if(!theApp.m_cfg_data.m_show_task_bar_wnd)	//如果没有显示任务栏窗口，则禁用“显示通知区图标”菜单项
		pMenu->EnableMenuItem(ID_SHOW_NOTIFY_ICON, MF_BYCOMMAND | MF_GRAYED);
	else
		pMenu->EnableMenuItem(ID_SHOW_NOTIFY_ICON, MF_BYCOMMAND | MF_ENABLED);

	pMenu->EnableMenuItem(ID_SELECT_ALL_CONNECTION, MF_BYCOMMAND | (theApp.m_general_data.show_all_interface? MF_GRAYED : MF_ENABLED));

	//pMenu->SetDefaultItem(ID_NETWORK_INFO);
}


BOOL CTrafficMonitorDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	//屏蔽按回车键和ESC键退出
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE;
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) return TRUE;

	if (theApp.m_main_wnd_data.show_tool_tip && m_tool_tips.GetSafeHwnd())
	{
		m_tool_tips.RelayEvent(pMsg);
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CTrafficMonitorDlg::OnLockWindowPos()
{
	// TODO: 在此添加命令处理程序代码
	theApp.m_cfg_data.m_lock_window_pos = !theApp.m_cfg_data.m_lock_window_pos;
	theApp.SaveConfig();
}


void CTrafficMonitorDlg::OnUpdateLockWindowPos(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(theApp.m_cfg_data.m_lock_window_pos);
}


void CTrafficMonitorDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);

	// TODO: 在此处添加消息处理程序代码

	if (!m_first_start)
	{
		theApp.m_cfg_data.m_position_x = x;
		theApp.m_cfg_data.m_position_y = y;
	}

}



afx_msg LRESULT CTrafficMonitorDlg::OnNotifyIcon(WPARAM wParam, LPARAM lParam)
{
	bool dialog_exist{ false };
	HWND handle{};
	if (lParam == WM_LBUTTONDOWN || lParam == WM_RBUTTONUP || lParam == WM_LBUTTONDBLCLK)
	{
		const int WIND_NUM{ 7 };
		const CString diloge_title[WIND_NUM]{
			CCommon::LoadText(IDS_TITLE_CONNECTION_DETIAL), CCommon::LoadText(IDS_TITLE_CHANGE_SKIN), CCommon::LoadText(IDS_TITLE_OPTION), CCommon::LoadText(IDS_TITLE_CHANGE_ICON) };
		//依次查找程序中的每一个对话框，如果找到一个没有关闭的对话框时，则不允许弹出右键菜单，防止用户在此时退出程序
		for (int i{}; i < WIND_NUM; i++)
		{
			handle = ::FindWindow(NULL, diloge_title[i]);
			if (handle != NULL)
			{
				HWND hParent = ::GetParent(handle);		//查找找到的窗口的父窗口的句柄
				if (hParent == m_hWnd || (m_tBarDlg!=nullptr && hParent==m_tBarDlg->m_hWnd))			//只有当找到的窗口的父窗口是程序主窗口或任务栏窗口时，才说明找到了
				{
					dialog_exist = true;
					break;
				}
			}
		}

	}

	if (lParam == WM_RBUTTONUP && !dialog_exist)
	{
		//在通知区点击右键弹出右键菜单
        if (IsTaskbarWndValid())		//如果显示了任务栏窗口，则在右击了通知区图标后将焦点设置到任务栏窗口
			m_tBarDlg->SetForegroundWindow();
		else				//否则将焦点设置到主窗口
			SetForegroundWindow();
		CPoint point1;	//定义一个用于确定光标位置的位置  
		GetCursorPos(&point1);	//获取当前光标的位置，以便使得菜单可以跟随光标
        theApp.m_main_menu.GetSubMenu(0)->SetDefaultItem(-1);		//设置没有默认菜单项
        theApp.m_main_menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point1.x, point1.y, this); //在指定位置显示弹出菜单

	}
	if (lParam == WM_LBUTTONDBLCLK)
	{
		if (dialog_exist)		//有打开的对话框时，点击通知区图标后将焦点设置到对话框
		{
			::SetForegroundWindow(handle);
		}
		else		//没有打开的对话框时，则显示主窗口
		{
			ShowWindow(SW_RESTORE);
			SetForegroundWindow();
			theApp.SaveConfig();
		}
	}
	return 0;
}


void CTrafficMonitorDlg::OnShowNotifyIcon()
{
	// TODO: 在此添加命令处理程序代码
	if (theApp.m_cfg_data.m_show_notify_icon)
	{
		DeleteNotifyIcon();
		theApp.m_cfg_data.m_show_notify_icon = false;
	}
	else
	{
		AddNotifyIcon();
		theApp.m_cfg_data.m_show_notify_icon = true;
	}
	theApp.SaveConfig();
}


void CTrafficMonitorDlg::OnUpdateShowNotifyIcon(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(theApp.m_cfg_data.m_show_notify_icon);
}


void CTrafficMonitorDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	//程序退出时删除通知栏图标
	::Shell_NotifyIcon(NIM_DELETE, &m_ntIcon);
	// TODO: 在此处添加消息处理程序代码
}


void CTrafficMonitorDlg::OnShowCpuMemory()
{
	// TODO: 在此添加命令处理程序代码
	CRect rect;
	GetWindowRect(rect);
	if (theApp.m_cfg_data.m_show_more_info)
	{
		rect.right = rect.left + m_layout_data.width_s;
		rect.bottom = rect.top + m_layout_data.height_s;
		MoveWindow(rect);
		theApp.m_cfg_data.m_show_more_info = false;
	}
	else
	{
		rect.right = rect.left + m_layout_data.width_l;
		rect.bottom = rect.top + m_layout_data.height_l;
		MoveWindow(rect);
		theApp.m_cfg_data.m_show_more_info = true;
	}
	SetItemPosition();
	ShowInfo();
	theApp.SaveConfig();
}


//任务栏窗口切换显示CPU和内存利用率时的处理
void CTrafficMonitorDlg::OnShowCpuMemory2()
{
	// TODO: 在此添加命令处理程序代码
	if (m_tBarDlg != nullptr)
	{
		bool show_cpu_memory = ((theApp.m_cfg_data.m_tbar_display_item & TDI_CPU) || (theApp.m_cfg_data.m_tbar_display_item & TDI_MEMORY));
		if (show_cpu_memory)
		{
			theApp.m_cfg_data.m_tbar_display_item &= ~TDI_CPU;
			theApp.m_cfg_data.m_tbar_display_item &= ~TDI_MEMORY;
		}
		else
		{
			theApp.m_cfg_data.m_tbar_display_item |= TDI_CPU;
			theApp.m_cfg_data.m_tbar_display_item |= TDI_MEMORY;
		}
		//theApp.m_cfg_data.m_tbar_show_cpu_memory = !theApp.m_cfg_data.m_tbar_show_cpu_memory;
		//切换显示CPU和内存利用率时，删除任务栏窗口，再重新显示
		CloseTaskBarWnd();
		OpenTaskBarWnd();
	}
}


void CTrafficMonitorDlg::OnUpdateShowCpuMemory(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(theApp.m_cfg_data.m_show_more_info);
}


void CTrafficMonitorDlg::OnMousePenetrate()
{	
	if (!theApp.m_cfg_data.m_show_notify_icon)	//如果通知图标没有显示，则将它显示出来，否则无法呼出右键菜单
	{
		//添加通知栏图标
		AddNotifyIcon();
		theApp.m_cfg_data.m_show_notify_icon = true;
	}

	//设置鼠标穿透后，弹出消息提示用户如何关闭鼠标穿透
	if (theApp.m_show_mouse_panetrate_tip)
	{
		if (MessageBox(CCommon::LoadText(IDS_MOUSE_PENETRATE_TIP_INFO), NULL, MB_ICONINFORMATION | MB_OKCANCEL) == IDCANCEL)		//点击“取消”后不再提示
		{
			theApp.m_show_mouse_panetrate_tip = false;
		}
	}

	theApp.SaveConfig();
}

void CTrafficMonitorDlg::OnShowTaskBarWnd()
{
	// TODO: 在此添加命令处理程序代码
	if (m_tBarDlg != nullptr)
	{
		CloseTaskBarWnd();
	}
	if (!theApp.m_cfg_data.m_show_task_bar_wnd)
	{
		theApp.m_cfg_data.m_show_task_bar_wnd = true;
		OpenTaskBarWnd();
	}
	else
	{
		theApp.m_cfg_data.m_show_task_bar_wnd = false;
		//关闭任务栏窗口后，如果没有显示通知区图标，且没有显示主窗口或设置了鼠标穿透，则将通知区图标显示出来
		if (!theApp.m_cfg_data.m_show_notify_icon)
		{
			AddNotifyIcon();
			theApp.m_cfg_data.m_show_notify_icon = true;
		}
	}
	theApp.SaveConfig();
}


//当资源管理器重启时会触发此消息
LRESULT CTrafficMonitorDlg::OnTaskBarCreated(WPARAM wParam, LPARAM lParam)
{
	if (m_tBarDlg != nullptr)
	{
		CloseTaskBarWnd();
		if (theApp.m_cfg_data.m_show_notify_icon)
		{
			//重新添加通知栏图标
			::Shell_NotifyIcon(NIM_ADD, &m_ntIcon);
		}
		OpenTaskBarWnd();
	}
	else
	{
		if (theApp.m_cfg_data.m_show_notify_icon)
			::Shell_NotifyIcon(NIM_ADD, &m_ntIcon);
	}
	return LRESULT();
}


void CTrafficMonitorDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnMouseMove(nFlags, point);
}


void CTrafficMonitorDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (theApp.m_main_wnd_data.double_click_action)
	{
	case DoubleClickAction::CONNECTION_INFO:
		OnNetworkInfo();			//双击后弹出“连接详情”对话框
		break;
	case DoubleClickAction::OPTIONS:
		OnOptions();				//双击后弹出“选项设置”对话框
		break;
    case DoubleClickAction::TASK_MANAGER:
        ShellExecuteW(NULL, _T("open"), (theApp.m_system_dir + L"\\Taskmgr.exe").c_str(), NULL, NULL, SW_NORMAL);		//打开任务管理器
        break;
    case DoubleClickAction::SEPCIFIC_APP:
        ShellExecuteW(NULL, _T("open"), (theApp.m_main_wnd_data.double_click_exe).c_str(), NULL, NULL, SW_NORMAL);	//打开指定程序，默认任务管理器
        break;
	default:
		break;
	}
	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void CTrafficMonitorDlg::OnOptions()
{
	// TODO: 在此添加命令处理程序代码
	_OnOptions(0);
}


//通过任务栏窗口的右键菜单打开“选项”对话框
void CTrafficMonitorDlg::OnOptions2()
{
	// TODO: 在此添加命令处理程序代码
	_OnOptions(1);
}


afx_msg LRESULT CTrafficMonitorDlg::OnExitmenuloop(WPARAM wParam, LPARAM lParam)
{
	m_menu_popuped = false;
	return 0;
}


afx_msg LRESULT CTrafficMonitorDlg::OnTaskbarMenuPopedUp(WPARAM wParam, LPARAM lParam)
{
	//设置“选择连接”子菜单项中各单选项的选择状态
	CMenu* select_connection_menu = theApp.m_taskbar_menu.GetSubMenu(0)->GetSubMenu(0);
	SetConnectionMenuState(select_connection_menu);
	return 0;
}


//任务栏窗口切换显示网速时的处理
void CTrafficMonitorDlg::OnShowNetSpeed()
{
	// TODO: 在此添加命令处理程序代码
	if (m_tBarDlg != nullptr)
	{
		bool show_net_speed = ((theApp.m_cfg_data.m_tbar_display_item & TDI_UP) || (theApp.m_cfg_data.m_tbar_display_item & TDI_DOWN));
		if (show_net_speed)
		{
			theApp.m_cfg_data.m_tbar_display_item &= ~TDI_UP;
			theApp.m_cfg_data.m_tbar_display_item &= ~TDI_DOWN;
		}
		else
		{
			theApp.m_cfg_data.m_tbar_display_item |= TDI_UP;
			theApp.m_cfg_data.m_tbar_display_item |= TDI_DOWN;
		}
		CloseTaskBarWnd();
		OpenTaskBarWnd();
	}
}


BOOL CTrafficMonitorDlg::OnQueryEndSession()
{
	if (!CDialogEx::OnQueryEndSession())
		return FALSE;

	// TODO:  在此添加专用的查询结束会话代码
	theApp.SaveConfig();
	theApp.SaveGlobalConfig();

	if (theApp.m_debug_log)
	{
		CCommon::WriteLog(_T("TrafficMonitor进程已被终止，设置已保存。"), (theApp.m_config_dir + L".\\debug.log").c_str());
	}

	return TRUE;
}
