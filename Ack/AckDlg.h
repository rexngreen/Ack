
// AckDlg.h : header file
//

#pragma once
#include "CSpatialAnalyzerSDK.h"
#include "AckDlg2.h"


class CAckDlgAutoProxy;

// CAckDlg dialog
class CAckDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAckDlg);
	friend class CAckDlgAutoProxy;

// Construction
public:
	CAckDlg(CWnd* pParent = nullptr);	// standard constructor
	virtual ~CAckDlg();

	CSpatialAnalyzerSDK sa;
	BOOL m_SDKConnected;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ACK_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	CAckDlgAutoProxy* m_pAutoProxy;
	HICON m_hIcon;

	BOOL CanExit();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnconnect();
	afx_msg void OnBnClickedBtndisconnect();
	afx_msg void OnBnClickedSaveas();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedRunMp();
	CComboBox m_comboBox;
	CEdit m_fileSaveName;
	afx_msg void OnBnClickedNextmagnet();
	void LoadPointNames();
	void LoadNominalPointNames();
	void LoadMeasurementModes();
	void UpdateCurrentPointName(LPCSTR newPointName);
	void UpdateCurrentPointGroup(LPCSTR newPointGroup);
	void LocateGroupToGroup(CString referenceGroup, CString measuredGroup);
	std::string GetPointName();
	std::string GetPointGroup();
	void CAckDlg::JumpInstrument();
	bool SetStringVariable(CString name, CString value);
	bool RunSASubroutine(CString pathFileName, bool shareParentVariable);
	BOOL PreTranslateMessage(MSG* pMsg);
	CComboBox m_filename_selector;
	afx_msg	void OnEnChangefilesavename();
	CEdit m_curPointName;
	afx_msg void OnBnClickedPrevmagnet();
	afx_msg void OnBnClickedPrevpoint();
	afx_msg void OnBnClickedNextpoint();
	CEdit m_curPointGroup;
	afx_msg void OnEnChangePointname();
	afx_msg void OnEnChangePointgroup();
	afx_msg void OnBnClickedMeasure();
	void ConfigureAndMeasure(CString pointGroup, CString pointName, CString measurementMode);
	CComboBox m_measurementProfile;
	afx_msg void OnBnClickedJumpinstrument();
	afx_msg void OnBnClickedDeletepoint();
	afx_msg void OnBnClickedOpendialog();
	afx_msg void OnBnClickedLocateinstrument();
};
