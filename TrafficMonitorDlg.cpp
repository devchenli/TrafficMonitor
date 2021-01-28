
// TrafficMonitorDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "TrafficMonitor.h"
#include "TrafficMonitorDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CTrafficMonitorDlg �Ի���

//��̬��Ա��ʼ��
unsigned int CTrafficMonitorDlg::m_WM_TASKBARCREATED{ ::RegisterWindowMessage(_T("TaskbarCreated")) };	//ע����������������Ϣ

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
	else		//�����ϴ�������λ��
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
	//����Ҫ��ʾ����Ŀ
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
		if (!m_layout_data.show_up_l)		//�����������û����ʾ�ϴ��ٶȣ�������ʾ��Ϣ����ʾ�ϴ��ٶ�
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
		if (!m_layout_data.show_up_s)		//�����������û����ʾ�ϴ��ٶȣ�������ʾ��Ϣ����ʾ�ϴ��ٶ�
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

	::SystemParametersInfo(SPI_GETWORKAREA, 0, &m_screen_rect, 0);   // ��ù�������С
}


void CTrafficMonitorDlg::AutoSelect()
{
	unsigned int max_in_out_bytes{};
	unsigned int in_out_bytes;
	//m_connection_selected = m_connections[0].index;
	m_connection_selected = 0;
	//�Զ�ѡ������ʱ�������ѷ��ͺ��ѽ����ֽ���֮�������Ǹ����ӣ�����������Ϊ��ǰ�鿴������
	for (size_t i{}; i<m_connections.size(); i++)
	{
		if (m_pIfTable->table[m_connections[i].index].dwOperStatus == IF_OPER_STATUS_OPERATIONAL)		//ֻѡ������״̬Ϊ����������
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
	//Ϊm_pIfTable���������С���ڴ�
	free(m_pIfTable);
	m_dwSize = sizeof(MIB_IFTABLE);
	m_pIfTable = (MIB_IFTABLE *)malloc(m_dwSize);
	int rtn;
	rtn = GetIfTable(m_pIfTable, &m_dwSize, FALSE);
	if (rtn == ERROR_INSUFFICIENT_BUFFER)	//�����������ֵΪERROR_INSUFFICIENT_BUFFER��˵��m_pIfTable�Ĵ�С����
	{
		free(m_pIfTable);
		m_pIfTable = (MIB_IFTABLE *)malloc(m_dwSize);	//���µĴ�С���¿���һ���ڴ�
	}
	//��ȡ��ǰ���е����ӣ������浽m_connections������
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

	//����������ˡ���ʾ�����������ӡ�ʱ�����ˡ�ѡ��ȫ���������Ϊ���Զ�ѡ��
	if (theApp.m_general_data.show_all_interface && theApp.m_cfg_data.m_select_all)
	{
		theApp.m_cfg_data.m_select_all = false;
		theApp.m_cfg_data.m_auto_select = true;
	}

	//д�������־
	if (theApp.m_debug_log)
	{
		CString log_str;
		log_str += _T("���ڳ�ʼ����������...\n");
		log_str += _T("�����б�\n");
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
	//ѡ����������
	if (theApp.m_cfg_data.m_auto_select)	//�Զ�ѡ��
	{
		if (m_restart_cnt != -1)	//��m_restart_cnt������-1ʱ�������ǵ�һ�γ�ʼ��ʱ����Ҫ��ʱ5�������³�ʼ������
		{
			KillTimer(DELAY_TIMER);
			SetTimer(DELAY_TIMER, 5000, NULL);
		}
		else
		{
			AutoSelect();
		}
	}
	else		//����������Ϊ�ϴ�ѡ�������
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

	//�����ѻ�ȡ���������ڲ˵��������Ӧ��Ŀ
    CMenu* select_connection_menu = theApp.m_main_menu.GetSubMenu(0)->GetSubMenu(0);		//���á�ѡ���������ӡ��Ӳ˵���
	IniConnectionMenu(select_connection_menu);		//��ѡ���������Ӳ˵��������Ŀ

	IniTaskBarConnectionMenu();		//��ʼ�������������еġ�ѡ���������ӡ��Ӳ˵���

	m_restart_cnt++;	//��¼��ʼ������
	m_connection_change_flag = true;
}

void CTrafficMonitorDlg::IniConnectionMenu(CMenu * pMenu)
{
    ASSERT(pMenu != nullptr);
    if (pMenu != nullptr)
    {
        //�Ƚ�ID_SELECT_ALL_CONNECTION��������в˵���ɾ��
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
	CMenu* select_connection_menu = theApp.m_taskbar_menu.GetSubMenu(0)->GetSubMenu(0);		//���á�ѡ���������ӡ��Ӳ˵���
	IniConnectionMenu(select_connection_menu);		//��ѡ���������Ӳ˵��������Ŀ
}

void CTrafficMonitorDlg::SetConnectionMenuState(CMenu * pMenu)
{
	if (theApp.m_cfg_data.m_select_all)
		pMenu->CheckMenuRadioItem(0, m_connections.size() + 1, 1, MF_BYPOSITION | MF_CHECKED);
	else if (theApp.m_cfg_data.m_auto_select)		//m_auto_selectΪtrueʱΪ�Զ�ѡ��ѡ�в˵��ĵ�1��
		pMenu->CheckMenuRadioItem(0, m_connections.size() + 1, 0, MF_BYPOSITION | MF_CHECKED);
	else		//m_auto_selectΪfalseʱ���Զ�ѡ�񣬸���m_connection_selected��ֵѡ���Ӧ����
		pMenu->CheckMenuRadioItem(0, m_connections.size() + 1, m_connection_selected + 2, MF_BYPOSITION | MF_CHECKED);
}

void CTrafficMonitorDlg::CloseTaskBarWnd()
{
	if (m_tBarDlg != nullptr)
	{
		HWND hParent = ::GetParent(theApp.m_option_dlg);
		if (hParent == m_tBarDlg->GetSafeHwnd())			//�ر�����������ǰ�����ѡ�����ô����Ѵ��Ҹ����������������ڣ�����ر�
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
	//���֪ͨ��ͼ��
	::Shell_NotifyIcon(NIM_ADD, &m_ntIcon);
	if (theApp.m_cfg_data.m_show_task_bar_wnd)
		OpenTaskBarWnd();
}

void CTrafficMonitorDlg::DeleteNotifyIcon()
{
	if (theApp.m_cfg_data.m_show_task_bar_wnd)
		CloseTaskBarWnd();
	//ɾ��֪ͨ��ͼ��
	::Shell_NotifyIcon(NIM_DELETE, &m_ntIcon);
	if (theApp.m_cfg_data.m_show_task_bar_wnd)
		OpenTaskBarWnd();
}

void CTrafficMonitorDlg::ShowNotifyTip(const wchar_t * title, const wchar_t * message)
{
	//Ҫ��ʾ֪ͨ����ʾ�������Ƚ�֪ͨ��ͼ����ʾ����
	if (!theApp.m_cfg_data.m_show_notify_icon)
	{
		//���֪ͨ��ͼ��
		AddNotifyIcon();
		theApp.m_cfg_data.m_show_notify_icon = true;
	}
	//��ʾ֪ͨ��ʾ
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
	CString strTip;			//���ָ��ͼ��ʱ��ʾ����ʾ
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
	//��ѡ���������ݴ��ݸ�ѡ�����öԻ���
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
			//��������������������������ʾ���ı��������������ڿ���Ҫ�仯�����ǹر��ٴ�����������
			CloseTaskBarWnd();
			OpenTaskBarWnd();
		}

		if(optionsDlg.m_tab_general_dlg.IsAutoRunModified())
			theApp.SetAutoRun(theApp.m_general_data.auto_run);

		if (optionsDlg.m_tab_general_dlg.IsShowAllInterfaceModified())
			IniConnection();

        if (optionsDlg.m_tab_general_dlg.IsMonitorTimeSpanModified())      //������ʱ�����ı��ˣ������趨ʱ��
        {
            KillTimer(MONITOR_TIMER);
            SetTimer(MONITOR_TIMER, theApp.m_general_data.monitor_time_span, NULL);
        }

		//���û�ȡCPU�����ʵķ�ʽ
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
	if (m_font.m_hObject)	//���m_font�Ѿ�������һ��������Դ�������ͷ���
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
	//Ӧ��������ɫ����
	SetTextColor();
	//Ӧ����������
	SetTextFont();
}

// CTrafficMonitorDlg ��Ϣ�������

BOOL CTrafficMonitorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	SetWindowText(_T("TrafficMonitor"));
	//��������������ͼ��
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);

	theApp.GetDPI(this);
	//��ȡ��Ļ��С
	GetScreenSize();

    theApp.InitMenuResourse();

	IniConnection();	//��ʼ������

	//�������ʱû����ʾ���������ڣ�����ʾ֪ͨ��ͼ��
	if (!theApp.m_cfg_data.m_show_task_bar_wnd)
		theApp.m_cfg_data.m_show_notify_icon = true;

	//����֪ͨ��ͼ��
	theApp.m_notify_icons[0] = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_NOFITY_ICON), IMAGE_ICON, theApp.DPI(16), theApp.DPI(16), LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);
	theApp.m_notify_icons[1] = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_NOFITY_ICON2), IMAGE_ICON, theApp.DPI(16), theApp.DPI(16), LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);
	theApp.m_notify_icons[2] = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_NOFITY_ICON3), IMAGE_ICON, theApp.DPI(16), theApp.DPI(16), LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);
	theApp.m_notify_icons[3] = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, theApp.DPI(16), theApp.DPI(16), LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);
	theApp.m_notify_icons[4] = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_NOFITY_ICON4), IMAGE_ICON, theApp.DPI(16), theApp.DPI(16), LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);
	theApp.m_notify_icons[5] = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_NOTIFY_ICON5), IMAGE_ICON, theApp.DPI(16), theApp.DPI(16), LR_DEFAULTCOLOR | LR_CREATEDIBSECTION);

	//����֪ͨ����ͼ��
	m_ntIcon.cbSize = sizeof(NOTIFYICONDATA);	//�ýṹ������Ĵ�С
	if (theApp.m_cfg_data.m_notify_icon_selected < 0 || theApp.m_cfg_data.m_notify_icon_selected >= MAX_NOTIFY_ICON)
		theApp.m_cfg_data.m_notify_icon_selected = 0;
	m_ntIcon.hIcon = theApp.m_notify_icons[theApp.m_cfg_data.m_notify_icon_selected];		//����ͼ��
	m_ntIcon.hWnd = this->m_hWnd;				//��������ͼ��֪ͨ��Ϣ�Ĵ��ھ��
	CString atip;			//���ָ��ͼ��ʱ��ʾ����ʾ
#ifdef _DEBUG
	atip = CCommon::LoadText(IDS_TRAFFICMONITOR, _T(" (Debug)"));
#else
	atip = CCommon::LoadText(IDS_TRAFFICMONITOR);
#endif
	//wcscpy_s(m_ntIcon.szTip, 128, strTip);
	CCommon::WStringCopy(m_ntIcon.szTip, 128, atip.GetString());
	m_ntIcon.uCallbackMessage = MY_WM_NOTIFYICON;	//Ӧ�ó��������ϢID��
	m_ntIcon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;	//ͼ������ԣ����ó�ԱuCallbackMessage��hIcon��szTip��Ч
	if (theApp.m_cfg_data.m_show_notify_icon)
		::Shell_NotifyIcon(NIM_ADD, &m_ntIcon);	//��ϵͳ֪ͨ�����������ͼ��

	//����1000���봥���Ķ�ʱ��
	SetTimer(MAIN_TIMER, 1000, NULL);

    SetTimer(MONITOR_TIMER, theApp.m_general_data.monitor_time_span, NULL);

	//��ʼ��Ƥ��
	CCommon::GetFiles(theApp.m_skin_path.c_str(), m_skins);
	if (m_skins.empty())
		m_skins.push_back(L"");
	m_skin_selected = 0;
	for (unsigned int i{}; i<m_skins.size(); i++)
	{
		if (m_skins[i] == theApp.m_cfg_data.m_skin_name)
			m_skin_selected = i;
	}


	//��ʼ������λ��
	SetItemPosition();
	if (theApp.m_cfg_data.m_position_x != -1 && theApp.m_cfg_data.m_position_y != -1)
		SetWindowPos(nullptr, theApp.m_cfg_data.m_position_x, theApp.m_cfg_data.m_position_y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	//��������
	SetTextFont();

	//����������ɫ
	SetTextColor();

	//��ȡ����ʱ��ʱ��
	GetLocalTime(&m_start_time);

	//��ʼ�������ʾ
	m_tool_tips.Create(this, TTS_ALWAYSTIP);
	m_tool_tips.SetMaxTipWidth(600);
	m_tool_tips.AddTool(this, _T(""));

	//���û�ȡCPU�����ʵķ�ʽ
	m_cpu_usage.SetUseCPUTimes(theApp.m_general_data.m_get_cpu_usage_by_cpu_times);

	SetTimer(TASKBAR_TIMER, 100, NULL);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}


//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CTrafficMonitorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//����ָ��������ʱ����Monitor��ʱ���ᴥ���Ĵ���
static int GetMonitorTimerCount(int second)
{
    return second * 1000 / theApp.m_general_data.monitor_time_span;
}

void CTrafficMonitorDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (nIDEvent == MONITOR_TIMER)
	{
		//��ȡ���������ٶ�
		int rtn = GetIfTable(m_pIfTable, &m_dwSize, FALSE);
		if (!theApp.m_cfg_data.m_select_all)		//��ȡ��ǰѡ�����ӵ�����
		{
			m_in_bytes = m_pIfTable->table[m_connections[m_connection_selected].index].dwInOctets;
			m_out_bytes = m_pIfTable->table[m_connections[m_connection_selected].index].dwOutOctets;
		}
		else		//��ȡȫ�����ӵ�����
		{
			m_in_bytes = 0;
			m_out_bytes = 0;
			for (size_t i{}; i<m_connections.size(); i++)
			{
				//if (i > 0 && m_pIfTable->table[m_connections[i].index].dwInOctets == m_pIfTable->table[m_connections[i - 1].index].dwInOctets
				//	&& m_pIfTable->table[m_connections[i].index].dwOutOctets == m_pIfTable->table[m_connections[i - 1].index].dwOutOctets)
				//	continue;		//�����б��п��ܻ�����ͬ�����ӣ�ͳ���������ӵ�����ʱ�����Ե��ѷ��ͺ��ѽ����ֽ�����ȫ��ͬ������
				m_in_bytes += m_pIfTable->table[m_connections[i].index].dwInOctets;
				m_out_bytes += m_pIfTable->table[m_connections[i].index].dwOutOctets;
			}
		}

        unsigned __int64 cur_in_speed{}, cur_out_speed{};       //���μ��ʱ�����ڵ��ϴ��������ٶ�

		//������ͺͽ��յ��ֽ���Ϊ0���ϴη��ͺͽ��յ��ֽ���Ϊ0��ǰ�����Ѹı�ʱ��������Ч
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
		////�������1GB/s��˵�����ܲ������쳣��������Ч
		//if (cur_in_speed > 1073741824)
		//	cur_in_speed = 0;
		//if (cur_out_speed > 1073741824)
		//	cur_out_speed = 0;

        //����ǰ���ʱ����������ת����ÿ��ʱ�����ڵ�����
        theApp.m_in_speed = static_cast<unsigned int>(cur_in_speed * 1000 / theApp.m_general_data.monitor_time_span);
        theApp.m_out_speed = static_cast<unsigned int>(cur_out_speed * 1000 / theApp.m_general_data.monitor_time_span);

		m_connection_change_flag = false;	//������ӷ����仯�ı�־

		m_last_in_bytes = m_in_bytes;
		m_last_out_bytes = m_out_bytes;

		//�����Զ�ѡ��״̬ʱ���������30��û�����٣�������Զ�ѡ������粻�ԣ���ʱִ��һ���Զ�ѡ��
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
			//���»�ȡ��ǰ��������
			static DWORD last_interface_num = -1;
			DWORD interface_num;
			GetNumberOfInterfaces(&interface_num);
			if (last_interface_num != -1 && interface_num != last_interface_num)	//��������������仯�������³�ʼ������
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
				//д�����ĵ�����Ϣ
				if (theApp.m_debug_log)
				{
					CString log_str;
					log_str = _T("�������Ʋ�ƥ�䣺\r\n");
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

		////ֻ�������ں�����������������һ����ʾʱ��ִ������Ĵ���
		//if (!theApp.m_cfg_data.m_hide_main_window || theApp.m_cfg_data.m_show_task_bar_wnd)
		//{
		//��ȡCPUʹ����
		theApp.m_cpu_usage = m_cpu_usage.GetCPUUsage();

		//��ȡ�ڴ�������
		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof(statex);
		GlobalMemoryStatusEx(&statex);
		theApp.m_memory_usage = statex.dwMemoryLoad;
		theApp.m_used_memory = static_cast<int>((statex.ullTotalPhys - statex.ullAvailPhys) / 1024);
		theApp.m_total_memory  = static_cast<int>(statex.ullTotalPhys / 1024);

		ShowInfo();		//ˢ�´�����Ϣ
	
		//���������ʾ
        if (theApp.m_main_wnd_data.show_tool_tip)
        {
            CString tip_info;
            tip_info = GetMouseTipsInfo();
            m_tool_tips.UpdateTipText(tip_info, this);
        }
		//�������������������ʾ
		if (IsTaskbarWndValid())
			m_tBarDlg->UpdateToolTips();

		//}
        m_monitor_time_cnt++;
	}

    if (nIDEvent == MAIN_TIMER)
    {
        if (m_first_start)		//���if����ڳ���������1��ִ��
        {
            //�����ô����ö��Ĵ���������������ڽ��
            //���ڳ�ʼ�������п��ܻ���������ö���Ч������
         
			ShowWindow(SW_HIDE);//��������������
            //������������
            if (theApp.m_cfg_data.m_show_task_bar_wnd && m_tBarDlg == nullptr)
                OpenTaskBarWnd();

            //������ڵ�λ��Ϊ(0, 0)�����ڳ�ʼ��ʱMoveWindow������Ч����ʱ���ƶ�һ�δ���
            if (theApp.m_cfg_data.m_position_x == 0 && theApp.m_cfg_data.m_position_y == 0)
            {
                SetWindowPos(nullptr, theApp.m_cfg_data.m_position_x, theApp.m_cfg_data.m_position_y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
            }

            m_first_start = false;
        }


        //ֻ�������ں�����������������һ����ʾʱ��ִ������Ĵ���
        if (theApp.m_cfg_data.m_show_task_bar_wnd)
        {
            //ÿ��10���Ӽ��һ���Ƿ����Ƕ��������
            if (IsTaskbarWndValid() && m_timer_cnt % 10 == 1)
            {
                if (m_tBarDlg->GetCannotInsertToTaskBar() && m_insert_to_taskbar_cnt < MAX_INSERT_TO_TASKBAR_CNT)
                {
                    CloseTaskBarWnd();
                    OpenTaskBarWnd();
                    m_insert_to_taskbar_cnt++;
                    if (m_insert_to_taskbar_cnt == MAX_INSERT_TO_TASKBAR_CNT)
                    {
                        if (m_tBarDlg->GetCannotInsertToTaskBar() && m_cannot_intsert_to_task_bar_warning)		//ȷ����ʾ��Ϣֻ����һ��
                        {
                            //д�������־
                            CString info;
                            info.LoadString(IDS_CONNOT_INSERT_TO_TASKBAR_ERROR_LOG);
                            info.Replace(_T("<%cnt%>"), CCommon::IntToString(m_insert_to_taskbar_cnt));
                            info.Replace(_T("<%error_code%>"), CCommon::IntToString(m_tBarDlg->GetErrorCode()));
                            CCommon::WriteLog(info, theApp.m_log_path.c_str());
                            //����������Ϣ
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

        //����Ƿ�Ҫ�����ڴ�ʹ���ʳ�����ʾ
        if (theApp.m_general_data.memory_usage_tip_enable)
        {
            static int last_memory_usage;
            static int notify_time{ -theApp.m_notify_interval };		//��¼�ϴε�����ʾʱ��ʱ��
            if (last_memory_usage < theApp.m_general_data.memory_tip_value && theApp.m_memory_usage >= theApp.m_general_data.memory_tip_value && (m_timer_cnt - notify_time > static_cast<unsigned int>(theApp.m_notify_interval)))
            {
                CString info;
                info.Format(CCommon::LoadText(IDS_MEMORY_UDAGE_EXCEED, _T(" %d%%!")), theApp.m_memory_usage);
                ShowNotifyTip(CCommon::LoadText(_T("TrafficMonitor "), IDS_NOTIFY), info.GetString());
                notify_time = m_timer_cnt;
            }
            last_memory_usage = theApp.m_memory_usage;
        }

        //����Ƿ�Ҫ��������ʹ�ó�����ʾ
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

        theApp.m_win_version.CheckWindows10LightTheme();		//ÿ��1���Ӽ��һ�µ�ǰϵͳ�Ƿ�Ϊ��ɫ����

        //���ݵ�ǰWin10��ɫģʽ�Զ��л���������ɫ
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

                //д�������־
                if (theApp.m_debug_log)
                {
                    CString log_str;
                    log_str += _T("��⵽ Windows10 ��ǳɫ�仯��\n");
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

            //���ݵ�ǰWin10��ɫģʽ�Զ��л�֪ͨ��ͼ��
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

        //������������ɫ�Զ��������������ڱ���ɫ
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
            COLORREF color = ::GetPixel(m_desktop_dc, pointx, pointy);        //ȡ�������������1���ش�����ɫ��Ϊ����ɫ
            if (!CCommon::IsColorSimilar(color, theApp.m_taskbar_data.back_color) && (/*theApp.m_win_version.IsWindows10LightTheme() ||*/ color != 0))
            {
                bool is_taskbar_transparent{ CTaskbarDefaultStyle::IsTaskbarTransparent(theApp.m_taskbar_data) };
                theApp.m_taskbar_data.back_color = color;
                CTaskbarDefaultStyle::SetTaskabrTransparent(is_taskbar_transparent, theApp.m_taskbar_data);
                if(is_taskbar_transparent)
                    m_tBarDlg->ApplyWindowTransparentColor();
            }
        }

        //����⵽����ɫ��������ɫ��Ϊ��ɫд�������־
        static bool erro_log_write{ false };
        if (theApp.m_taskbar_data.back_color == 0 && theApp.m_taskbar_data.text_colors[0] == 0)
        {
            if (!erro_log_write)
            {
                CString log_str;
                log_str.Format(_T("��鵽����ɫ��������ɫ��Ϊ��ɫ��IsWindows10LightTheme: %d, ϵͳ����ʱ�䣺%d/%.2d/%.2d %.2d:%.2d:%.2d"),
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	//���õ������Ҽ������˵�
    CMenu* pContextMenu = theApp.m_main_menu.GetSubMenu(0); //��ȡ��һ�������˵������Ե�һ���˵��������Ӳ˵� 
	CPoint point1;	//����һ������ȷ�����λ�õ�λ��  
	GetCursorPos(&point1);	//��ȡ��ǰ����λ�ã��Ա�ʹ�ò˵����Ը�����
	//����Ĭ�ϲ˵���
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
	pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point1.x, point1.y, this); //��ָ��λ����ʾ�����˵�

	CDialogEx::OnRButtonUp(nFlags, point1);
}


void CTrafficMonitorDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	//��δ��������λ��ʱ����ͨ����������ڲ����϶�����
	if (!theApp.m_cfg_data.m_lock_window_pos)
		PostMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CTrafficMonitorDlg::OnNetworkInfo()
{
	// TODO: �ڴ���������������
	//�������������顱�Ի���
	CNetworkInfoDlg aDlg(m_connections, m_pIfTable->table, m_connection_selected);
	////��CNetworkInfoDlg�ഫ�������������ѷ��ͺͽ��յ��ֽ���
	//aDlg.m_in_bytes = m_pIfTable->table[m_connections[m_connection_selected].index].dwInOctets - m_connections[m_connection_selected].in_bytes;
	//aDlg.m_out_bytes = m_pIfTable->table[m_connections[m_connection_selected].index].dwOutOctets - m_connections[m_connection_selected].out_bytes;
	aDlg.m_start_time = m_start_time;
	aDlg.DoModal();
	//SetAlwaysOnTop();	//�����ڡ��������顱�Ի�����������ȡ�������ö��������ڶԻ���رպ��������ô����ö�
	if(m_tBarDlg!=nullptr)
		m_tBarDlg->m_tool_tips.SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);	//�����������������ڵ���ʾ��Ϣ�ö�
}


//�˺�������ʹ�� OnUpdatexxxxx(CCmdUI *pCmdUI) �����ڻ��ڶԻ����������Ч
void CTrafficMonitorDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialogEx::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	// TODO: �ڴ˴������Ϣ����������
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	theApp.m_cannot_save_config_warning = true;
	theApp.m_cannot_save_global_config_warning = true;
	theApp.SaveConfig();	//�˳�ǰ�������õ�ini�ļ�
	theApp.SaveGlobalConfig();

	if (IsTaskbarWndValid())
		m_tBarDlg->OnCancel();

	CDialogEx::OnClose();
}


BOOL CTrafficMonitorDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: �ڴ����ר�ô����/����û���
	UINT uMsg = LOWORD(wParam);
	if (uMsg == ID_SELECT_ALL_CONNECTION)
	{
		theApp.m_cfg_data.m_select_all = true;
		theApp.m_cfg_data.m_auto_select = false;
		m_connection_change_flag = true;
	}
	//ѡ���ˡ�ѡ���������ӡ��Ӳ˵�����Ŀʱ�Ĵ���
	if (uMsg == ID_SELETE_CONNECTION)	//ѡ���ˡ��Զ�ѡ�񡱲˵���
	{
		AutoSelect();
		theApp.m_cfg_data.m_auto_select = true;
		theApp.m_cfg_data.m_select_all = false;
		theApp.SaveConfig();
		m_connection_change_flag = true;
	}
	if (uMsg > ID_SELECT_ALL_CONNECTION && uMsg <= ID_SELECT_ALL_CONNECTION + m_connections.size())	//ѡ����һ����������
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

	// TODO: �ڴ˴������Ϣ����������
	m_menu_popuped = true;

	//���á�ѡ�����ӡ��Ӳ˵����и���ѡ���ѡ��״̬
    CMenu* select_connection_menu = theApp.m_main_menu.GetSubMenu(0)->GetSubMenu(0);
	SetConnectionMenuState(select_connection_menu);

	if(!theApp.m_cfg_data.m_show_task_bar_wnd)	//���û����ʾ���������ڣ�����á���ʾ֪ͨ��ͼ�ꡱ�˵���
		pMenu->EnableMenuItem(ID_SHOW_NOTIFY_ICON, MF_BYCOMMAND | MF_GRAYED);
	else
		pMenu->EnableMenuItem(ID_SHOW_NOTIFY_ICON, MF_BYCOMMAND | MF_ENABLED);

	pMenu->EnableMenuItem(ID_SELECT_ALL_CONNECTION, MF_BYCOMMAND | (theApp.m_general_data.show_all_interface? MF_GRAYED : MF_ENABLED));

	//pMenu->SetDefaultItem(ID_NETWORK_INFO);
}


BOOL CTrafficMonitorDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	//���ΰ��س�����ESC���˳�
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
	// TODO: �ڴ���������������
	theApp.m_cfg_data.m_lock_window_pos = !theApp.m_cfg_data.m_lock_window_pos;
	theApp.SaveConfig();
}


void CTrafficMonitorDlg::OnUpdateLockWindowPos(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->SetCheck(theApp.m_cfg_data.m_lock_window_pos);
}


void CTrafficMonitorDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);

	// TODO: �ڴ˴������Ϣ����������

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
		//���β��ҳ����е�ÿһ���Ի�������ҵ�һ��û�йرյĶԻ���ʱ�����������Ҽ��˵�����ֹ�û��ڴ�ʱ�˳�����
		for (int i{}; i < WIND_NUM; i++)
		{
			handle = ::FindWindow(NULL, diloge_title[i]);
			if (handle != NULL)
			{
				HWND hParent = ::GetParent(handle);		//�����ҵ��Ĵ��ڵĸ����ڵľ��
				if (hParent == m_hWnd || (m_tBarDlg!=nullptr && hParent==m_tBarDlg->m_hWnd))			//ֻ�е��ҵ��Ĵ��ڵĸ������ǳ��������ڻ�����������ʱ����˵���ҵ���
				{
					dialog_exist = true;
					break;
				}
			}
		}

	}

	if (lParam == WM_RBUTTONUP && !dialog_exist)
	{
		//��֪ͨ������Ҽ������Ҽ��˵�
        if (IsTaskbarWndValid())		//�����ʾ�����������ڣ������һ���֪ͨ��ͼ��󽫽������õ�����������
			m_tBarDlg->SetForegroundWindow();
		else				//���򽫽������õ�������
			SetForegroundWindow();
		CPoint point1;	//����һ������ȷ�����λ�õ�λ��  
		GetCursorPos(&point1);	//��ȡ��ǰ����λ�ã��Ա�ʹ�ò˵����Ը�����
        theApp.m_main_menu.GetSubMenu(0)->SetDefaultItem(-1);		//����û��Ĭ�ϲ˵���
        theApp.m_main_menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point1.x, point1.y, this); //��ָ��λ����ʾ�����˵�

	}
	if (lParam == WM_LBUTTONDBLCLK)
	{
		if (dialog_exist)		//�д򿪵ĶԻ���ʱ�����֪ͨ��ͼ��󽫽������õ��Ի���
		{
			::SetForegroundWindow(handle);
		}
		else		//û�д򿪵ĶԻ���ʱ������ʾ������
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
	// TODO: �ڴ���������������
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
	// TODO: �ڴ������������û����洦��������
	pCmdUI->SetCheck(theApp.m_cfg_data.m_show_notify_icon);
}


void CTrafficMonitorDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	//�����˳�ʱɾ��֪ͨ��ͼ��
	::Shell_NotifyIcon(NIM_DELETE, &m_ntIcon);
	// TODO: �ڴ˴������Ϣ����������
}


void CTrafficMonitorDlg::OnShowCpuMemory()
{
	// TODO: �ڴ���������������
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


//�����������л���ʾCPU���ڴ�������ʱ�Ĵ���
void CTrafficMonitorDlg::OnShowCpuMemory2()
{
	// TODO: �ڴ���������������
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
		//�л���ʾCPU���ڴ�������ʱ��ɾ�����������ڣ���������ʾ
		CloseTaskBarWnd();
		OpenTaskBarWnd();
	}
}


void CTrafficMonitorDlg::OnUpdateShowCpuMemory(CCmdUI *pCmdUI)
{
	// TODO: �ڴ������������û����洦��������
	pCmdUI->SetCheck(theApp.m_cfg_data.m_show_more_info);
}


void CTrafficMonitorDlg::OnMousePenetrate()
{	
	if (!theApp.m_cfg_data.m_show_notify_icon)	//���֪ͨͼ��û����ʾ��������ʾ�����������޷������Ҽ��˵�
	{
		//���֪ͨ��ͼ��
		AddNotifyIcon();
		theApp.m_cfg_data.m_show_notify_icon = true;
	}

	//������괩͸�󣬵�����Ϣ��ʾ�û���ιر���괩͸
	if (theApp.m_show_mouse_panetrate_tip)
	{
		if (MessageBox(CCommon::LoadText(IDS_MOUSE_PENETRATE_TIP_INFO), NULL, MB_ICONINFORMATION | MB_OKCANCEL) == IDCANCEL)		//�����ȡ����������ʾ
		{
			theApp.m_show_mouse_panetrate_tip = false;
		}
	}

	theApp.SaveConfig();
}

void CTrafficMonitorDlg::OnShowTaskBarWnd()
{
	// TODO: �ڴ���������������
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
		//�ر����������ں����û����ʾ֪ͨ��ͼ�꣬��û����ʾ�����ڻ���������괩͸����֪ͨ��ͼ����ʾ����
		if (!theApp.m_cfg_data.m_show_notify_icon)
		{
			AddNotifyIcon();
			theApp.m_cfg_data.m_show_notify_icon = true;
		}
	}
	theApp.SaveConfig();
}


//����Դ����������ʱ�ᴥ������Ϣ
LRESULT CTrafficMonitorDlg::OnTaskBarCreated(WPARAM wParam, LPARAM lParam)
{
	if (m_tBarDlg != nullptr)
	{
		CloseTaskBarWnd();
		if (theApp.m_cfg_data.m_show_notify_icon)
		{
			//�������֪ͨ��ͼ��
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CDialogEx::OnMouseMove(nFlags, point);
}


void CTrafficMonitorDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	switch (theApp.m_main_wnd_data.double_click_action)
	{
	case DoubleClickAction::CONNECTION_INFO:
		OnNetworkInfo();			//˫���󵯳����������顱�Ի���
		break;
	case DoubleClickAction::OPTIONS:
		OnOptions();				//˫���󵯳���ѡ�����á��Ի���
		break;
    case DoubleClickAction::TASK_MANAGER:
        ShellExecuteW(NULL, _T("open"), (theApp.m_system_dir + L"\\Taskmgr.exe").c_str(), NULL, NULL, SW_NORMAL);		//�����������
        break;
    case DoubleClickAction::SEPCIFIC_APP:
        ShellExecuteW(NULL, _T("open"), (theApp.m_main_wnd_data.double_click_exe).c_str(), NULL, NULL, SW_NORMAL);	//��ָ������Ĭ�����������
        break;
	default:
		break;
	}
	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void CTrafficMonitorDlg::OnOptions()
{
	// TODO: �ڴ���������������
	_OnOptions(0);
}


//ͨ�����������ڵ��Ҽ��˵��򿪡�ѡ��Ի���
void CTrafficMonitorDlg::OnOptions2()
{
	// TODO: �ڴ���������������
	_OnOptions(1);
}


afx_msg LRESULT CTrafficMonitorDlg::OnExitmenuloop(WPARAM wParam, LPARAM lParam)
{
	m_menu_popuped = false;
	return 0;
}


afx_msg LRESULT CTrafficMonitorDlg::OnTaskbarMenuPopedUp(WPARAM wParam, LPARAM lParam)
{
	//���á�ѡ�����ӡ��Ӳ˵����и���ѡ���ѡ��״̬
	CMenu* select_connection_menu = theApp.m_taskbar_menu.GetSubMenu(0)->GetSubMenu(0);
	SetConnectionMenuState(select_connection_menu);
	return 0;
}


//�����������л���ʾ����ʱ�Ĵ���
void CTrafficMonitorDlg::OnShowNetSpeed()
{
	// TODO: �ڴ���������������
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

	// TODO:  �ڴ����ר�õĲ�ѯ�����Ự����
	theApp.SaveConfig();
	theApp.SaveGlobalConfig();

	if (theApp.m_debug_log)
	{
		CCommon::WriteLog(_T("TrafficMonitor�����ѱ���ֹ�������ѱ��档"), (theApp.m_config_dir + L".\\debug.log").c_str());
	}

	return TRUE;
}
