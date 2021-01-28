
// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ


#include <afxdisp.h>        // MFC �Զ�����



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // �������Ϳؼ����� MFC ֧��

#include "resource.h"
#include <string>
using std::string;
using std::wstring;
#include <vector>
using std::vector;
#include <deque>
using std::deque;
#include<iostream>
#include<fstream>
using std::ifstream;
using std::ofstream;
#include<io.h>
#include<algorithm>
#include <cmath>
#include <afxinet.h>	//����֧��ʹ��������ص���


#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


#define MY_WM_NOTIFYICON (WM_USER+1001)

//#define CONFIG_PATH _T(".\\config.ini")
//#define CONFIG_PATHA ".\\config.ini"
//#define LOG_PATH _T(".\\error.log")
//#define HISTORY_TRAFFIC_PATH _T(".\\history_traffic.dat")
#define MAX_RESTART_CNT 10		//������³�ʼ������

#define MAIN_TIMER 1234			//����ʱ����ID
#define DELAY_TIMER 1235		//��ʱ��ʱ��ID
#define TASKBAR_TIMER 1236
#define CONNECTION_DETAIL_TIMER 1237
#define MONITOR_TIMER 1238

#define MAX_INSERT_TO_TASKBAR_CNT 5		//����Ƕ����������������

#define VERSION L"1.79.1"
#define COMPILE_DATE L"2020/08/05"

#define MAX_NOTIFY_ICON 6		//��ѡ��֪ͨ��ͼ������

//����ͼƬ���ļ���
#define BACKGROUND_IMAGE_S L"\\background.bmp"
#define BACKGROUND_IMAGE_L L"\\background_l.bmp"
#define BACKGROUND_MASK_S L"\\background_mask.bmp"
#define BACKGROUND_MASK_L L"\\background_mask_l.bmp"

#define MIN_FONT_SIZE 5
#define MAX_FONT_SIZE 72

#define SAFE_DELETE(p) do \
{\
    if(p != nullptr) \
    { \
        delete p; \
        p = nullptr; \
    } \
} while (false)

//��������������Ϊ������͸��ɫ����ɫ������Ҫ�������������ڲ�͸��ʱʹ�ã�
#define TASKBAR_TRANSPARENT_COLOR1 RGB(52, 28, 41)
#define TASKBAR_TRANSPARENT_COLOR2 RGB(38, 67, 55)

//�����ҪΪWindows XPϵͳ���룬��ȥ������һ�д����ע��
//#define COMPILE_FOR_WINXP