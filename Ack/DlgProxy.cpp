
// DlgProxy.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Ack.h"
#include "DlgProxy.h"
#include "AckDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAckDlgAutoProxy

IMPLEMENT_DYNCREATE(CAckDlgAutoProxy, CCmdTarget)

CAckDlgAutoProxy::CAckDlgAutoProxy()
{
	EnableAutomation();

	// To keep the application running as long as an automation
	//	object is active, the constructor calls AfxOleLockApp.
	AfxOleLockApp();

	// Get access to the dialog through the application's
	//  main window pointer.  Set the proxy's internal pointer
	//  to point to the dialog, and set the dialog's back pointer to
	//  this proxy.
	ASSERT_VALID(AfxGetApp()->m_pMainWnd);
	if (AfxGetApp()->m_pMainWnd)
	{
		ASSERT_KINDOF(CAckDlg, AfxGetApp()->m_pMainWnd);
		if (AfxGetApp()->m_pMainWnd->IsKindOf(RUNTIME_CLASS(CAckDlg)))
		{
			m_pDialog = reinterpret_cast<CAckDlg*>(AfxGetApp()->m_pMainWnd);
			m_pDialog->m_pAutoProxy = this;
		}
	}
}

CAckDlgAutoProxy::~CAckDlgAutoProxy()
{
	// To terminate the application when all objects created with
	// 	with automation, the destructor calls AfxOleUnlockApp.
	//  Among other things, this will destroy the main dialog
	if (m_pDialog != nullptr)
		m_pDialog->m_pAutoProxy = nullptr;
	AfxOleUnlockApp();
}

void CAckDlgAutoProxy::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CAckDlgAutoProxy, CCmdTarget)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CAckDlgAutoProxy, CCmdTarget)
END_DISPATCH_MAP()

// Note: we add support for IID_IAck to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the
//  dispinterface in the .IDL file.

// {b9978040-073a-4fdd-aaed-39b897a34ba7}
static const IID IID_IAck =
{0xb9978040,0x073a,0x4fdd,{0xaa,0xed,0x39,0xb8,0x97,0xa3,0x4b,0xa7}};

BEGIN_INTERFACE_MAP(CAckDlgAutoProxy, CCmdTarget)
	INTERFACE_PART(CAckDlgAutoProxy, IID_IAck, Dispatch)
END_INTERFACE_MAP()

// The IMPLEMENT_OLECREATE2 macro is defined in pch.h of this project
// {ee6069fc-7945-4f47-afb2-d2704d63e4d3}
IMPLEMENT_OLECREATE2(CAckDlgAutoProxy, "Ack.Application", 0xee6069fc,0x7945,0x4f47,0xaf,0xb2,0xd2,0x70,0x4d,0x63,0xe4,0xd3)


// CAckDlgAutoProxy message handlers
