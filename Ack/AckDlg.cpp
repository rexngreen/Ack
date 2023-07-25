
// AckDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Ack.h"
#include "AckDlg.h"
#include "DlgProxy.h"
#include "afxdialogex.h"


#include <filesystem>
#include <fstream>
#include <iostream>
#include <Lmcons.h>
#include <string>
#include <Windows.h>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif




// Define global variables
BOOL m_SDKConnected;
static AckDlg2 dlgPointViewer;
std::vector<std::string> PointNames;			// Change this as needed, depending on the amount of features/different point names you expect to have
int CurrentPointNameIndex = 0;			
std::string MeasurementModes[24];
int NumberOfMeasurementModes = 0;
std::string NominalPointGroupName;				// The name of the Nominal Points point group to be used in a locate
std::string LocatePointGroupName;				// The name of the Measured Points point group to be used in a locate
std::vector<std::string> NominalPointNames;		// The name of the Nominal Points to be used in a locate
BOOL LocateInProgress = false;					// A boolean flag to determine if the user is currently measuring points for a Locate


BOOL EnterInUse = false;	// TODO: Delete this once its no longer in use



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAckDlg dialog


IMPLEMENT_DYNAMIC(CAckDlg, CDialogEx);

CAckDlg::CAckDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ACK_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pAutoProxy = nullptr;
}

CAckDlg::~CAckDlg()
{
	// If there is an automation proxy for this dialog, set
	//  its back pointer to this dialog to null, so it knows
	//  the dialog has been deleted.
	if (m_pAutoProxy != nullptr)
		m_pAutoProxy->m_pDialog = nullptr;
}

void CAckDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO2, m_comboBox);
	DDX_Control(pDX, IDC_fileSaveName, m_fileSaveName);
	DDX_Control(pDX, IDC_COMBO1, m_filename_selector);
	DDX_Control(pDX, IDC_PointName, m_curPointName);
	DDX_Control(pDX, IDC_PointGroup, m_curPointGroup);
	DDX_Control(pDX, IDC_MeasurementMode, m_measurementProfile);
}

BEGIN_MESSAGE_MAP(CAckDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(BtnConnect, &CAckDlg::OnBnClickedBtnconnect)
	ON_BN_CLICKED(BtnDisconnect, &CAckDlg::OnBnClickedBtndisconnect)
	ON_BN_CLICKED(SaveAs, &CAckDlg::OnBnClickedSaveas)
	ON_WM_CREATE()
	ON_BN_CLICKED(BTNRunMP, &CAckDlg::OnBnClickedRunMp)
	ON_BN_CLICKED(Btn_NextMagnet, &CAckDlg::OnBnClickedNextmagnet)
	ON_EN_CHANGE(IDC_fileSaveName, &CAckDlg::OnEnChangefilesavename)
	ON_BN_CLICKED(btn_PrevMagnet, &CAckDlg::OnBnClickedPrevmagnet)
	ON_BN_CLICKED(btn_PrevPoint, &CAckDlg::OnBnClickedPrevpoint)
	ON_BN_CLICKED(btn_NextPoint, &CAckDlg::OnBnClickedNextpoint)
	ON_EN_CHANGE(IDC_PointName, &CAckDlg::OnEnChangePointname)
	ON_EN_CHANGE(IDC_PointGroup, &CAckDlg::OnEnChangePointgroup)
	ON_BN_CLICKED(btn_Measure, &CAckDlg::OnBnClickedMeasure)
	ON_BN_CLICKED(btn_JumpInstrument, &CAckDlg::OnBnClickedJumpinstrument)
	ON_BN_CLICKED(btn_DeletePoint, &CAckDlg::OnBnClickedDeletepoint)
	ON_BN_CLICKED(btn_OpenDialog, &CAckDlg::OnBnClickedOpendialog)
	ON_BN_CLICKED(btn_LocateInstrument, &CAckDlg::OnBnClickedLocateinstrument)
END_MESSAGE_MAP()


// CAckDlg message handlers

BOOL CAckDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Populate the drop-down menu (m_comboBox) with all of the measurement plans located in the local MEASUREMENT_PLANS file
	m_comboBox.Dir(DDL_DIRECTORY, _T("./MEASUREMENT_PLANS/*.mp"));

	// Load the Dialog that shows the Point Name and Point Group
	dlgPointViewer.Create(IDD_DIALOG_POINT_VIEWER, this);
	dlgPointViewer.ShowWindow(SW_SHOW);
	dlgPointViewer.SetFont();


	// Load the Point Names from PointNames.txt
	LoadPointNames();

	// Load the Nominal Point Names from Nominal_Point_Names.txt
	LoadNominalPointNames();

	// Load the Measurement Modes from MeasurementModes.txt
	LoadMeasurementModes();

	// Load a generic Point Group name to start
	SetDlgItemText(IDC_PointGroup, _T("STA_0"));
	

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAckDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAckDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAckDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// Automation servers should not exit when a user closes the UI
//  if a controller still holds on to one of its objects.  These
//  message handlers make sure that if the proxy is still in use,
//  then the UI is hidden but the dialog remains around if it
//  is dismissed.

void CAckDlg::OnClose()
{
	if (CanExit())
		CDialogEx::OnClose();
}

void CAckDlg::OnOK()
{
	if (CanExit())
		CDialogEx::OnOK();
}

void CAckDlg::OnCancel()
{
	if (CanExit())
		CDialogEx::OnCancel();
}

BOOL CAckDlg::CanExit()
{
	// If the proxy object is still around, then the automation
	//  controller is still holding on to this application.  Leave
	//  the dialog around, but hide its UI.
	if (m_pAutoProxy != nullptr)
	{
		ShowWindow(SW_HIDE);
		return FALSE;
	}

	return TRUE;
}


void CAckDlg::OnBnClickedBtnconnect()
{
	/**/
	if (!m_SDKConnected)
	{
		if (sa.Connect(_T("localhost")) == false)
		{
			MessageBox( _T("Failed to connect") );
		}
		else
		{
			MessageBox( _T("Success!") );
		}
	}
	/**/
}


void CAckDlg::OnBnClickedBtndisconnect()
{
	m_SDKConnected = FALSE;
	sa = NULL;

	// If there is an automation proxy for this dialog, set
	//  its back pointer to this dialog to NULL, so it knows
	//  the dialog has been deleted.
	if (m_pAutoProxy != NULL)
		m_pAutoProxy->m_pDialog = NULL;
}

int CAckDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!sa.CreateDispatch(_T("SpatialAnalyzerSDK.Application")))
	{
		AfxMessageBox(_T("Cannot create SA SDK Link"));
		return -1;  // fail
	}

	return 0;
}

void CAckDlg::OnBnClickedSaveas()
{
	// Check that the textbox isn't empty
	if (m_fileSaveName.LineLength(0) == 0) // if there is no text in the textbox
	{
		AfxMessageBox(_T("Please enter the desired filename: "));
		return;
	}


	// Check that a valid option has been selected from the dropdown menu
	if (m_filename_selector.GetCurSel() < 0) // if there is no option selected
	{
		AfxMessageBox(_T("Please select a file name from the dropdown box"));
		return;
	}
	

	// Get the filename from the current selection from the filename dropdown box
	CString fileName;
	m_filename_selector.GetLBText(m_filename_selector.GetCurSel(), fileName);


	// Validate the fileName, to ensure it has no illegal characters: !@#$%^&*().,<>{}\:;'"
	// We only want to allow digits (0-9), characters (A-Z, a-z), and dashes _ -
	if (fileName.SpanExcluding("`~!@#$%^&*()+=<>,.?/:;\'\"{}[]\\|") != fileName) {
		AfxMessageBox("ERROR: Illegal characters in the file name. Only use numbers, letters, and dashes.");
		return;
	}

	// Build the complete path and file name to save the SA file to

	// Get the name of the current Windows user
	char username[UNLEN + 1];
	DWORD username_len = UNLEN + 1;
	GetUserName(username, &username_len);


	CString uname = username;
	
	CString saveTo = "C:\\users\\" + uname + "\\Desktop\\" + fileName + ".xit64";	// the complete filepath to save to


	// Run the Spatial Analyzer code to save the file
	sa.SetStep(_T("Save As..."));
	sa.SetFilePathArg(_T("File Name"), _T(saveTo), FALSE);
	sa.SetBoolArg(_T("Add Serial Number?"), FALSE);
	sa.SetIntegerArg(_T("Optional Number"), 0);
	sa.ExecuteStep();


	// Notify the user that the file was saved and where it was saved to
	AfxMessageBox("The Spatial Analyzer file was saved with the name: " + fileName + ".xit64" +
		"\nThe file was saved to the location: " + saveTo, MB_ICONINFORMATION);

	// Release our buffers, for memory purposes
	fileName.ReleaseBuffer();
	uname.ReleaseBuffer();
	saveTo.ReleaseBuffer();
}


void CAckDlg::OnBnClickedRunMp()
{
	std::filesystem::path workingDirectory = std::filesystem::current_path() / "MEASUREMENT_PLANS/"; // get the current working directory

	CString mp;
	m_comboBox.GetLBText(m_comboBox.GetCurSel(), mp);

	CString fullpath = (workingDirectory.string()).c_str() + mp;

	
	/**/
	sa.SetStep(_T("Run Subroutine"));
	sa.SetFilePathArg(_T("MP Subroutine File Path"), fullpath, FALSE);
	sa.SetBoolArg(_T("Share Parent Variables?"), FALSE);
	sa.ExecuteStep();
	/**/


	/* Code below is used for deugging */
	/**
	std::ofstream myfile;
	myfile.open("C:/scream.txt");
	myfile << fullpath;
	myfile.close();
	/**/
}

/*
* This method increments the current Point Name
* 
* Inputs:
*	N/A
* 
* Results:
*	@ The current Point Name has been decremented by 1
*/
void CAckDlg::OnBnClickedPrevpoint()
{
	std::string point = GetPointName();

	// Parse the string into the point name and its number
	int index = point.find_last_not_of("0123456789");		// find the index of the last non-digit
	int length = point.size() - index - 1;
	int pointNumber = std::stoi(point.substr(index + 1, point.size() - index - 1));
	
	// Decrement the point. If the point is less than 1, set it back to 1
	pointNumber--;
	if (pointNumber < 1) { pointNumber = 1; }

	// Reconstruct the Point Name using the decremented point number
	point = point.substr(0, index + 1) + std::to_string(pointNumber);

	// Update the IDC_PointName textbox with the new value
	UpdateCurrentPointName(point.c_str());
}


/*
* This method increments the current Point Name
*
* Inputs:
*	N/A
*
* Results:
*	@ The current Point Name has been incremented by 1
*/
void CAckDlg::OnBnClickedNextpoint()
{

	std::string point = GetPointName();

	// Parse the string into the point name and its number
	int index = point.find_last_not_of("0123456789");		// find the index of the last non-digit
	int length = point.size() - index - 1;
	int pointNumber = std::stoi(point.substr(index + 1, point.size() - index - 1));

	// Reconstruct the point name using the incremented value
	pointNumber++;
	point = point.substr(0, index + 1) + std::to_string(pointNumber);

	// Update the IDC_PointName textbox with the new value
	UpdateCurrentPointName(point.c_str());
}


/**
* This method uses the value of the array PointNames[], whose value should have been loaded into the array on form load from the
*		Point_Names.txt file
*
* Inputs:
*	N/A
*
* Results:
*	@ The IDC_PointName textbox will be updated to reflect the Point Name for the new feature, which will be the previous one in the
*			order of the array
*
*/
void CAckDlg::OnBnClickedPrevmagnet()
{
	CString temp;
	std::string string1;		// This will be the value from PointNames[CurrentPointNameIndex]
	std::string string2;		// This will be the value from the textbox IDC_PointName

	int bufferLength = m_curPointName.LineLength();

	// Check that the textbox is not empty
	if (bufferLength == 0)
	{
		temp = " ";
	}
	else
	{
		string2 = GetPointName();
	}

	// Set string1 to equal the value of the array
	if (LocateInProgress)	// if there is a locate in progress, use the appropriate value from the NominalPointNames
	{
		string1 = NominalPointNames[CurrentPointNameIndex];
	}
	else					// otherwise, use the regular point names
	{
		string1 = PointNames[CurrentPointNameIndex];
	}

	// Parse the strings so they can be compared
	// I'm doing this by finding the last character in each string that's not a digit, because point names will end in a digit,
	// and then comparing the substrings that result
	int found1 = string1.find_last_not_of("0123456789");
	int found2 = string2.find_last_not_of("0123456789");
	string1 = string1.substr(0, found1 + 1);
	string2 = string2.substr(0, found2 + 1);

	// If PointNames[NumberOfPointNames] does not match the Point Name listed in the textbox, we will set the current Point Name equal to
	//		what it should be at our current index, but will not incremement
	// If the point names do match, we will increment our index in the array and the update our current Point Name
	if (string1.compare(string2) == 0) { CurrentPointNameIndex--; }
	


	if (LocateInProgress)
	{
		if (CurrentPointNameIndex < 0) { CurrentPointNameIndex = NominalPointNames.size() - 1; }
		UpdateCurrentPointName(NominalPointNames[CurrentPointNameIndex].c_str());
	}
	else
	{
		// If CurrentPointNameIndex is < 0, set it to the highest value it can be
		if (CurrentPointNameIndex < 0) { CurrentPointNameIndex = PointNames.size() - 1; }
		UpdateCurrentPointName(PointNames[CurrentPointNameIndex].c_str());
	}
	
}

/*
* This method uses the values of the array PointNames[], whose values should have been loaded into the array on form load from the 
*		Point_Names.txt file.
* 
* Inputs:
*	N/A
* 
* Results:
*	@ The IDC_PointName textbox will be updated to reflect the Point Name for the new feature, which will be the previous one in the
*		order of the array
* 
*/
void CAckDlg::OnBnClickedNextmagnet()
{
	CString temp;
	std::string string1;		// This will be the value from PointNames[CurrentPointNameIndex]
	std::string string2;		// This will be the value from the textbox IDC_PointName
	
	int bufferLength = m_curPointName.LineLength();

	// Check that the textbox is not empty
	if (bufferLength == 0)
	{
		temp = " ";
	}
	else
	{
		temp = (GetPointName()).c_str();
	}
	
	// Set string1 to equal the value of the array
	if (LocateInProgress)	// if there is a locate in progress, use the appropriate value from the NominalPointNames
	{
		string1 = NominalPointNames[CurrentPointNameIndex];
	}
	else					// otherwise, use the regular point names
	{
		string1 = PointNames[CurrentPointNameIndex];
	}

	// Parse the strings so they can be compared
	// I'm doing this by finding the last character in each string that's not a digit, because point names will end in a digit
	// and then comparing the substrings that result
	int found1 = string1.find_last_not_of("0123456789");	// Because points will end in a number, find the last character that's not a number
	string2 = temp.GetString();								// Set string2 equal to the value from the textbox. We need it in std::string to perform the operations
	int found2 = string2.find_last_not_of("0123456789");
	string1 = string1.substr(0, found1 + 1);
	string2 = string2.substr(0, found2 + 1);

	// If PointNames[NumberOfPointNames] does not match the Point Name listed in the textbox, set the current Point Name equal to what
	//		it should be at the current index, but do not increment
	// If the point names do match, increment our index in the array
	if (string1.compare(string2) == 0)	// the strings are equal
	{
		CurrentPointNameIndex++;
	}


	if (LocateInProgress)
	{
		// If CurrentPointNameIndex is => the number of points in the vector, reset it back to 0
		if (CurrentPointNameIndex >= NominalPointNames.size()) { CurrentPointNameIndex = 0; }

		// Update the point name in the textbox
		UpdateCurrentPointName(NominalPointNames[CurrentPointNameIndex].c_str());
	}
	else
	{
		// If CurrentPointNameIndex is => the number of points, reset it back to 0
		if (CurrentPointNameIndex >= PointNames.size()) { CurrentPointNameIndex = 0; }

		// Update the point name in the textbox
		UpdateCurrentPointName(PointNames[CurrentPointNameIndex].c_str());
	}
	
	
	
	/* Code below is used for deugging */
	/**
	std::ofstream myfile;
	myfile.open("C:/scream.txt");
	myfile << PointNames[CurrentPointNameIndex];
	myfile.close();
	/**/
}


void CAckDlg::LoadPointNames()
{
	// Get the location of the file which holds the point names
	std::string fileName = "Point_Names.txt";
	std::filesystem::path pointNamesFile = std::filesystem::current_path() / fileName; // get the current working directory + Point_Names.txt
			// Create an array to store the point names
	
	// Open the Point_Names.txt file, located in the current directory
	std::ifstream inFile;				// declare an input file stream
	inFile.open(pointNamesFile);		// open the file
	if (!inFile)						// check that the file was opened
	{
		std::string errorMessage = "Unable to find file: " + fileName;
		AfxMessageBox(errorMessage.c_str());
		return;
	}									// file was not opened. We have informed the user and exited the function

	// Read in the text file, set a variable to equal the amount of lines in the txt file
	std::string inputStream;
	while (std::getline(inFile, inputStream))
	{
		if (!( inputStream.empty() || (inputStream.find_first_not_of(' ') == std::string::npos) )) // Check that the string isn't null or only whitespace
		{
			PointNames.push_back(inputStream);					// Add the point name to the end of the vector
		}	// end if-statement
		
	}	// end while-loop
	
	// Update the text box which shows the current point name
	CurrentPointNameIndex = 0;
	UpdateCurrentPointName(PointNames[CurrentPointNameIndex].c_str());
	
	
	inFile.close();					// always close the file we opened
}


/*
* This function gets called on form load and loads the Nominal Point Names and the names of the point groups to be used in a
*	Locate Instrument from a text file Nominal_Point_Names.txt,
*	located in the same directory as this program, to an array, so they can be selected from the IDC_MeasurementMode dropdown menu
*
* Inputs:
*	N/A
*
* Outputs:
*	N/A
*
* Results:
*	The measurement modes from MeasurementModes.txt have been loaded and stores in the global MeasurementModes array
*/
void CAckDlg::LoadNominalPointNames()
{
	// Open the Nominal_Point_Names.txt, get the name of both point groups
	// Open the Nominal_Point_Names.txt file, which contains the names of the target point group and the target points
	std::string fileName = "Nominal_Point_Names.txt";
	std::filesystem::path targetPath = std::filesystem::current_path() / fileName; // get the current working directory + Point_Names.txt

	std::ifstream targetFile;
	targetFile.open(targetPath);
	// Iterate through the text document and create an array for the point names
	if (std::filesystem::exists(targetPath))
	{
		std::string temp;
		while (getline(targetFile, temp))
		{
			if (!(temp.empty() || (temp.find_first_not_of(' ') == std::string::npos))) // Check that the string isn't null or only whitespace
				NominalPointNames.push_back(temp);		// Add the point name to the end of the vector
		}
	}
	else	// File not found
	{
		CString fn = fileName.c_str();
		CString errorMessage = "Unable to find a file " + fn + " containing the point group names";
		AfxMessageBox(errorMessage);
		return;
	}

	targetFile.close();

	// Store the first vector value as the Reference Group name
	NominalPointGroupName = NominalPointNames.front();
	NominalPointNames.erase(NominalPointNames.begin());	// Eliminate the element that represents the reference group

	// Store the second vector element as the Measured Group name
	LocatePointGroupName = NominalPointNames.front();
	NominalPointNames.erase(NominalPointNames.begin());	// Eliminate the first vector value again, so only point names remain
	NominalPointNames.shrink_to_fit();

}

/*
* This function gets called on form load and loads the measurement plans from a textfile MeasurementModes.txt,
*	located in the same directory as this program, to an array, so they can be selected from the IDC_MeasurementMode dropdown menu
* 
* Inputs:
*	N/A
* 
* Outputs:
*	N/A
* 
* Results:
*	The measurement modes from MeasurementModes.txt have been loaded and stores in the global MeasurementModes array
*/
void CAckDlg::LoadMeasurementModes()
{
	// Get the location of the file which holds the measurement modes
	std::string fileName = "MeasurementModes.txt";
	std::filesystem::path measurementModesFile = std::filesystem::current_path() / fileName; // get the current working directory + MeasurementModes.txt

	// Open the MeasurementModes.txt file, located in the current directory
	std::ifstream inFile;				// declare an input file stream
	inFile.open(measurementModesFile);		// open the file
	if (!inFile)						// check that the file was opened
	{
		std::string errorMessage = "Unable to find file: " + fileName;
		AfxMessageBox(errorMessage.c_str());
		return;
	}									// file was not opened. We have informed the user and exited the function

	// Read in the text file, set a variable to equal the amount of lines in the txt file
	std::string inputStream;
	while (std::getline(inFile, inputStream))
	{
		if (!(inputStream.empty() || (inputStream.find_first_not_of(' ') == std::string::npos))) // Check that the string isn't null or only whitespace
			m_measurementProfile.AddString(inputStream.c_str());
	}

	// Update the dropdown menu which shows the current measurement mode
	m_measurementProfile.SetCurSel(0);


	inFile.close();					// always close the file we opened
}


/* 
* This is a simple utility function which updates the IDC_PointName textbox to show the current Point Name
* 
* Inputs:
*	@ newPointName - an LPCSTR (CString) which is the value of the new current point name
* 
* Results:
*	@ The IDC_PointName textbox is updated with the new point name
*/
void CAckDlg::UpdateCurrentPointName(LPCSTR newPointName)
{
	SetDlgItemText(IDC_PointName, newPointName);

	dlgPointViewer.SetPointName(newPointName);
}


/*
* This is a simple utility function which updates the IDC_PointName textbox to show the current Point Name
*
* Inputs:
*	@ newPointName - an LPCSTR (CString) which is the value of the new current point name
*
* Results:
*	@ The IDC_PointName textbox is updated with the new point name
*/
void CAckDlg::UpdateCurrentPointGroup(LPCSTR newPointGroup)
{
	SetDlgItemText(IDC_PointGroup, newPointGroup);

	dlgPointViewer.SetPointGroup(newPointGroup);
}



/*
* This function is called any time someone types in the textbox designated for the module name
* 
* This function will take the name typed into module name and populate the drop-down menu for the file save name
* 
* The list of file names will include the name that was typed in, and then the name that was typed appended with Iteration0 through Iteration5
*/
void CAckDlg::OnEnChangefilesavename()
{
	// clear the list box of the combo box before we add the new items to it
	m_filename_selector.Clear(); // clear the current selection
	int count = m_filename_selector.GetCount();
	for (int i = 0; i < count; i++)
	{
		m_filename_selector.DeleteString(0);
	}


	// Check if there is any text in the textbox. If there isn't, exit this method
	if (m_fileSaveName.LineLength(0) == 0) // if there are no characters in the textbox
	{
		return;
	}

	// Get the text from the textbox
	int bufferLength = m_fileSaveName.LineLength();
	CString filename;
	m_fileSaveName.GetLine(0, filename.GetBuffer(bufferLength), bufferLength); // get the first string from the first line of the text box
	filename.SetString(filename.GetBuffer(), bufferLength); // the .GetLine() function doesn't include a null-terminating character, and that was causing buffer issues, so this line ensures
												// that the variable is in proper string format, including a null-terminating character


	//  Populate the list box of the combobox with the correct names
	m_filename_selector.AddString(filename); // Add the normal filename without anything appended to it

	for (int i = 0; i < 6; i++)
	{
		// Convert i from an int to a CString
		CString iteration;
		iteration.Format(_T("%d"), i); 

		if (i != 0) { iteration = "0" + iteration; }

		// Append the iteration number to the string and add the string to the combo box
		m_filename_selector.AddString(filename + _T("_USMN_Iteration_") + iteration);
	}
}


/*
* This module handles the event of the text in the IDC_PointGroup being changed. It updates the TXT_PointGroup field in AckDlg2
*	to reflect the text change
*
* Inputs:
*	N/A
*
* Outputs:
*	N/A
*
* Results:
*	@ The text field TXT_PointGroup in AckDlg2 will be updated to reflect the new name
*/
void CAckDlg::OnEnChangePointgroup()
{
	// if LocateInProgress flag = true, set it to false, because trying to do a locate with an unexpected Point Group name may cause issues
	LocateInProgress = false;
	
	// Get the text from the textbox
	int bufferLength = m_curPointGroup.LineLength();
	CString text;

	if (bufferLength == 0) { text = " "; }
	else
	{
		m_curPointGroup.GetLine(0, text.GetBuffer(bufferLength), bufferLength);
		text.SetString(text.GetBuffer(), bufferLength);				// ensure the text is in a null-terminated string format
	}

	// Call the function to update the dialog's text field
	dlgPointViewer.SetPointGroup(text);
}


/*
* This method handles the event of the text in the IDC_PointName being changed. It updates the TXT_PointName field in AckDlg2
*	to reflect the text change
* 
* Inputs:
*	N/A
* 
* Outputs: 
*	N/A
* 
* Results:
*	@ The text field TXT_PointName in AckDlg2 will be updated to reflect the new name
*/
void CAckDlg::OnEnChangePointname()
{
	// Get the text from the textbox
	int bufferLength = m_curPointName.LineLength();
	CString text;

	if (bufferLength == 0) { text = " "; }
	else
	{
		m_curPointName.GetLine(0, text.GetBuffer(bufferLength), bufferLength);
		text.SetString(text.GetBuffer(), bufferLength);			// ensure the text is in a null-terminated string format
	}

	// Call the function to update the dialog's text field
	dlgPointViewer.SetPointName(text);
}


/*
* This function deletes the current point, as shown by the TXT_Point text field in the dlgPointViewer dialog
* 
* Inputs:
*	N/A
* 
* Outputs:
*	N/A
* 
* Results:
*	The currently listed point has been deleted from Spatial Analyzer
*/

void CAckDlg::OnBnClickedDeletepoint()
{

	// verify that DeletePointsFromGroup.mp exists
	std::string fileName = "DeletePoint.mp";
	std::filesystem::path targetPath = std::filesystem::current_path() / "Supporting_MP" / fileName;
	if (!std::filesystem::exists(targetPath))
	{
		CString fn = fileName.c_str();
		CString errorMessage = "Unable to find file: " + fn;
		AfxMessageBox(errorMessage);
		return;
	}

	// Delete any existing points from our measuredPoints point group
	
	SetStringVariable("pointGroup", GetPointGroup().c_str());
	SetStringVariable("pointName", GetPointName().c_str());
	RunSASubroutine(targetPath.string().c_str(), true);


}


/* 
* This method handles the button click event for btn_Measure. It takes a measurement with the current instrument 
*	using the measurement profile selected from the IDC_MeasurementMode dropdown menu
* 
* Inputs:
*	N/A
* 
* Outputs:
*	N/A
* 
* Results:
*	Spatial Analyzer takes a measurement using the live instrument and the selected measurement profile from the 
*		IDC_MeasurementMode dropdown menu
*/
void CAckDlg::OnBnClickedMeasure()
{
	// Get the measurement profile from the dropdown menu
	// Check that a valid option has been selected from the dropdown menu
	if (m_measurementProfile.GetCurSel() < 0) // if there is no option selected
	{
		AfxMessageBox(_T("Please select a measurement profile from the dropdown box"));
		return;
	}


	// Get the measurement profile from the current selection from the IDC_MeasurementMode dropdown box
	CString measurementMode;
	m_measurementProfile.GetLBText(m_measurementProfile.GetCurSel(), measurementMode);

	// Because the options from m_measurementProfile will be loaded from a text file, ensure that the option isn't 
	//	an empty string
	if (measurementMode.Trim().GetLength() <= 0)
	{
		AfxMessageBox(_T("Selected measurement profile is empty. Please select a valid measurement profile."));
		return;
	}


	// Get the Point Group from its textbox
	if (m_curPointGroup.LineLength() == 0)
	{
		AfxMessageBox(_T("ERROR: Invalid Point Group"));
		return;
	}
	else if (m_curPointName.LineLength() == 0)
	{
		AfxMessageBox(_T("ERROR: Invalid Point Name"));
		return;
	}
	int bufferLength = m_curPointGroup.LineLength();
	CString pointGroup;
	m_curPointGroup.GetLine(0, pointGroup.GetBuffer(bufferLength), bufferLength); // get the first string from the first line of the text box
	pointGroup.SetString(pointGroup.GetBuffer(), bufferLength);

	// Get the Point Name from its textbox
	bufferLength = m_curPointName.LineLength();
	CString pointName;
	m_curPointName.GetLine(0, pointName.GetBuffer(bufferLength), bufferLength); // get the first string from the first line of the text box
	pointName.SetString(pointName.GetBuffer(), bufferLength);


	/** Interact with Spatial Analyzer to take a measurement **/
	ConfigureAndMeasure(pointGroup, pointName, measurementMode);
	
	
	// Increment the current point
	OnBnClickedNextpoint();
}


/*
*  This function interacts with Spatial Analyzer to perform a Configure and Measure with a laser tracker
* 
* Inputs:
*	@ pointGroup		-	The Point Group to which the measured point belongs
*	@ pointName			-	The name of the point to be measured
*	@ measurementMode	-	The measurement mode to be used by the laser tracker for this measurement
* 
* Ouputs:
*	N/A
* 
* Results:
*	A configure and measure is performed with the current laser tracker
*/
void CAckDlg::ConfigureAndMeasure(CString pointGroup, CString pointName, CString measurementMode)
{
	// Get the last instrument index
	sa.SetStep("Get Last Instrument Index");
	sa.ExecuteStep();

	long value;
	sa.GetIntegerArg("Instrument ID", &value);

	BSTR s_Col = NULL;
	long instId;
	sa.GetColInstIdArg("Instrument ID", &s_Col, &instId);
	CString colName(s_Col);
	::SysFreeString(s_Col);

	// Get the active collection name
	sa.SetStep("Get Active Collection Name");
	sa.ExecuteStep();

	BSTR sValue = NULL;
	sa.GetStringArg("Currently Active Collection Name", &sValue);
	CString collectionName(sValue);
	::SysFreeString(sValue);

	// Create the point name from the collection name, the point group, and the point name
	// The point group and point name come from the text boxes. The collection name comes from the step above
	sa.SetStep("Make a Point Name from Strings");
	sa.SetStringArg("Collection", collectionName);
	sa.SetStringArg("Group", pointGroup);
	sa.SetStringArg("Target", pointName);
	sa.ExecuteStep();

	BSTR sCol = NULL;
	BSTR sGrp = NULL;
	BSTR sTarg = NULL;
	sa.GetPointNameArg("Resultant Point Name", &sCol, &sGrp, &sTarg);
	CString collection(sCol);
	CString group(sGrp);
	CString target(sTarg);
	::SysFreeString(sCol);
	::SysFreeString(sGrp);
	::SysFreeString(sTarg);

	// Measure the point
	sa.SetStep("Configure and Measure");
	sa.SetColInstIdArg("Instrument's ID", colName, instId);
	sa.SetPointNameArg("Target Name", collectionName, pointGroup, pointName);
	sa.SetStringArg("Measurement Mode", measurementMode);
	sa.SetBoolArg("Measure Immediately", TRUE);
	sa.SetBoolArg("Wait for Completion", TRUE);
	sa.SetDoubleArg("Timeout in Seconds", 0.000000);
	sa.ExecuteStep();
}


/* This function handles the btn_OpenDialog button click, and opens the Point Viewer dialog */
void CAckDlg::OnBnClickedOpendialog()
{
	if (!dlgPointViewer.IsWindowVisible())
	{
		dlgPointViewer.ShowWindow(SW_SHOW);
		dlgPointViewer.SetFont();
	}
}


/*
* This function is to be called to locate an instrument. It runs the LocateGroupToGroup.mp measurement plan in Spatial 
*	Analyzer to locate the current instrument. This function assumes that 
*
* Inputs:
*	N/A
*
* Outputs:
*	N/A
*
* Results:
*	The current active instrument has been located
*/
void CAckDlg::OnBnClickedLocateinstrument()
{
	if (!LocateInProgress)
	{

		CurrentPointNameIndex = 0;
		UpdateCurrentPointName(NominalPointNames[CurrentPointNameIndex].c_str());
		UpdateCurrentPointGroup(LocatePointGroupName.c_str());

		// verify that DeletePointsFromGroup.mp exists
		std::string fileName = "DeletePointsFromGroup.mp";
		std::filesystem::path targetPath = std::filesystem::current_path() / "Supporting_MP" / fileName;
		if (!std::filesystem::exists(targetPath))
		{
			CString fn = fileName.c_str();
			CString errorMessage = "Unable to find file: " + fn;
			AfxMessageBox(errorMessage);
			return;
		}

		// Delete any existing points from our LocatePointGroupName point group
		SetStringVariable("groupName", LocatePointGroupName.c_str());
		RunSASubroutine(targetPath.string().c_str(), true);

		// Set the measurement plan to Fast Pt. To SA
		int i = m_measurementProfile.FindString(0, "Fast Pt. To SA");
		CString measurementMode;
		m_measurementProfile.GetLBText(i, measurementMode);
		m_measurementProfile.SetCurSel(i);

		LocateInProgress = TRUE;
	}
	else		// LocateInProgress == true
	{
		// TO HAVE THE LOCATE USE THE CURRENT MEASUREMENT MODE, UNCOMMENT THIS CODE BLOCK AND COMMENT OUT
		//		THE CODE BLOCK WITH THE LABEL "Set the measurement plan to Standard Pt. To SA"
		/**
		// Get the measurement mode
		if (m_measurementProfile.GetCurSel() < 0) // if there is no option selected
		{
			AfxMessageBox(_T("Please select a measurement profile from the dropdown box"));
			return;
		}
		CString measurementMode;
		m_measurementProfile.GetLBText(m_measurementProfile.GetCurSel(), measurementMode);

		if (measurementMode.Trim().GetLength() <= 0)
		{
			AfxMessageBox(_T("Selected measurement profile is empty. Please select a valid measurement profile."));
			return;
		}
		/**/
		

		// Run the Locate
		LocateGroupToGroup(NominalPointGroupName.c_str(), LocatePointGroupName.c_str());

		// Ensure that the jump instrument step was successful before incrementing the point group STA_#
		// Can update point group by getting the last instrument index and using that value appended to "STA_" to create point group
		// If it failed, keep point group to what it was before
		CurrentPointNameIndex = 0;
		UpdateCurrentPointName(PointNames[CurrentPointNameIndex].c_str());

		// Get the last Instrument Index
		/**/
		sa.SetStep("Get Last Instrument Index");
		sa.ExecuteStep();

		long value;
		sa.GetIntegerArg("Instrument ID", &value);

		BSTR s_Col = NULL;
		long instId;
		sa.GetColInstIdArg("Instrument ID", &s_Col, &instId);
		CString colName(s_Col);
		::SysFreeString(s_Col);

		// TO HAVE THE LOCATE USE THE CURRENT MEASUREMENT MODE, COMMENT OUT THIS CODE BLOCK
		// Set the measurement plan to Standard Pt. To SA
		int i = m_measurementProfile.FindString(0, "Standard Pt. To SA");
		CString measurementMode;
		m_measurementProfile.GetLBText(i, measurementMode);
		m_measurementProfile.SetCurSel(i);

		// Set point group to STA_#, where # is the latest instrument index
		std::string group = "STA_" + std::to_string(value);
		UpdateCurrentPointGroup(group.c_str());			// calling this method also sets LocateInProgress back to false
	}


	// If the LocateInProgress flag is true, don't perform a locate, because it may cause issues with the Jump and Locate process
	//if (LocateInProgress) return;


	// Call the mp
	//LocateGroupToGroup(NominalPointGroupName.c_str(), LocatePointGroupName.c_str());


}



/*
* This function is to be called after an instrument is moved to a new station. It grabs the Point Group and Point Names
*	of the nominal points from a Location_Points.txt file, allows the user to take a measurement for each point
*	and then locates the instrument in respect to the nominal points
* 
* Inputs:
*	N/A
* 
* Outputs:
*	N/A
* 
* Results:
*	A new instrument has been added and located
*/
void CAckDlg::OnBnClickedJumpinstrument()
{
	if (!LocateInProgress)
	{
		CurrentPointNameIndex = 0;
		UpdateCurrentPointName(NominalPointNames[CurrentPointNameIndex].c_str());
		UpdateCurrentPointGroup(LocatePointGroupName.c_str());

		// verify that DeletePointsFromGroup.mp exists
		std::string fileName = "DeletePointsFromGroup.mp";
		std::filesystem::path targetPath = std::filesystem::current_path() / "Supporting_MP" / fileName;
		if (!std::filesystem::exists(targetPath))
		{
			CString fn = fileName.c_str();
			CString errorMessage = "Unable to find file: " + fn;
			AfxMessageBox(errorMessage);
			return;
		}

		// Delete any existing points from our LocatePointGroupName point group
		SetStringVariable("groupName", LocatePointGroupName.c_str());
		RunSASubroutine(targetPath.string().c_str(), true);


		// TODO: Jump the instrument
		JumpInstrument();

		// TO HAVE THE LOCATE USE THE CURRENT MEASUREMENT MODE, COMMENT OUT THIS CODE BLOCK
		// Set the measurement plan to Fast Pt. To SA
		int i = m_measurementProfile.FindString(0, "Fast Pt. To SA");
		CString measurementMode;
		m_measurementProfile.GetLBText(i, measurementMode);
		m_measurementProfile.SetCurSel(i);


		LocateInProgress = TRUE;
	}
	else		// LocateInProgress == true
	{
		// TO HAVE THE LOCATE USE THE CURRENT MEASUREMENT MODE, UNCOMMENT THIS CODE BLOCK AND COMMENT OUT
		//		THE CODE BLOCK WITH THE LABEL "Set the measurement plan to Standard Pt. To SA"
		/**
		// Get the measurement mode
		if (m_measurementProfile.GetCurSel() < 0) // if there is no option selected
		{
			AfxMessageBox(_T("Please select a measurement profile from the dropdown box"));
			return;
		}
		CString measurementMode;
		m_measurementProfile.GetLBText(m_measurementProfile.GetCurSel(), measurementMode);

		if (measurementMode.Trim().GetLength() <= 0)
		{
			AfxMessageBox(_T("Selected measurement profile is empty. Please select a valid measurement profile."));
			return;
		}
		/**/

		// Run the Locate
		LocateGroupToGroup(NominalPointGroupName.c_str(), LocatePointGroupName.c_str());

		// Ensure that the jump instrument step was successful before incrementing the point group STA_#
		// Can update point group by getting the last instrument index and using that value appended to "STA_" to create point group
		// If it failed, keep point group to what it was before
		CurrentPointNameIndex = 0;
		UpdateCurrentPointName(PointNames[CurrentPointNameIndex].c_str());

		// Get the last Instrument Index
		/**/
		sa.SetStep("Get Last Instrument Index");
		sa.ExecuteStep();

		long value;
		sa.GetIntegerArg("Instrument ID", &value);

		BSTR s_Col = NULL;
		long instId;
		sa.GetColInstIdArg("Instrument ID", &s_Col, &instId);
		CString colName(s_Col);
		::SysFreeString(s_Col);

		// Set the measurement plan to Standard Pt. To SA
		int i = m_measurementProfile.FindString(0, "Standard Pt. To SA");
		CString measurementMode;
		m_measurementProfile.GetLBText(i, measurementMode);
		m_measurementProfile.SetCurSel(i);

		// Set point group to STA_#, where # is the latest instrument index
		std::string group = "STA_" + std::to_string(value);
		UpdateCurrentPointGroup(group.c_str());			// calling this method also sets LocateInProgress back to false
	}



	
}


/*
* This method runs a measurement plan in Spatial Analyzer that runs a measurement plan that performs a
*	Locate Instrument (Best Fit - Group to Group) and then sets the transform of the latest instrument
* 
* Inputs:
*	@ referenceGroup	-	the name of the reference point group
*	@ measuredGroup		-	the name of the measured point group
* 
* Outputs:
*	N/A
* 
* Results:
*	The active instrument in Spatial Analyzer has been located according to the measured nominal points
*/
void CAckDlg::LocateGroupToGroup(CString referenceGroup, CString measuredGroup)
{
	// Build the complete user path to the mp

	// Get the location of the file which holds the point names
	std::string fileName = "LocateGroupToGroup.mp";
	std::filesystem::path workingPath = std::filesystem::current_path() / "Supporting_MP" / fileName; // get the current working directory + file name
			// Create an array to store the point names


	// Open the Point_Names.txt file, located in the current directory
	if (!std::filesystem::exists(workingPath))				// check that the file was opened
	{
		std::string errorMessage = "Unable to find file: " + fileName;
		AfxMessageBox(errorMessage.c_str());
		return;
	}									// file was not opened. We have informed the user and exited the function

	// Set the variables in Spatial Analyzer
	SetStringVariable("referenceGroup", referenceGroup);
	SetStringVariable("measuredGroup", measuredGroup);

	// run the LocateGroupToGroup measurement plan
	RunSASubroutine(workingPath.string().c_str(), true);


}

/*
* This function returns the Point Name currently listed in IDC_PointName, or NULL if the text box is empty
*/
std::string CAckDlg::GetPointName()
{
	CString temp;
	std::string point;
	int bufferLength = m_curPointName.LineLength();
	// Check that the textbox is not empty
	if (bufferLength == 0)
	{
		AfxMessageBox(_T("ERROR: The Point Name is empty."));
		return NULL;
	}

	// Get the current Point Name from the IDC_PointName textbox
	m_curPointName.GetLine(0, temp.GetBuffer(bufferLength), bufferLength);
	temp.SetString(temp.GetBuffer(), bufferLength);
	point = temp.GetString();

	return point;
}

/*
* This function returns the Point Group currently listed in IDC_PointGroup, or NULL if the text box is empty
*/
std::string CAckDlg::GetPointGroup()
{
	CString temp;
	std::string group;
	int bufferLength = m_curPointGroup.LineLength();
	// Check that the textbox is not empty
	if (bufferLength == 0)
	{
		AfxMessageBox(_T("ERROR: The Point Name is empty."));
		return NULL;
	}

	// Get the current Point Name from the IDC_PointName textbox
	m_curPointGroup.GetLine(0, temp.GetBuffer(bufferLength), bufferLength);
	temp.SetString(temp.GetBuffer(), bufferLength);
	group = temp.GetString();

	return group;
}


/*
* This function jumps the last instrument 
*/
void CAckDlg::JumpInstrument()
{
	// verify that JumpInstrument.mp exists
	std::string fileName = "JumpInstrument.mp";
	std::filesystem::path targetPath = std::filesystem::current_path() / "Supporting_MP" / fileName;
	if (!std::filesystem::exists(targetPath))
	{
		CString fn = fileName.c_str();
		CString errorMessage = "Unable to find file: " + fn;
		AfxMessageBox(errorMessage);
		return;
	}

	// Delete any existing points from our LocatePointGroupName point group
	SetStringVariable("groupName", LocatePointGroupName.c_str());
	RunSASubroutine(targetPath.string().c_str(), true);
}

// This function sets a string variable in Spatial Analyzer
bool CAckDlg::SetStringVariable(CString name, CString value)

{
	sa.SetStep("Set String Variable");
	sa.SetStringArg("Name", name);
	sa.SetStringArg("Value", value);

	return sa.ExecuteStep();
}

// This function runs a subroutine in Spatial Analyzer
bool CAckDlg::RunSASubroutine(CString pathFileName, bool shareParentVariable)
{
	sa.SetStep("Run Subroutine");
	sa.SetFilePathArg("MP Subroutine File Path", pathFileName, false);
	sa.SetBoolArg("Share Parent Variables?", shareParentVariable);
	return sa.ExecuteStep();
}




/*
* This method will handle Key press events
*
* IMPORTANT: Set CEdit control to read only or it will process input twice
* Unlike +, -, and /, the * key is also key for number 8, so it must be handled
*
*/
BOOL CAckDlg::PreTranslateMessage(MSG* pMsg)
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
	if (pMsg->message == WM_KEYDOWN)
	{

		// Decrement Point
		if (X == VK_NUMPAD8)	{ OnBnClickedPrevpoint();		return TRUE; }

		// Increment Point
		if (X == VK_NUMPAD9)	{ OnBnClickedNextpoint();		return TRUE; }

		// Previous Feature
		if (X == VK_NUMPAD5)	{ OnBnClickedPrevmagnet();		return TRUE; }

		// Next Feature
		if (X == VK_NUMPAD6)	{ OnBnClickedNextmagnet();		return TRUE; }

		// Jump instrument to new location
		if (X == VK_NUMPAD0)	{ OnBnClickedJumpinstrument();	return TRUE; }

		// Delete Point
		if (X == VK_NUMPAD1)	{ OnBnClickedDeletepoint();		return TRUE; }

		// Take measurement
		if (X == VK_SUBTRACT)		{ OnBnClickedMeasure();			return TRUE; }

		

	}



	return CDialog::PreTranslateMessage(pMsg);
}