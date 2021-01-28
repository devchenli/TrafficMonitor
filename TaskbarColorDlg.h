#pragma once
#include "ColorStatic.h"
#include "afxwin.h"

// CTaskbarColorDlg �Ի���

class CTaskbarColorDlg : public CDialog
{
	DECLARE_DYNAMIC(CTaskbarColorDlg)

public:
	CTaskbarColorDlg(COLORREF colors[TASKBAR_COLOR_NUM], CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CTaskbarColorDlg();

	const COLORREF* GetColors() const { return m_colors; }
	
	// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TASKBAR_COLOR_DIALOG };
#endif

protected:
	COLORREF m_colors[TASKBAR_COLOR_NUM];

	//�ؼ�����
	CColorStatic m_statics[TASKBAR_COLOR_NUM];		//��ɫ�ؼ�

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	afx_msg LRESULT OnStaticClicked(WPARAM wParam, LPARAM lParam);
};
