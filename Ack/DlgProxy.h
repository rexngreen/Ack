
// DlgProxy.h: header file
//

#pragma once

class CAckDlg;


// CAckDlgAutoProxy command target

class CAckDlgAutoProxy : public CCmdTarget
{
	DECLARE_DYNCREATE(CAckDlgAutoProxy)

	CAckDlgAutoProxy();           // protected constructor used by dynamic creation

// Attributes
public:
	CAckDlg* m_pDialog;

// Operations
public:

// Overrides
	public:
	virtual void OnFinalRelease();

// Implementation
protected:
	virtual ~CAckDlgAutoProxy();

	// Generated message map functions

	DECLARE_MESSAGE_MAP()
	DECLARE_OLECREATE(CAckDlgAutoProxy)

	// Generated OLE dispatch map functions

	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

