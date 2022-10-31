// AckDlg2.cpp : implementation file
//

#include "pch.h"
#include "Ack.h"
#include "AckDlg2.h"



// AckDlg2 dialog

IMPLEMENT_DYNAMIC(AckDlg2, CDialogEx)

AckDlg2::AckDlg2(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_POINT_VIEWER, pParent)
{

}

AckDlg2::~AckDlg2()
{
}

void AckDlg2::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);

    DDX_Control(pDX, TXT_PointGroup, m_PointGroup);
    DDX_Control(pDX, TXT_Point, m_PointName);
}


/* This method sets the TXT_Pointgroup static text field to match the input argument string */
void AckDlg2::SetPointGroup(LPCSTR pointGroup)
{
    m_PointGroup.SetWindowTextA(pointGroup);
    SetFont();
}

/* This method sets the TXT_PointName static text field to match the input argument string */
void AckDlg2::SetPointName(LPCSTR pointName)
{
    m_PointName.SetWindowTextA(pointName);
    SetFont();
}



/* 
* This method sets the font of the Point Name and Point Group static text fields
* 
* Inputs:
*   N/A
* 
* Outputs:
*   N/A
* 
* Results:
*   @ The font of TXT_PointGroup and TXT_Point have been set to font
*/
void AckDlg2::SetFont()
{
    // Create a font for this dialog
    CFont font;
    font.CreateFont(
        96,                       // nHeight
        0,                        // nWidth
        0,                        // nEscapement
        0,                        // nOrientation
        FW_NORMAL,                // nWeight
        FALSE,                    // bItalic
        FALSE,                    // bUnderline
        0,                        // cStrikeOut
        ANSI_CHARSET,             // nCharSet
        OUT_DEFAULT_PRECIS,       // nOutPrecision
        CLIP_DEFAULT_PRECIS,      // nClipPrecision
        DEFAULT_QUALITY,          // nQuality
        DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily
        _T("Arial"));             // lpszFacename

    // Set the text fields to the created font
    GetDlgItem(TXT_PointGroup)->SetFont(&font);
    GetDlgItem(TXT_Point)->SetFont(&font);



    font.DeleteObject();

}


/*
* This method will handle Key press events. It returns FALSE on any keydown event, letting it be handled by the main dialog
*
*/
BOOL AckDlg2::PreTranslateMessage(MSG* pMsg)
{
    int X = (int)pMsg->wParam;

    /* Code below is used for deugging */
    /**
    std::ofstream myfile;
    myfile.open("C:/scream.txt");
    myfile << X;
    myfile.close();
    /**/

    // If a key has been pressed, do something
    if (pMsg->message == WM_KEYDOWN) { return FALSE; }
    return CDialog::PreTranslateMessage(pMsg);
}



BEGIN_MESSAGE_MAP(AckDlg2, CDialogEx)
END_MESSAGE_MAP()



