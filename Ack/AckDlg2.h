#pragma once

#include <string>



// AckDlg2 dialog

class AckDlg2 : public CDialogEx
{
	DECLARE_DYNAMIC(AckDlg2)

public:
	AckDlg2(CWnd* pParent = nullptr);   // standard constructor
	virtual ~AckDlg2();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_POINT_VIEWER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual void SetFont();
	virtual void SetPointName(LPCSTR pointName);
	virtual void SetPointGroup(LPCSTR pointGroup);
	BOOL PreTranslateMessage(MSG* pMsg);

	
	CStatic m_PointGroup;
	CStatic m_PointName;
};
