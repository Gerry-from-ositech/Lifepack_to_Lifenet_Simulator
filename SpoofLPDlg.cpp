// SpoofLPDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpoofLP.h"
#include "SpoofLPDlg.h"
#include "IniFile.h"

#include "crcxm.h"
#include "zmodemdef.h"
#include "zmodemcore.h"

#define LOG_DEBUG 0

unsigned char mainBuf[1024];
unsigned char packetBuf[10000];
unsigned char* bufPos;
unsigned char* packetPos;
unsigned char* bufTop;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpoofLPDlg dialog

CSpoofLPDlg::CSpoofLPDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpoofLPDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpoofLPDlg)
	m_bHold = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSpoofLPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpoofLPDlg)
	DDX_Control(pDX, IDC_CHK_TIME_STAMP, m_chkTimeStamp);
	DDX_Control(pDX, IDC_CMB_ERROR, m_cmbError);
	DDX_Control(pDX, IDC_CHK_ERROR, m_chkError);
	DDX_Control(pDX, IDC_CHK_BANNER, m_chkBanner);
	DDX_Control(pDX, IDC_PROGRESS1, m_Progress1);
	DDX_Check(pDX, IDC_CHECK_HOLD, m_bHold);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSpoofLPDlg, CDialog)
	//{{AFX_MSG_MAP(CSpoofLPDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_OPEN, OnBtnOpen)
	ON_BN_CLICKED(IDC_BTN_CLOSE, OnBtnClose)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_CANCEL, OnBtnCancel)
	ON_BN_CLICKED(IDC_CHK_ERROR, OnChkError)
	ON_CBN_EDITCHANGE(IDC_CMB_ERROR, OnEditchangeCmbError)
	ON_CBN_SELCHANGE(IDC_CMB_ERROR, OnSelchangeCmbError)
	ON_BN_CLICKED(IDC_CHK_BANNER, OnChkBanner)
	ON_BN_CLICKED(IDC_CHK_TIME_STAMP, OnChkTimeStamp)
	ON_BN_CLICKED(IDC_CHECK_HOLD, OnCheckHold)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpoofLPDlg message handlers

BOOL CSpoofLPDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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

	// TODO: Add extra initialization here
	CIniFile iniFile(".\\settings.ini", 1024);
	CString csTemp;

	iniFile.GetString("settings", "COM", csTemp, "3");
	GetDlgItem(IDC_EDIT_COM)->SetWindowText(csTemp);

	iniFile.GetString("settings", "DELAY", csTemp, "4");
	GetDlgItem(IDC_EDIT_DELAY)->SetWindowText(csTemp);

	iniFile.GetString("settings", "INITDELAY", csTemp, "45");
	GetDlgItem(IDC_EDIT_INIT_DELAY)->SetWindowText(csTemp);

	iniFile.GetString("settings", "ITERATIONS", csTemp, "3");
	GetDlgItem(IDC_EDIT_ITERATIONS)->SetWindowText(csTemp);

	iniFile.GetString("settings", "OFFDELAY", csTemp, "3");
	GetDlgItem(IDC_EDIT_OFF_DELAY)->SetWindowText(csTemp);

	iniFile.GetString("settings", "BANNER", csTemp, "");
	csTemp.MakeUpper();
	if(csTemp == "YES")
	{
		m_chkBanner.SetCheck(BM_SETCHECK); // use BST_CHECKED to determine if it is checked
		banner = 1;
		m_chkTimeStamp.EnableWindow(TRUE);
	}
	else
		banner = 0;

	iniFile.GetString("settings", "TIMESTAMP", csTemp, "");
	csTemp.MakeUpper();
	if(csTemp == "YES")
	{
		m_chkTimeStamp.SetCheck(BM_SETCHECK); // use BST_CHECKED to determine if it is checked
		timestamp = 1;
	}
	else
		timestamp = 0;

	error = iniFile.GetInt("settings", "ERRORNO", 0);	// display last used error on dialog
	m_cmbError.SetCurSel(error - 1);

	iniFile.GetString("settings", "ERROR", csTemp, "");	// set checkbox if need
	csTemp.MakeUpper();
	if(csTemp == "YES")
		m_chkError.SetCheck(BM_SETCHECK);
	else
		error = 0;		// don't cause errors otherwise

	GetDlgItem(IDC_EDIT_SENT)->SetWindowText("0");
	GetDlgItem(IDC_EDIT_FAIL)->SetWindowText("0");

	port.m_bPortReady = false;	// use this to indicate port is closed

	fails = 0;
#if LOG_DEBUG == 1
    receive_ch_sniffer = true;
    send_ch_sniffer = true;
    sniffer_receive_index = 0;
    sniffer_send_index = 0;
#else
    receive_ch_sniffer = false;
    send_ch_sniffer = false;
#endif
	// Display software version
	CString csTemp2;
	csTemp.Format("V%s", GetSoftwareVersion());
	csTemp.Replace(",",".");
	//GetDlgItem(IDC_LBL_VERSION)->SetWindowText(str);
	GetWindowText(csTemp2);
	csTemp2 = csTemp2 + " " + csTemp;
	SetWindowText(csTemp2);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

CString CSpoofLPDlg::GetSoftwareVersion()
{

	// read the version info for display
	CString version("Unknown");

	char pNameBuff[MAX_PATH+1];
	DWORD nNameBuffSize = MAX_PATH;
	DWORD nSize = GetModuleFileName( NULL,(LPTSTR)pNameBuff, nNameBuffSize);
	if(nSize > 0)
	{
		// now use the location to open the resource
		DWORD zero;
		DWORD fiSize = ::GetFileVersionInfoSize( (LPTSTR)pNameBuff, &zero);
		if(fiSize > 0)
		{
			char* pInfo = new char[fiSize+1];
			BOOL bGood = ::GetFileVersionInfo( (LPTSTR)pNameBuff, 0, fiSize+1, pInfo);
			if(bGood)
			{
				void* pBuff;
				UINT pSize;

				// NOTE: 040904B0 is the value found in the resource translation
				// table for the English version of WinStop.exe.
				// If future versions are to be internationalized, this code
				// may need to be modified to determine the proper
				// language and character set values to use.
				// WARNING: this needs to be looked at (Gordon Foster)
				BOOL bVer = VerQueryValue(pInfo,
				TEXT("\\StringFileInfo\\040904B0\\ProductVersion"),&pBuff, &pSize );

				if(bVer && pSize > 0)
					version = (LPCTSTR)pBuff;
			}
			delete[] pInfo;
		}
	}

	return version;
}

void CSpoofLPDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSpoofLPDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSpoofLPDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void showErrorMessage(void)
{
	DWORD err = GetLastError();

	CString Error;
     LPTSTR s;
     FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            err,
            0,
            (LPTSTR)&s,
            0,
            NULL);

     Error = s;
     LocalFree(s);

	 MessageBox(NULL, Error, "error", MB_OK);
}


void CSpoofLPDlg::hexdump(void *ptr, int buflen, bool isSend) {
unsigned char *buf = (unsigned char*)ptr;
int i, j;
int width = 20;
char out_buff[300];
char temp[10];

    if(buflen == 0) return;

    for (i=0; i<buflen; i+=width) {
         sprintf(out_buff,"%06d: ", i);
         for (j=0; j<width; j++){
            if (i+j < buflen)  {
                  sprintf(temp,"%02x ", buf[i+j]);
                  strcat(out_buff, temp);
            } else {
                 strcat(out_buff, "   ");
            }
         }
         strcat(out_buff, " ");
         for (j=0; j<width; j++){
            if (i+j < buflen){
               sprintf(temp, "%c", isprint(buf[i+j]) ? buf[i+j] : '.');
               strcat(out_buff, temp);
            }
         }
         logFileMessage(out_buff);
    }

    if(isSend)
        sniffer_send_index = 0;
    else
        sniffer_receive_index = 0;

}


void CSpoofLPDlg::enable_send_ch_sniffer(bool state){
   send_ch_sniffer = state;
   if(state){
      sniffer_send_index = 0;
      sniffer_send_buff[0] = 0;
   }
}

void CSpoofLPDlg::enable_receive_ch_sniffer(bool state){
   receive_ch_sniffer = state;
   if(state){
      sniffer_receive_index = 0;
      sniffer_receive_buff[0] = 0;
   }
}

// capture data sent to Titan
void CSpoofLPDlg::put_ch_outbuff(unsigned char c){
    if (sniffer_send_index > 2000){
        printf("sniffer_send_buf overflow!\n");
    }else {
        sniffer_send_buff[sniffer_send_index++] = c;
    }
}



// capture data received from Titan
void CSpoofLPDlg::put_ch_inbuff(unsigned char c){
    if (sniffer_receive_index > 2000){
        printf("sniffer_receive_buff overflow!\n");
    }else {
        sniffer_receive_buff[sniffer_receive_index++] = c;
    }

}


bool debug;
bool debug2;
bool debug3;
bool debug4;
bool debug5;


void CSpoofLPDlg::OnBtnOpen()
{
	bool fin=false,quit=false,ok=true,offdelay=false;
	bool continuous;
	int tries=0;
	int iterations, delay, offDelay;
	CString csTemp, csPort;
	time_t to;
	bool justOpened;
	unsigned long length;
	DWORD dwNumberOfBytesWritten;


	// determine how many transfers to perform
	GetDlgItem(IDC_EDIT_ITERATIONS)->GetWindowText(csTemp);
	iterations = atoi(csTemp);

	// set continuous flag
	if(iterations)
		continuous = false;		// if the number is not 0, then we stop after so many sent
	else
		continuous = true;		// otherwise we will keep on going

	// setup our com port var
	GetDlgItem(IDC_EDIT_COM)->GetWindowText(csPort);
	csPort.Insert(0, "\\\\.\\COM");

	// set max buffer size for zmodem packet
	maxTx = ZMCORE_MAXTX;
	filesSent = 0;			// no file sent
	cancel = false;			// user has not hit stop button
	m_bWait = true;			// allow delays to responses from Titan

	GetDlgItem(IDC_BTN_OPEN)->EnableWindow(FALSE);	// start button
	GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_SENT)->SetWindowText("0");
	GetDlgItem(IDC_EDIT_FAIL)->SetWindowText("0");

	GetDlgItem(IDC_EDIT_COM)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_DELAY)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_INIT_DELAY)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_ITERATIONS)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_OFF_DELAY)->EnableWindow(FALSE);

	debug = false;
	debug2 = false;
	debug3 = false;
	debug4 = false;
	debug5 = false;

	while(ALLOK && !cancel)
	{
//		GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(FALSE);		// disable the Cancel button
		logFileMessage("");
		m_Progress1.SetPos(0);

		justOpened = FALSE;

		if(!port.m_bPortReady)	// if the port is closed
		{
			if(!port.OpenPort(csPort))	// try and open the port
			{
				MessageBox("Could not open COM port!", "Error", MB_OK | MB_ICONERROR);
				logFile(false, ERROR_OPEN_PORT);
				SetLastError(ERROR_OPEN_PORT);
				cleanUp();
				GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(FALSE);
				return;
			}
			port.setDTR();	// cause power to be applied to the port
			offdelay = false;
			justOpened = TRUE;

#if 0  // values are all zeros so all timeouts are disabled
if (port.GetCommunicationTimeouts()){
      sprintf(temp_buff,"ReadInterval: %d, ReadTotal: %d, ReadMult: %d, WriteTotal: %d, WriteMult: %d",
               port.m_CommTimeouts.ReadIntervalTimeout,
               port.m_CommTimeouts.ReadTotalTimeoutConstant,
               port.m_CommTimeouts.ReadTotalTimeoutMultiplier,
               port.m_CommTimeouts.WriteTotalTimeoutConstant,
               port.m_CommTimeouts.WriteTotalTimeoutMultiplier) ;
      logFileMessage(temp_buff);
}
#endif
        }

		if(banner)
		{

			GetDlgItem(IDC_LBL_STATUS)->SetWindowText("Sending Banner Data");
			port.ConfigurePort(CBR_9600, 8, FALSE, NOPARITY, ONESTOPBIT);
			length = getBannerData();
			to = time(0) + 4;
			while((time(0) < to) && !cancel)
				Pump();
			//MessageBox((char*)mainBuf);
			if(!WriteFile(port.hComm,mainBuf,length,&dwNumberOfBytesWritten,NULL))
			{
//				MessageBox("uh oh");
			}

			//FlushFileBuffers(port.hComm);
			//csTemp.Format("%u", dwNumberOfBytesWritten);
			//MessageBox(csTemp);
			to = time(0) + 2;
			while((time(0) < to) && !cancel)
				Pump();
		}

		port.ConfigurePort(CBR_38400, 8, FALSE, NOPARITY, ONESTOPBIT);	// start baud

//logFileMessage("banner sent, change baud rate from 9600 to 38400 for Zmodem session.");

		// Initial delay after power up	************************
		if(justOpened)
		{
			GetDlgItem(IDC_EDIT_INIT_DELAY)->GetWindowText(csTemp);
			delay = atoi(csTemp);
			if(delay)
			{
				to = time(0) + delay;
				while(1 && !cancel)
				{
					csTemp.Format("Performing Power on Delay...%d", to - time(0));
					GetDlgItem(IDC_LBL_STATUS)->SetWindowText(csTemp);
					if(time(0) > to)
						break;
					Pump2(1);
				}
			}
			if(cancel)
				break;
		}


		GetDlgItem(IDC_LBL_STATUS)->SetWindowText("Busy...");	// update status

		sendrz();			// send initial rz

		if(error == 1)
		{
			resetDialog();
			return;
		}
		if(error == 2)
		{
			//sendZACK();		// Titan is expect ZRQINIT
			WriteBlock("\x18g",2);
			resetDialog();
			return;
		}

		sendZRQINIT();		// and ZRQINIT
		getZMHeader();		// wait for ZRINIT
#if LOG_DEBUG == 1
sprintf(temp_buff,"After first getZMHeader() call headerType = %d, GetLastError() = %d...", headerType, GetLastError() );
logFileMessage(temp_buff);

sprintf(temp_buff,"After first call to logFileMessage()  GetLastError() = %d...", GetLastError() );
logFileMessage(temp_buff);
#endif
		if(ALLOK)
		{
//logFileMessage("Continuing...........");
			switch(headerType)
			{
				case ZCAN://canceled from other side
				{
					TRACE("set error %s\n","ZMODEM_GOTZCAN\n");
					logFile(false, ZMODEM_GOTZCAN);
					SetLastError(ZMODEM_GOTZCAN);
					quit=true;
					fin=true;
					break;
				}

				case ZRINIT:// ok, let's communicate
				{

//logFileMessage("Got expected ZRINT header from Titam Zmodem set for seq1..." );
           			TRACE("ZRINIT erhalten\n");
					//validate CRC-32-property and CANFX-property
					static int Rxflags = 0377 & headerData[3];
					bcrc32 = (Rxflags & CANFC32) != 0;
					bcanfdx = Rxflags & CANFDX;
					TRACE("bcanfdx ist %s\n",bcanfdx ? "true" : "false");
					TRACE("bcrc32 ist %s\n",bcrc32 ? "true" : "false");

					if(error == 3)
					{
						//sendZEOF();
						WriteBlock("\x18g",2);
						resetDialog();
						return;
					}

					ok = sendCommandSeq1();
#if LOG_DEBUG == 1
receive_ch_sniffer = false;
send_ch_sniffer = false;
#endif
					//if(error<14)	// error 14 means send incorrect ZFILE
					//	break;

					if(!ok)
					{
						logFile(false, ERROR_ZCOMMAND_1);
						SetLastError(ERROR_ZCOMMAND_1);
					}
					else
					{
						// start sending all files (there is only one)
						ok = sendFiles();
						if(error)
							break;

						if(!ok)
						{
							logFile(false, ERROR_ZSEND_FILES);
							SetLastError(ERROR_ZSEND_FILES);
						}
					}
					fin=true;
					break;
				}

				default:
				{
//sprintf(temp_buff,"Got unexpected header %d", headerType);
//logFileMessage(temp_buff);
					TRACE("got unexpected header\n");
					logFile(false, ERROR_ZMODEM_HEADER);
					SetLastError(ERROR_ZMODEM_HEADER);
					break;
				}
			}
		}
		else
		{
//sprintf(temp_buff,"Failed ZMODEM_INIT - GetLastError() = %d...", GetLastError() );
//logFileMessage(temp_buff);
			logFile(false, ERROR_ZMODEM_INIT);
			SetLastError(ERROR_ZMODEM_INIT);
		}

		if(cancel || error)
			break;

		// sendig is over, finish session
		if(ALLOK && !cancel)
		{
			// show that the file has been fully sent
			m_Progress1.SetPos(m_Filesize);

			ok = sendCommandSeq2();

			if(!ok)
			{
				logFile(false, ERROR_ZCOMMAND_2);
				SetLastError(ERROR_ZCOMMAND_2);
			}
			else
			{
				quit=false;
				tries=0;
				sendZFIN();

				while(ALLOK && !quit && !cancel)
				{
      				getZMHeader();
					if(ALLOK && (headerType == ZFIN))
         				quit=true;//other side has finish accepted
					else
					{
						if(tries < 100)
						{
							SetLastError(0);
							sendZFIN();
							tries++;
						}
					}
				}
			}

			if(ALLOK)
			{
				filesSent++;			// update no. of files sent
				logFile(true, 0);		// log success
				csTemp.Format("%d", filesSent);		// update textbox
				GetDlgItem(IDC_EDIT_SENT)->SetWindowText(csTemp);
				sendOO();//over and out
			}
			else
			{
				logFile(false, ERROR_ZFIN);
				SetLastError(ERROR_ZFIN);
			}

		}
		else
		{

		}

		if(cancel)
			break;

		// Delay after sending file ************************
		if(ALLOK && !cancel)
		{
			GetDlgItem(IDC_EDIT_OFF_DELAY)->GetWindowText(csTemp);
			offDelay = atoi(csTemp);
			if(offDelay)
			{
				if(m_bHold)
				{
					GetDlgItem(IDC_LBL_STATUS)->SetWindowText("Holding...");

					while(m_bHold && !cancel)
					{
						Pump();
					}
				}
				else
				{
					to = time(0) + offDelay;
					while(1 && !cancel)
					{
						csTemp.Format("Performing Power off Delay...%d", to - time(0));
						GetDlgItem(IDC_LBL_STATUS)->SetWindowText(csTemp);
						if(time(0) > to)
							break;
						Pump2(1);
					}
					offdelay = true;	// what is this again?
				}
			}
		}

		if(cancel)
			break;

		// if we're not sending continuously, see if we reached our limit
		if(!continuous)
		{
			iterations--;
			if(iterations == 0)
			{
				if(port.m_bPortReady)
				{
					port.clearDTR();	// reset device
					port.ClosePort();
				}
				break;
			}


		}



		// power off and close port
		if(ALLOK && !cancel)			// power off delay is not zero
		{
			if(port.m_bPortReady && offDelay)
			{
				port.clearDTR();
				port.ClosePort();
			}

			GetDlgItem(IDC_EDIT_DELAY)->GetWindowText(csTemp);
			delay = atoi(csTemp);

			if(delay)
			{

				to = time(0) + atoi(csTemp);
				while(1 && !cancel)
				{
					csTemp.Format("Performing Retransmit Delay...%d", to - time(0));
					GetDlgItem(IDC_LBL_STATUS)->SetWindowText(csTemp);
					if(time(0) > to)
						break;
					Pump2(1);			// allow user to hit the stop button

				}
			}
		}

		if(port.m_bPortReady && !ALLOK)
		{
			port.clearDTR();
			port.ClosePort();
			to = time(0) + 3;
			while((time(0) < to) && !cancel)
				Pump();
		}
	}


	if(offdelay == false && !error)
	{
		offDelay = atoi(csTemp);
		if(offDelay)
		{
			to = time(0) + offDelay;
			while(1 && !cancel)
			{
				csTemp.Format("Performing Power off Delay...%d", to - time(0));
				GetDlgItem(IDC_LBL_STATUS)->SetWindowText(csTemp);
				if(time(0) > to)
					break;
				Pump2(1);
			}
			offdelay = true;
		}
	}



	if(port.m_bPortReady && !error)
	{
		port.clearDTR();
		port.ClosePort();
	}
/*	if(error)
	{
		time_t timeout;
		timeout = time(0) + 5;
		while(time(0) < timeout)
			Pump();
	}*/

	PurgeComm(port.hComm, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);

	resetDialog();
}

void CSpoofLPDlg::Pump()
{
	MSG msg;
    while (::PeekMessage(&msg, NULL,   // theApp.Pump message until none
           NULL, NULL, PM_NOREMOVE) && !cancel)   // are left in the queue
    {

        AfxGetThread()->PumpMessage();


    }
	SetLastError(0);
}

void CSpoofLPDlg::Pump2(int timev)
{
	MSG msg;

	time_t to;
	to = time(0) + timev;
	while(time(0) < to && !cancel)
	{
		while (::PeekMessage(&msg, NULL,   // theApp.Pump message until none
			   NULL, NULL, PM_NOREMOVE) && !cancel)   // are left in the queue
		{

			AfxGetThread()->PumpMessage();


		}
	}
	SetLastError(0);
}

void CSpoofLPDlg::OnBtnClose()
{
	BYTE byt;
	port.ReadByte(byt);
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendrz()
//-----------------------------------------------------------------------------
{
	WriteBlock("\x0d\x0a\x72\x7a\x0d",5);
}


void WritePacket(void *buffer, DWORD num)
{
	memcpy(packetPos,(char*)buffer,1);
	packetPos++;
}

#define PACKET_CHUNK 16;

void CSpoofLPDlg::sendPacket()
{
	HANDLE HandlesToWaitFor[2];

	unsigned char* pStart = packetBuf;
	DWORD start=0;


	DWORD dwLastError;
	OVERLAPPED ow={0,0,0,0,NULL};
	ow.hEvent=CreateEvent(NULL,true,true,NULL);
	DWORD dwNumberOfBytesWritten;
	DWORD dwHandleSignaled;


	//create the abort event to end all threads
	m_hCancelEvent = CreateEvent( NULL, TRUE, FALSE, NULL );


	DWORD size = packetPos - packetBuf;
	DWORD chunk;



	HandlesToWaitFor[0] = m_hCancelEvent;
	HandlesToWaitFor[1] = ow.hEvent;

	do
	{
		if(size > 16)
			chunk = 16;
		else
			chunk = size;

		if(!WriteFile(port.hComm,pStart+start,chunk,&dwNumberOfBytesWritten,&ow))
		{
			dwLastError=GetLastError();
			if(dwLastError==ERROR_INVALID_HANDLE)
				break;
			if(dwLastError!=ERROR_IO_PENDING)
				break;

			dwHandleSignaled=WaitForMultipleObjects(2,HandlesToWaitFor,false,INFINITE);

			switch(dwHandleSignaled)
			{
				case WAIT_OBJECT_0:// CloseEvent signaled!
					break;
           		case WAIT_OBJECT_0 + 1: // Wait finished.
					break;
				default: // This case should never occur.
					break;
			}

			if(!GetOverlappedResult(port.hComm,&ow,&dwNumberOfBytesWritten,true))
			{
				dwLastError=GetLastError();
				if(dwLastError==ERROR_INVALID_HANDLE)
				{
					break;
				}
			}
		}

		start += chunk;
		size -= chunk;

		if(size <= 0)
			break;


	}while(1 && ALLOK && !cancel);

	CloseHandle(ow.hEvent);

	CloseHandle(m_hCancelEvent);
}

//-----------------------------------------------------------------------------
DWORD CSpoofLPDlg::WriteBlock(void* buf,DWORD max)
//-----------------------------------------------------------------------------
{
	if(!debug)
	{
		return WriteBuffer(buf,max);
	}
	else
	{
		WritePacket(buf,max);
		return 1;
	}
}

//-----------------------------------------------------------------------------
DWORD CSpoofLPDlg::WriteBuffer(void *buffer, DWORD num)
//-----------------------------------------------------------------------------
{

	HANDLE HandlesToWaitFor[2];
	DWORD start=0;
	DWORD dwLastError;
	OVERLAPPED ow={0,0,0,0,NULL};
	ow.hEvent=CreateEvent(NULL,true,true,NULL);
	DWORD dwNumberOfBytesWritten;
	DWORD dwHandleSignaled;

	CString csTemp;

	//ERROR_INVALID_USER_BUFFER
	//ERROR_NOT_ENOUGH_MEMORY
	//ERROR_IO_PENDING


	//create the abort event to end all threads
	m_hCancelEvent = CreateEvent( NULL, TRUE, FALSE, NULL );


	HandlesToWaitFor[0] = m_hCancelEvent;
	HandlesToWaitFor[1] = ow.hEvent;
	// Keep looping until all characters have been written.
	do
	{	// Start the overlapped I/O.

		if(!WriteFile(port.hComm,(char*)(buffer)+start,num,&dwNumberOfBytesWritten,&ow))
		{  // WriteFile failed.  Expected; lets handle it.

			dwLastError=GetLastError();
			if(dwLastError==ERROR_INVALID_HANDLE)
			{
				csTemp.Format("WriteFile returned ERROR_INVALID_HANDLE %x\n", GetLastError);
				TRACE(csTemp);
				return(0);
			}
			if(dwLastError!=ERROR_IO_PENDING)
			{
				csTemp.Format("WriteFile returned ERROR_IO_PENDING %x\n", GetLastError);
				TRACE(csTemp);
				return(0);
			}
			dwHandleSignaled=WaitForMultipleObjects(2,HandlesToWaitFor,false,INFINITE);
			switch(dwHandleSignaled)
			{
				case WAIT_OBJECT_0:// CloseEvent signaled!
					TRACE("CloseEvent signaled\n");
					return(0);
           	case WAIT_OBJECT_0 + 1: // Wait finished.
					TRACE("WriteFile finished\n");
					break;
				default: // This case should never occur.
					csTemp.Format("If this should never occur, it occured! %x\n", GetLastError());
					TRACE(csTemp);
					return(0);
			}
			if(!GetOverlappedResult(port.hComm,&ow,&dwNumberOfBytesWritten,true))
			{
				dwLastError=GetLastError();
				// Its possible for this error to occur if the
				// service provider has closed the port.
				if(dwLastError==ERROR_INVALID_HANDLE)
					csTemp.Format("GetOverlappedResult returned ERROR_INVALID_HANDLE %x\n", GetLastError());
					TRACE(csTemp);
					return(0);
					csTemp.Format("GetOverlappedResult returned %x\n", GetLastError());
					TRACE(csTemp);
				return(0);
			}
         else
		 {
			//csTemp.Format("WriteFile returned %x\n", GetLastError);
			//TRACE(csTemp);
          	//SetLastError(0);
		 }
		}
		num -= dwNumberOfBytesWritten;
		start += dwNumberOfBytesWritten;
	}
	while(num > 0 && !cancel);  // Write the whole thing!

	CloseHandle(ow.hEvent);

	CloseHandle(m_hCancelEvent);

	//csTemp.Format("WriteBuffer function returns %x\n", GetLastError());
	//TRACE(csTemp);


	return(start);
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendHexHeader()
//-----------------------------------------------------------------------------
{	//
	CRCXM crc;
	//
	crcxmInit(&crc);

	ch = ZPAD;
	sendChar();
	sendChar();

	ch = ZDLE;
	sendChar();

  	ch = frameType;
    sendChar();

   	ch = headerType;
    crcxmUpdate(&crc, ch);
	sendHexChar();

   	ch = headerData[0];
	crcxmUpdate(&crc, ch);
    sendHexChar();
   	ch = headerData[1];
    crcxmUpdate(&crc, ch);
    sendHexChar();
   	ch = headerData[2];
	crcxmUpdate(&crc, ch);
    sendHexChar();
   	ch = headerData[3];
    crcxmUpdate(&crc, ch);
    sendHexChar();

   	ch = crcxmHighbyte(&crc);
	sendHexChar();
   	ch = crcxmLowbyte(&crc);
	sendHexChar();
	ch = 0x0d;
	sendChar();
	ch = 0x0a;
	sendChar();
	ch = 0x11;
	sendChar();
}


//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendZRQINIT()
//-----------------------------------------------------------------------------
{
	frameType = ZHEX;
	headerType = ZRQINIT;
	headerData[0]=0x00;
	headerData[1]=0x00;
	headerData[2]=0x00;
	headerData[3]=0x12;	// going to send zcommand
	sendHeader();
}
//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendChar()
//-----------------------------------------------------------------------------
{	//
	unsigned char buf[1];
	//
	buf[0] = (unsigned char)ch;
	WriteBlock(buf, 1);
    if(send_ch_sniffer){
          put_ch_outbuff(ch);
    }

}

// Layer 1 ####################################################################
//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendHeader()
//-----------------------------------------------------------------------------
{
#ifdef _DEBUG
	char out[1000];
	wsprintf(out,"sending frametype: 0x%02x headertyp: 0x%02x, headerdata 0: 0x%02x 1: 0x%02x 2: 0x%02x 3: 0x%02x\n",
   			frameType,headerType,headerData[0],headerData[1],headerData[2],headerData[3]);
	TRACE(out);
#endif
	switch(frameType)
	{
		case ZHEX:
			sendHexHeader();
			break;
		case ZBIN:
			sendBinHeader();
			break;
		case ZBIN32:
			sendBin32Header();
			break;
		default:
			TRACE("wrong headertype\n");
			break;
	}
}
//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendHexChar()
//-----------------------------------------------------------------------------
{	//
	int tempCh;
	unsigned char hexdigit[]="\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x61\x62\x63\x64\x65\x66";
	//
	tempCh = ch;
	ch = hexdigit[(tempCh >> 4) & 0x0f];
	sendChar();
	if(ALLOK)
	{
   		ch = hexdigit[tempCh & 0x0f];
		sendChar();
	}
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendBinHeader()
//-----------------------------------------------------------------------------
{	//
	CRCXM crc;
   //
	crcxmInit(&crc);
	ch = ZPAD;
	sendChar();

	ch = ZDLE;
	sendChar();

	ch = frameType;
	sendChar();

	ch = headerType;
	crcxmUpdate(&crc, ch);
	sendChar();

   	ch = headerData[0];
    crcxmUpdate(&crc, ch);
	sendChar();

   	ch = headerData[1];
    crcxmUpdate(&crc, ch);
	sendChar();

   	ch = headerData[2];
    crcxmUpdate(&crc, ch);
	sendChar();

  	ch = headerData[3];
    crcxmUpdate(&crc, ch);
	sendChar();

	ch = crcxmHighbyte(&crc);
	sendChar();

   	ch = crcxmLowbyte(&crc);
	sendChar();
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendBin32Header()
//-----------------------------------------------------------------------------
{	//
	CRC32 crc;
	//
	crc32Init(&crc);
	ch = ZPAD;
	sendChar();
   	ch = ZDLE;
    sendChar();
	ch = frameType;
    sendChar();
  	ch = headerType;
    crc32Update(&crc, ch);
    sendDLEChar();
	ch = headerData[0];
    crc32Update(&crc, ch);
    sendDLEChar();
   	ch = headerData[1];
    crc32Update(&crc, ch);
    sendDLEChar();
	ch = headerData[2];
    crc32Update(&crc, ch);
    sendDLEChar();
	ch = headerData[3];
    crc32Update(&crc, ch);
    sendDLEChar();
	crc = ~crc;
	for(int i=0;i<4;i++)
	{
   		ch=(unsigned char)crc;
		sendDLEChar();
		crc >>= 8;
	}
}


		//zmodem software escapes ZDLE (0x18), 0x10, 0x90, 0x11, 0x91, 0x13,
        //and 0x93.
        //If preceded by 0x40 or 0xc0 (@), 0x0d and 0x8d are also escaped to
        //protect the Telenet command escape CR-@-CR. The receiver ignores
        //0x11, 0x91, 0x13, and 0x93 characters in the data stream.
//-----------------------------------------------------------------------------
// purpose: masking special characters withZDLE
//			HEX: 0x18(ZDLE selbst) 0x10 0x90 0x11 0x91 0x13 0x93 0xFF
//			OCT:  030               020 0220  021 0221  023 0223 0377
void CSpoofLPDlg::sendDLEChar()
//-----------------------------------------------------------------------------
{
	//
	ch &= 0xFF;
	switch(ch)
	{
   		case ZDLE:
      		sendChar(ZDLE);
			sendChar(ch ^= 0x40);
			break;
		case 021:
		case 0221:
		case 023:
		case 0223:
      		sendChar(ZDLE);
			sendChar(ch ^= 0x40);
			break;
		default:
			if((ch & 0140) == 0 && ch != 0 && ch != 2 && ch != 0x0d && ch != 3 && ch != 1 && ch != 0x0b && ch != 0x80 && ch != 4)//GWFdirty
			{
				sendChar(ZDLE);
				ch ^= 0x40;
			}
			sendChar(ch);
			break;
	}
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendChar(int c)
//-----------------------------------------------------------------------------
{	//
	unsigned char buf[1];
	//
	buf[0] = (unsigned char)c;
	WriteBlock(buf, 1);
    if(send_ch_sniffer){
          put_ch_outbuff(ch);
    }


}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::getZMHeader()
//-----------------------------------------------------------------------------
{	//
	int count;
	int canCount, dleCount;
	//
	gotHeader = 0;
	canCount = 0;
	dleCount = 0;
	getNextCh();
	while(ALLOK && !gotHeader && !cancel)
	{
		while (ALLOK && (ch != ZPAD) && !cancel)
		{

			if(ch == ZCANCEL)
			{
				canCount ++;
				if(canCount == 5)
				{
					headerType = ZCAN;
					SetLastError(ERROR_RECV_ZCAN);
					break;
				}
			}
			if(ch == ZDLE)
			{
				dleCount ++;
				if(dleCount == 8)
				{
					headerType = ZABORT;
					SetLastError(ERROR_RECV_ZABORT);
					break;
				}
			}

			getNextCh();
		}
		if(ALLOK)
		{
			count = 1;
			getNextCh();
			while (ALLOK && (ch == ZPAD) && !cancel)
			{
				count++;
				getNextCh();
			}
			if(cancel)
				break;
			if(ALLOK && (ch == ZDLE))
			{
				getNextCh();
				if(ALLOK)
				{
					if (ch == ZBIN)
					{
						frameType = ZBIN;
						getBinaryHeader();
					}
					else if (ch == ZBIN32)
					{
						frameType = ZBIN32;
						getBin32Header();
					}
					else if ((ch == ZHEX) && (count >= 2))
					{
						frameType = ZHEX;
						getHexHeader();
					}
				}
			}
		}

	}
	if(gotHeader)
	{
#ifdef _DEBUG
		char out[1000];
		wsprintf(out,"headerdata 0: %u 1: %u 2: %u 3: %u\n",headerData[0],
           			headerData[1],headerData[2],headerData[3]);
		TRACE(out);
#endif
		;
	}


}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::getNextCh()
//-----------------------------------------------------------------------------
{	//
	unsigned char buf[1];
	DWORD actual;
    DWORD dwLastError;
	//
	if(m_bWait)
		GetBlock(buf,1,&actual);
	else
		GetBlockImm(buf,1,&actual);

	if(ALLOK && (actual == 1)){
		ch = buf[0];
        if (receive_ch_sniffer)
            put_ch_inbuff(ch);
    }else {
         dwLastError=GetLastError();
         sprintf(temp_buff, "Failed getNextCh() with NumOfChar = %d, GetLastError() =  %d", actual, dwLastError);
         if (dwLastError)
               SetLastError(0);
         logFileMessage(temp_buff);
    }
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::getBinaryHeader()
//-----------------------------------------------------------------------------
{	//
	CRCXM crc;
	unsigned int theirCRC;
	//
	getNextDLECh();
	while(ALLOK && !gotHeader && !cancel)
	{
   		crcxmInit(&crc);
		headerType = (unsigned char)ch;
		TRACE("got headertype %lu\n",headerType);
		crcxmUpdate(&crc, ch);
		getNextDLECh();
		if(ALLOK && !cancel)
		{
      		headerData[0] = (unsigned char)ch;
			crcxmUpdate(&crc, ch);
			getNextDLECh();
			headerData[1] = (unsigned char)ch;
			crcxmUpdate(&crc, ch);
			getNextDLECh();
	      	headerData[2] = (unsigned char)ch;
			crcxmUpdate(&crc, ch);
			getNextDLECh();
	      	headerData[3] = (unsigned char)ch;
			crcxmUpdate(&crc, ch);
		    getNextDLECh();
		}
		if(ALLOK && !cancel)
		{
      		theirCRC = ch;
			getNextDLECh();
		}
		if(ALLOK && !cancel)
		{
      		theirCRC = (theirCRC << 8) | ch;
			if (crcxmValue(&crc) != theirCRC)
			{
				TRACE("set error %s\n","ZMODEM_CRCXM");
				SetLastError(ZMODEM_CRCXM);
			}
			else
				gotHeader = 1;
		}
	}
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::getBin32Header()
//-----------------------------------------------------------------------------
{	//
	CRC32 crc;
	DWORD theirCRC;
	//
	getNextDLECh();
	while(ALLOK && !gotHeader)
	{
   		crc32Init(&crc);
		headerType = (unsigned char)ch;
		TRACE("got headertype %lu\n",headerType);
		crc32Update(&crc, ch);
		getNextDLECh();
		if(ALLOK)
		{
			headerData[0] = (unsigned char)ch;
			crc32Update(&crc, ch);
			getNextDLECh();
			headerData[1] = (unsigned char)ch;
			crc32Update(&crc, ch);
			getNextDLECh();
      		headerData[2] = (unsigned char)ch;
			crc32Update(&crc, ch);
			getNextDLECh();
      		headerData[3] = (unsigned char)ch;
			crc32Update(&crc, ch);
			getNextDLECh();
		}
		if(ALLOK)
		{
      		theirCRC = (unsigned long)ch;
			getNextDLECh();
			theirCRC = theirCRC | ((unsigned long)ch << 8);
			getNextDLECh();
      		theirCRC = theirCRC | ((unsigned long)ch << 16);
			getNextDLECh();
      		theirCRC = theirCRC | ((unsigned long)ch << 24);
			if (~crc32Value(&crc) != theirCRC)
			{
				TRACE("set error %s\n","ZMODEM_CRC32");
         		SetLastError(ZMODEM_CRC32);
			}
			else
				gotHeader = 1;
		}
	}
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::getHexHeader()
//-----------------------------------------------------------------------------
{	//
	CRCXM crc;
	unsigned int theirCRC;
	//
	getNextHexCh();
	while(ALLOK && !gotHeader && !cancel)
	{
		crcxmInit(&crc);
		headerType = (unsigned char)ch;
		TRACE("got headertype %lu\n",headerType);
		crcxmUpdate(&crc, ch);
		getNextHexCh();
		if(ALLOK && !cancel)
		{
			headerData[0] = (unsigned char)ch;
			crcxmUpdate(&crc, ch);
			getNextHexCh();
			headerData[1] = (unsigned char)ch;
			crcxmUpdate(&crc, ch);
			getNextHexCh();
			headerData[2] = (unsigned char)ch;
			crcxmUpdate(&crc, ch);
			getNextHexCh();
			headerData[3] = (unsigned char)ch;
			crcxmUpdate(&crc, ch);
			getNextHexCh();
		}
		if(cancel)
			break;
		if(ALLOK)
		{
			theirCRC = ch;
			getNextHexCh();
		}
		if(ALLOK && !cancel)
		{
			theirCRC = (theirCRC << 8) | ch;
			if (crcxmValue(&crc) != theirCRC)
			{
				TRACE("set error %s\n","ZMODEM_CRCXM");
				SetLastError(ZMODEM_CRCXM);
			}
			else
			{
				gotHeader = 1;
			}
		}

	}
}

//---Get a byte and retry and wait up to  Mins --------------------------------------------------------------------------
void CSpoofLPDlg::GetBlock(void *buffer,DWORD max,LPDWORD actual)
//-----------------------------------------------------------------------------
{
	int x;
    int w = 0;
    int cnt;
	//Routine
	x= ReadBuffer(buffer,max);
	if(x==0)
	{
//logFileMessage("Waiting to get byte from Titan...");
w = 1;
        // wait for 2 Hrs 10 seconds at a time
		for (cnt=0;cnt<1000;cnt++ && !cancel)
		{
			Sleep(10);

			x= ReadBuffer(buffer,max);
			if(x!=0)
				break;
		}
	}
	if(x==0)
	{
// logFileMessage("Timed out after 10 seconds waiting for a byte from Titan");
 if (receive_ch_sniffer &&  sniffer_receive_index > 0){
#if LOG_DEBUG == 1
     logFileMessage("\nBytes received <<<<");
     hexdump(sniffer_receive_buff, sniffer_receive_index, false);
     sniffer_receive_index = 0;
#endif
}
		TRACE("Setze Fehler %s\n","ZMODEM_TIMEOUT");
		SetLastError(ZMODEM_TIMEOUT);
	} else if (w){
        //sprintf(temp_buff,"Got the byte waited for after %d seconds", (cnt * 10)/1000);
        //logFileMessage(temp_buff);
        ;
    }
	*actual = x;
}

// Get immediate no retry or waiting -----------------------------------------------------------------------------
void CSpoofLPDlg::GetBlockImm(void *buffer,DWORD max,LPDWORD actual)
//-----------------------------------------------------------------------------
{
	int x;

	x= ReadBuffer(buffer,max);
	if(x==0)
	{
logFileMessage("Timed out in GetBlockImm() waiting for byte from Titan, returning '00' ");
		TRACE("set error %s\n","ZMODEM_TIMEOUT");
		SetLastError(ZMODEM_TIMEOUT);
	}
	*actual=x;
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::getNextDLECh()
//-----------------------------------------------------------------------------
{
	gotSpecial = 0;
	getNextCh();
	if(ALLOK)
	{
		if(ch == ZDLE)
		{
      		getNextCh();
			if(ALLOK)
			{
         		if(((ch & 0x40) != 0) && ((ch & 0x20) == 0))
				{
					ch &= 0xbf;
					gotSpecial = 0;
				}
				else
				{
            		gotSpecial = 1;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::getNextHexCh()
//-----------------------------------------------------------------------------
{	//
	int tempCh;
	//
	getNextCh();
	if(ALLOK)
	{
		if ((ch <= 0x39) && (ch >= 0x30))
		{
			tempCh = (ch - 0x30);
		}
		else if ((ch >= 0x61) && (ch <= 0x66))
		{
			tempCh = (ch - 0x61) + 0x0a;
		}
		else if ((ch >= 0x41) && (ch <= 0x46))
		{
			tempCh = (ch - 0x41) + 0x0a;
		}
		else
		{
			SetLastError(ZMODEM_BADHEX);
			TRACE("set error %s\n","ZMODEM_BADHEX");
		}
		if(ALLOK)
			getNextCh();
		if(ALLOK)
		{
			tempCh = tempCh << 4;
			if ((ch <= 0x39) && (ch >= 0x30))
			{
         		ch = (ch - 0x30);
			}
			else if ((ch >= 0x61) && (ch <= 0x66))
			{
         		ch = (ch - 0x61) + 0x0a;
			}
			else if ((ch >= 0x41) && (ch <= 0x46))
			{
				ch = (ch - 0x41) + 0x0a;
			}
			else
			{
				TRACE("set error %s\n","ZMODEM_BADHEX");
         		SetLastError(ZMODEM_BADHEX);
			}
		}
		if(ALLOK)
		{
      		ch = ch | tempCh;
		}
	}
}
//CE_RXPARITY
//-----------------------------------------------------------------------------
DWORD CSpoofLPDlg::ReadBuffer(void *buffer, DWORD num)
//-----------------------------------------------------------------------------
{	//Variablen
	DWORD dwRead,numWaiting;
	COMSTAT cst;
	DWORD err;
	//Routine
	if(!ClearCommError(port.hComm,&err,&cst))
	{
		TRACE("error ClearCommError() %lu\n",GetLastError());
		return(0);
	}
	numWaiting=cst.cbInQue;
	if(numWaiting==0)
		return(0);
	if(num > numWaiting)
		num = numWaiting;
	//start overlapped io
	OVERLAPPED ov={0,0,0,0,NULL};
	ov.hEvent=CreateEvent(NULL,true,true,NULL);
	HANDLE waitfor[2];

	//create the abort event to end all threads
	m_hCancelEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	waitfor[0] = m_hCancelEvent;
	waitfor[1] = ov.hEvent;
	DWORD dwHandleSignaled;
	if(!SetupReadEvent(&ov,buffer,num,&dwRead))
	{
		while(true && ALLOK && !cancel)
		{
			dwHandleSignaled=WaitForMultipleObjects(2,waitfor,false,10000);
			// Which event occured?
			switch(dwHandleSignaled)
			{
				case WAIT_OBJECT_0:    //cancelled
				{   // Time to exit.
   					SetLastError(ZMODEM_ABORTFROMOUTSIDE);
      				goto EndZModem;
				}
   				case WAIT_OBJECT_0 + 1: // ReadEvent signaled.
      			{
         			if(HandleReadEvent(&ov,buffer,num,&dwRead))
						goto EndZModem;
					break;
				}
	   			default:
            		goto EndZModem;
			}
		}
	}
EndZModem:

	CloseHandle(ov.hEvent);

	CloseHandle(m_hCancelEvent);

	//all data received
	return(dwRead);
}

//-----------------------------------------------------------------------------
bool CSpoofLPDlg::SetupReadEvent(LPOVERLAPPED lpOverlappedRead,void* lpszInputBuffer,
					           DWORD dwSizeofBuffer,LPDWORD lpnNumberOfBytesRead)
//-----------------------------------------------------------------------------
{
	DWORD dwLastError;

	if(ReadFile(port.hComm,lpszInputBuffer,dwSizeofBuffer,lpnNumberOfBytesRead,
   				lpOverlappedRead))
	{
		TRACE("ReadFile successful in SetupReadEvent\n");
		return(true);
	}
	// ReadFile failed.  Expected because of overlapped I/O.
	dwLastError=GetLastError();
	// LastError was ERROR_IO_PENDING, as expected.
	if(dwLastError==ERROR_IO_PENDING)
		return(false);
	SetLastError(ZMODEM_TIMEOUT);
	return(true);//ok
}

//-----------------------------------------------------------------------------
bool CSpoofLPDlg::HandleReadEvent(LPOVERLAPPED lpOverlappedRead,void* lpszInputBuffer,
						         DWORD dwSizeofBuffer,LPDWORD lpnNumberOfBytesRead)
//-----------------------------------------------------------------------------
{
	if(GetOverlappedResult(port.hComm,lpOverlappedRead,lpnNumberOfBytesRead,true))
	{
		TRACE("GetOverlapped succesfull in HandleReadEvent\n");
  		SetLastError(0);
	  	return(true);
	}
 sprintf(temp_buff, "GetOverlapped Failed in HandleReadEvent GetLastError() = %d", GetLastError());
 logFileMessage(temp_buff);

	return(false);
}

//-----------------------------------------------------------------------------
bool CSpoofLPDlg::sendFiles()
//-----------------------------------------------------------------------------
{	//
	bool gotfile;
	int count = 0;
	bool rw=true;
	int i=0;
	bool fileinfosent=false;
	DWORD offset;

	int nSize = 1;


	int tries, sent;

	while(ALLOK && ( i < nSize) && !cancel)
	{
		gotfile = Open(".\\sendfile",OPEN_EXISTING);
     	if(gotfile)
			TRACE("SendFiles filename %s\n",".\\sendfile");
		else
      		break;
		goodOffset = 0;
		tries = 0;
		sent = false;
		while(ALLOK && (tries < 10) && !sent && !cancel)
		{
			if(!fileinfosent)
			{
				if(error == 14)
				{
					//sendZEOF();
					WriteBlock("\x18g", 2);
					return false;
				}
				sendZFILE();
				sendFILEINFO();
				fileinfosent=true;
			}
			getZMHeader();
			if(ALLOK && !cancel)
			{
				if(headerType == ZRINIT)
				{
            		TRACE("got ZRINIT\n");
					tries++;
				}
				else if (headerType == ZRPOS)
				{

           			TRACE("got ZRPOS\n");
					offset=(headerData[3] << 24) | (headerData[2] << 16) |
							 (headerData[1] << 8)  | headerData[0];
					SetPos(offset);
					goodOffset = offset;
					threadStarted = TRUE;
					//rw=sendFile(NULL);
					sendFileThread();
					while(threadStarted)
						Pump();

					if(headerType != ZRINIT)
						rw = false;


					sent = true;//
				}
				else if(headerType == ZNAK)//nochmal senden
				{
            		TRACE("got ZNAK\n");
					fileinfosent=false;
				}
				else if(headerType == ZSKIP)//bitte nächstes file
				{
					TRACE("got ZSKIP\n");
            		sent=true;
				}
				else if(headerType== ZCRC)
				{
					TRACE("got ZCRC, no action\n");
				}
			}
			else
			{
				MessageBox("Error sending ZFILE!", "Error", MB_OK | MB_ICONERROR);
				logFile(false, ERROR_ZFILE);
				rw = false;
			}
		}

		Finish();
		i++;//next file
	}

	return rw;
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendZACK()
//-----------------------------------------------------------------------------
{
	frameType = ZHEX;
	headerType = ZACK;
/*	headerData[0] = is already filled, bouncing back
	headerData[1] =
	headerData[2] =
	headerData[3] =*/
	sendHeader();
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendZFIN()
//-----------------------------------------------------------------------------
{
	frameType = ZHEX;
	headerType = ZFIN;
	headerData[0] = 0x00;
	headerData[1] = 0x00;
	headerData[2] = 0x00;
	headerData[3] = 0x00;
	sendHeader();
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendZCAN()
//-----------------------------------------------------------------------------
{
	//m_ZModemComm->WriteBlock("OO",2);
	WriteBlock("\x0d\x0a\x08\x08\x08\x08\x08",7);
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendOO()
//-----------------------------------------------------------------------------
{
	//m_ZModemComm->WriteBlock("OO",2);
	WriteBlock("OO",2);
}

//-----------------------------------------------------------------------------
bool CSpoofLPDlg::Open(CString filename,DWORD mode)
//-----------------------------------------------------------------------------
{	//Variablen
	bool ret=false;
	//Routine
	TRACE("opening file %s\n",filename);
	m_hFile=CreateFile(filename,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ,
								NULL,mode,FILE_ATTRIBUTE_NORMAL,NULL);
	if(m_hFile==INVALID_HANDLE_VALUE)
	{
		TRACE("error open file %s in CreateFile %lu\n",filename,GetLastError());
	}
	else
	{   //posting filename
		m_Filename=filename;
		//::PostMessage(m_hOwner,WM_USER_ZMODEMNEXTFILE,0,(LPARAM)(LPCTSTR)m_Filename);
		m_Filesize=::GetFileSize(m_hFile,NULL);
		m_Progress1.SetRange32(0, m_Filesize);
		if(m_Filesize==0xFFFFFFFF)
			TRACE("error GetFileSize %lu\n",GetLastError());
		else
		{	//posting filesize
			//::PostMessage(m_hOwner,WM_USER_ZMODEMNEXTFILE,0,(LPARAM)(LPCTSTR)m_Filename);
			//::PostMessage(m_hOwner,WM_USER_ZMODEMNEXTFILESIZE,0,(LPARAM)m_Filesize);
			ret=true;
		}
	}
	return ret;
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendZFILE()
//-----------------------------------------------------------------------------
{
	if(bcrc32)
		frameType= ZBIN32;
	else
   		frameType= ZBIN;
	headerType=ZFILE;
	headerData[0]=0x00;
	headerData[1]=0x00;
	headerData[2]=0x04;
	headerData[3]=0x01;//ZCBIN;
	sendHeader();
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendFILEINFO()
//-----------------------------------------------------------------------------
{	//
	unsigned char* buf;//buffer for FILEINFO
	int cnt;
	int x;
	CRCXM crc;
	CRC32 crc32;
	//
	buf = new unsigned char[500];
	memset(buf,0,500);
	//cnt = m_ZModemFile->MakeFileInfo(buf);
	cnt = MakeFileInfo(buf);
	if(bcrc32)
   		crc32Init(&crc32);
	else
   		crcxmInit(&crc);
	for(x = 0; x < cnt; x++)
	{
   		if(bcrc32)
      		crc32Update(&crc32, buf[x]);
		else
			crcxmUpdate(&crc, buf[x]);
	}
	// add CRC
	buf[cnt++] = ZDLE;
	buf[cnt++] = ZCRCW;
	if(bcrc32)
		crc32Update(&crc32, ZCRCW);
	else
		crcxmUpdate(&crc, ZCRCW);
	crc32 = ~crc32;

	if(bcrc32)
	{
		for(int i=4;--i>=0;)
		{
			buf[cnt++]=(unsigned char)crc32;
      		crc32 >>= 8;
		}
	}
	else
	{
		buf[cnt++] = (unsigned char)crcxmHighbyte(&crc);
		buf[cnt++] = (unsigned char)crcxmLowbyte(&crc);
	}
	//buf[cnt++] = 0x11;
	//m_ZModemComm->WriteBlock(buf,cnt);
	WriteBlock(buf,cnt);
	delete[] buf;
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::SetPos(DWORD offset)
//-----------------------------------------------------------------------------
{
	if(0xFFFFFFFF==SetFilePointer(m_hFile,offset,NULL,FILE_BEGIN))
   		TRACE("error SetFilePointer %lu\n",GetLastError());
	else
		TRACE("set filepointer to offset %lu\n",(DWORD)offset);
}

//-----------------------------------------------------------------------------
UINT sendFile(LPVOID pParam)
//-----------------------------------------------------------------------------
{	//
	int tries = 0;
	DWORD lastpos = 0;
	int state, erg;
	bool sentfile = false;
	bool rw=true;
	CString csTemp;

	CSpoofLPDlg* pObject = (CSpoofLPDlg*)pParam;

	state = SM_SENDZDATA;
	while(ALLOK && !sentfile && !pObject->cancel)
	{
		switch(state)
		{
			case SM_SENDZDATA:
				pObject->moreData = 1;
				erg=pObject->GetData(mainBuf,pObject->maxTx,&pObject->bytes);
				if(erg==ZMODEMFILE_NOMOREDATA)
				{
					TRACE("set state SM_SENDZEOF1");
					state = SM_SENDZEOF;
					pObject->moreData = 0;
				}
				else if(erg == ZMODEMFILE_OK)
				{
					TRACE("sendZDATA");
					pObject->sendZDATA();
					state = SM_SENDDATA;
					debug = true;
				}
				else//ZMODEMFILE_ERROR
				{
					TRACE("set error ZMODEM_ERROR_FILE1\n");
					SetLastError(ZMODEM_ERROR_FILE);
					rw = false;
				}
				break;

			case SM_SENDDATA:
				while (ALLOK && (state == SM_SENDDATA) && !pObject->cancel)
				{
					packetPos = packetBuf;
					pObject->sendData();
					pObject->sendPacket();
					DWORD err=GetLastError();
					if(pObject->error == 15 && pObject->goodOffset > 4096)
					{
						SetLastError(ERROR_CAUSE_ERROR);
						rw = false;
					}
					else
					if(err==ZMODEM_TIMEOUT)//got no header
					{
						TRACE("set error ZMODEM_ERROR_FILE2\n");
						SetLastError(ZMODEM_ERROR_FILE);
						rw = false;
					}
					else
					if(ALLOK && !pObject->cancel)
					{
						if(!pObject->moreData)
						{
							TRACE("break moreData = FALSE, state = SM_SENDZEOF\n");
							state = SM_SENDZEOF;
							debug = false;
							//MessageBox(NULL,"supposed to send ZEOF","look",NULL);
							break;
						}
						else
						{
							int erg = pObject->GetData(mainBuf,pObject->maxTx,&pObject->bytes);
							if(erg == ZMODEMFILE_NOMOREDATA)
							{
								TRACE("set moreData=0 ZMODEMFILE_NOMOREDATA\n");
								pObject->moreData = 0;
								//MessageBox(NULL,"moreData = 0","look",NULL);
							}
							else if(erg == ZMODEMFILE_OK)
							{
								TRACE("got ZMODEMFILE_OK\n");

								//csTemp.Format("%x\n", GetLastError());
								//TRACE(csTemp);
								continue;
							}
							else//ZMODEMFILE_ERROR
							{
								TRACE("set error ZMODEM_ERROR_FILE\n");
								SetLastError(ZMODEM_ERROR_FILE);
								rw = false;
							}
						}
					}
				}
				break;

			case SM_SENDZEOF:
				TRACE("in SM_SENDZEOF\n");
				pObject->getZMHeader();	// GWFdirty

				if(pObject->error == 16)
				{
					// sending nothing here or even a ZACK does not make the
					// Titan error, it waits for ZEOF i guess
					SetLastError(ERROR_CAUSE_ERROR);
					rw = false;
				}
				else
				{
					pObject->logFileMessage("File sent, sending initial ZEOF");
					pObject->sendZEOF();
					state = SM_WAITZRINIT;
				}
				break;

			case SM_WAITZRINIT:
				TRACE("in SM_WAITZRINIT\n");
				pObject->getZMHeader();
				if(ALLOK && (pObject->headerType == ZRINIT))
				{
					pObject->logFileMessage("Received ZRINIT in reponse to ZEOF");
					sentfile = true;
				}
				else if (ALLOK && pObject->headerType == ZACK)
					pObject->sendZEOF();
				//erweitert ...
				else if(ALLOK && (pObject->headerType == ZFERR))
				{
					TRACE("got ZFERR, finish sending\n");
					SetLastError(ZMODEM_ERROR_FILE);
					rw = false;
				}
				else if((pObject->headerType == ZCAN))
				{
					TRACE("got ZCAN\n");
					SetLastError(ZMODEM_ERROR_FILE);
					rw = false;
				}
				else if((pObject->headerType == ZABORT))
				{
					TRACE("got ZABORT\n");
					SetLastError(ZMODEM_ERROR_FILE);
					rw = false;
				}
				else
				{

					tries++;
					if(tries < 100)
					{
						pObject->sendZEOF();
						SetLastError(0);
					}
					else
					{
						pObject->logFileMessage("No ZRINIT received in response to ZEOF");
						SetLastError(ZMODEM_ERROR_FILE);
						rw = false;
					}
				}
				break;
		}//switch
	}//while

	csTemp.Format("%x\n", GetLastError());
	TRACE(csTemp);

	pObject->threadStarted = FALSE;

	return rw;
}

// sendfile2.h
unsigned long data1_len = 18;
unsigned char data1[18]=
{
	0x73,
	0x65,0x6E,0x64,0x66,0x69,0x6C,0x65,0x31,0x2E,0x62,0x69,
	0x6E,0x00,0x32,0x35,0x36,0x00,
};

// sendfile3.h
unsigned long data2_len = 256;
unsigned char data2[256]=
{
	0x49,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x15,0x4C,0x50,0x31,
	0x32,0x00,0x00,0x0A,0x4F,0x00,0xF0,0x00,0x30,0x30,0x30,
	0x2E,0x30,0x30,0x30,0x2E,0x30,0x30,0x30,0x2E,0x30,0x30,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x32,0x00,
	0x03,0x00,
};

// sendfile5.h
unsigned long data3_len = 256;
unsigned char data3[256]=
{
	0x52,
	0x58,0x0B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,
};

// sendfile8.h
unsigned long data4_len = 256;
unsigned char data4[256]=
{
	0x52,
	0x58,0x02,0x02,0x00,0x31,0x31,0x35,0x32,0x30,0x30,0x0D,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,
};

// sendfile10.h
unsigned long data5_len = 256;
unsigned char data5[256]=
{
	0x49,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x15,0x4C,0x50,
	0x31,0x35,0x00,0x00,0x0A,0x4F,0x00,0xF0,0x00,0x30,0x30,
	0x30,0x2E,0x30,0x30,0x30,0x2E,0x30,0x30,0x30,0x2E,0x30,
	0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x32,
	0x00,0x03,0x00,
};

unsigned long time_len = 256;
unsigned char time_dat[256]=
{
	0x52,0x58,0x0D,0x01,0x00,0xD1,0x2C,0xD6,0x4F,0xD4,0xFE,
	0xF4,0x03,0x30,0x32,0x30,0x32,0x30,0x32,0x30,0x30,0x30,
	0x30,0x31,0x30,0x30,0x31,0x30,0x32,0x30,0x30,0x30,0x30,
	0xC4,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,
};

unsigned long okay_len = 256;
unsigned char okay[256]=
{
	0x53,0x47,0x00,
	0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};




// sendfile18.h
unsigned long data6_len = 256;
unsigned char data6[256]=
{
	0x52,
	0x53,0x80,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x34,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3E,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x01,0x3E,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x08,0x01,0x00,0x00,0x05,0x00,0x00,
	0x19,0x04,0x00,0xDC,0x07,0x04,0x11,0x1A,0x03,0x00,
	0x10,0x0B,0x1C,0x04,0x03,0x00,0x32,0x00,0x01,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,
	0x01,0x00,0x07,0x00,
};
#define NUM_SENDFILES 6

unsigned long data25_len = 12;
unsigned char data25[12]=
{
	0x7B,
	0x70,0x68,0x79,0x3A,0x3A,0x74,0x65,0x72,0x6D,0x7D,0x0D,
};

unsigned long data26_len = 256;
unsigned char data26[256]=
{
	0x44,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

#define NUM_SENDFILES2 2






/*
Description of the SCP data and how to interpret it.  Initially the LP device
has 'control' which means it is the first to send.  All packets are sent with
ZRCW to inidicate a ZMODEM ZACK is expected.  The first four bytes of the ZMODEM
payload which consist of the beginning of SCP data packets will be stored.
Based on the first byte received, other bytes may need to be processed as below.


If SCP-ECG I is sent - expect to receive an SCP-ECG I packet in return
If RX<11> bus turn-around is sent - rcv ZACK, rcv SCP-ECG OK message, send ZACK, wait for next packet


bool respNeeded;
bool haveControl;


When receiving SCP-ECG packets:
SCP msg			hex val		byte1		byte2		ascii val		type		action
-------			-------		-----		-----		---------		----		------
SCP-ECG	I		0x49		  -			  -			I				info		Send ZACK, if have control,
																				send next data and rcv ZACK
SCP-ECG S		0x53		0x47		0x00		S				status		Send ZACK, 0x47 == G == OK
																				otherwise check byte 2
SCP-ECG RX<13>	0x52		0x58		0x0D		RX<13>			req date	Send ZACK, send next data and
																				rcv ZACK, send next data (first is OK, second data)
																				send ZACK, rcv OK
SCP-ECG RX<256>	0x52		0x58		0x0C		RX<256>			set date	send ZACK, send next data (OK), rcv ZACK
SCP-ECG RX<11>	0x52		0x58		0x0B		RX<11>			bus t/a		send ZACK, now in control, send OK, if more
																				packets, send them
*/


//-----------------------------------------------------------------------------
int CSpoofLPDlg::GetCommandData(void *buffer,DWORD max,LPDWORD bytes)
//-----------------------------------------------------------------------------
{
	int rw;
	rw = TRUE;


	if(sendOK)
	{
		memcpy(buffer, okay, okay_len);
		*bytes = okay_len;
	}
	else
	if(sendTime)
	{
		memcpy(buffer, time_dat, time_len);
		*bytes = time_len;
	}
	else
	if(sendfile == 1)
	{
		memcpy(buffer, data1, data1_len);
		*bytes = data1_len;
	}
	else
	if(sendfile == 2)
	{
		memcpy(buffer, data2, data2_len);
		*bytes = data2_len;
	}
	else
	if(sendfile == 3)
	{
		memcpy(buffer, data3, data3_len);
		*bytes = data3_len;
	}
	else
	if(sendfile == 4)
	{
		memcpy(buffer, data4, data4_len);
		*bytes = data4_len;
	}
	else
	if(sendfile == 5)
	{
		memcpy(buffer, data5, data5_len);
		*bytes = data5_len;
	}
	else
	if(sendfile == 6)
	{
		memcpy(buffer, data6, data6_len);
		*bytes = data6_len;
	}
	else
	if(sendfile == 25)
	{
		memcpy(buffer, data25, data25_len);
		*bytes = data25_len;
	}
	else
	if(sendfile == 26)
	{
		memcpy(buffer, data26, data26_len);
		*bytes = data26_len;
	}

	if(!sendOK && !sendTime)
		sendfile++;



	return(rw);
}



//int SCP[3];				// first 3 bytes of SCP data to be send
void CSpoofLPDlg::SetFlags()
{
	CString csTemp;

	if(mainBuf[0] == 0x52 &&	// 'R'
	   mainBuf[1] == 0x58 &&	// 'X'
	   mainBuf[2] == 0x0B &&	// Bus turnaround
	   mainBuf[3] == 0x01)
	{
		haveControl = FALSE;
		recvOK = TRUE;
		recvResponse = TRUE;
		sendResponse = FALSE;
		sendOK = FALSE;
	}

	if(mainBuf[0] == 0x52 &&
	   mainBuf[1] == 0x58 &&
	   mainBuf[2] == 0x02 &&
	   mainBuf[3] == 0x01)
	{

		sendOK = TRUE;
		recvOK = FALSE;
		recvResponse = FALSE;
		sendResponse = TRUE;
	}

	if(mainBuf[0] == 0x52 &&
	   mainBuf[1] == 0x58 &&
	   mainBuf[2] == 0x02 &&
	   mainBuf[3] == 0x02)
	{

		sendOK = FALSE;
		recvOK = TRUE;
		recvResponse = FALSE;
		sendResponse = TRUE;
	}

	if(mainBuf[0] == 0x49)			// 'I'nformation
	{
		recvResponse = TRUE;
	}

	if(mainBuf[0] == 0x44)
	{
		recvResponse = TRUE;
	}

	//0x52,0x58,0x0D,0x01,0x00,0x00
	// when i receive request for time
	if(mainBuf[0] == 0x52 &&	// 'R'
	   mainBuf[1] == 0x58 &&	// 'X'
	   mainBuf[2] == 0x0D &&
	   mainBuf[3] == 0x01)
	{
		recvOK = FALSE;
		recvResponse = FALSE;
		sendResponse = FALSE;
		sendOK = TRUE;
		sendTime = TRUE;
	}

	//0x52,0x58,0x0D,0x01,0x00,0xD1
	// when i send my time
	if(mainBuf[0] == 0x52 &&	// 'R'
	   mainBuf[1] == 0x58 &&	// 'X'
	   mainBuf[2] == 0x0D &&
	   mainBuf[3] == 0x01 &&
	   mainBuf[4] == 0x00 &&
	   mainBuf[5] == 0xD1)
	{
		recvOK = TRUE;
		recvResponse = TRUE;
		sendResponse = FALSE;
		sendOK = FALSE;
		sendTime = FALSE;
	}

	if(mainBuf[0] == 0x52 &&	// 'R'
	   mainBuf[1] == 0x58 &&	// 'X'
	   mainBuf[2] == 0x0C &&
	   mainBuf[3] == 0x03)
	{
		recvOK = FALSE;
		recvResponse = FALSE;
		sendResponse = FALSE;
		sendOK = TRUE;
		debug2 = true;
	}

	if(mainBuf[0] == 0x52 &&	// 'R'
	   mainBuf[1] == 0x58 &&	// 'X'
	   mainBuf[2] == 0x0E &&
	   mainBuf[3] == 0x04)
	{
		recvOK = FALSE;
		recvResponse = FALSE;
		sendResponse = FALSE;
		sendOK = TRUE;
		debug3 = true;
	}

	if(mainBuf[0] == 0x52 &&	// 'R'
	   mainBuf[1] == 0x58 &&	// 'X'
	   mainBuf[2] == 0x0B &&
	   mainBuf[3] == 0x04)
	{
		recvOK = FALSE;
		recvResponse = FALSE;
		sendResponse = TRUE;
		sendOK = TRUE;
		debug4 = true;
	}

	// again!!!!gwfdirty
	if(mainBuf[0] == 0x52 &&	// 'R'
	   mainBuf[1] == 0x58 &&	// 'X'
	   mainBuf[2] == 0x02 &&
	   mainBuf[3] == 0x02)
	{
		recvOK = TRUE;
		recvResponse = FALSE;
		sendResponse = TRUE;
		sendOK = FALSE;
		changeBaud = TRUE;
	}

	if(mainBuf[0] == 0x52 &&	// 'R'
	   mainBuf[1] == 0x53 &&
	   mainBuf[2] == 0x80 &&
	   mainBuf[3] == 0x04)
	{
		recvOK = TRUE;
		recvResponse = FALSE;
		sendResponse = FALSE;
		sendOK = FALSE;
		exitCommands = TRUE;
	}

}

//-----------------------------------------------------------------------------
bool CSpoofLPDlg::sendCommandSeq2()
//-----------------------------------------------------------------------------
{	//
	int state;
	DWORD err;
	bool loop;
	bool ret = true;

	loop = true;
	state = SM_SENDZCOMMAND;
	sendfile = 25;
	haveControl = FALSE;
	sendResponse = TRUE;
	recvResponse = FALSE;
	recvOK = TRUE;
	sendOK = FALSE;
	exitCommands = FALSE;

	while(ALLOK && loop && !cancel)
	{

		switch(state)
		{
			case SM_SENDZCOMMAND:

				GetCommandData(mainBuf,maxTx,&bytes);	// from the next file
				sendZCOMMAND();
				sendCommand16(bytes, ZCRCW);
				state = SM_RECVZACK;
				break;

			case SM_RECVZACK:

				getZMHeader();
				err=GetLastError();
				if(ALLOK && (headerType == ZACK))
				{
					if(recvOK)
						state = SM_RECVOK;
					else
					if(recvResponse)
						state = SM_RECVRESP;
					else
					if(sendOK)
						state = SM_SENDDATA;
					else
					if(sendResponse || haveControl || sendTime)// || sendOK
						state = SM_SENDDATA;
					else
						state = SM_RECVDATA;

				}
				else
				if(ALLOK && (headerType == ZCAN))
				{
					logFile(false, ERROR_RECV_ZCAN);
					SetLastError(ERROR_RECV_ZCAN);
					ret = false;
					loop = false;
				}
				else
				if(ALLOK && (headerType == ZCOMPL))
				{
					loop = FALSE;
				}
				else
				{
					logFile(false, ERROR_RECV_ZACK);
					SetLastError(ERROR_RECV_ZACK);
					ret = false;
					loop = false;
				}
				break;

			case SM_RECVOK:

				recvOK = FALSE;

				getZMHeader();
				err=GetLastError();
				if(ALLOK && (headerType == ZDATA) && !cancel)
				{
					receiveData();
					if(ALLOK)
					{
						sendZACK();
						if(recvResponse)
							state = SM_RECVRESP;
						else
						if(sendResponse || haveControl || sendOK || sendTime)
							state = SM_SENDDATA;
						else
							state = SM_RECVDATA;
					}
					else
					{
						logFile(false, ERROR_RECV_SCP_ECG1);
						SetLastError(ERROR_RECV_SCP_ECG1);
						ret = false;
						loop = false;
					}
				}
				else
				if(ALLOK && (headerType == ZCAN))
				{
					logFile(false, ERROR_RECV_ZCAN);
					SetLastError(ERROR_RECV_ZCAN);
					ret = false;
					loop = false;
				}
				else
				if(ALLOK && (headerType == ZCOMPL))
				{
					loop = FALSE;
				}
				else
				{
					logFile(false, ERROR_RECV_DATA);
					SetLastError(ERROR_RECV_DATA);
					ret = false;
					loop = false;
				}
				break;

			case SM_RECVRESP:

				recvResponse = FALSE;
				getZMHeader();
				err=GetLastError();
				if(ALLOK && (headerType == ZDATA) && !cancel)
				{
					receiveData();
					if(ALLOK)
					{
						SetFlags();
						sendZACK();
						if(sendResponse || haveControl || sendOK || sendTime)
							state = SM_SENDDATA;
						else
							state = SM_RECVDATA;
					}
					else
					{
						logFile(false, ERROR_RECV_SCP_ECG2);
						SetLastError(ERROR_RECV_SCP_ECG2);
						ret = false;
						loop = false;
					}
				}
				else
				if(ALLOK && (headerType == ZCAN))
				{
					logFile(false, ERROR_RECV_ZCAN);
					SetLastError(ERROR_RECV_ZCAN);
					ret = false;
					loop = false;
				}
				else
				if(ALLOK && (headerType == ZACK))
					TRACE("Received second ZACK");
				else
				if(ALLOK && (headerType == ZCOMPL))
				{
					loop = FALSE;
				}
				else
				{
					logFile(false, ERROR_RECV_DATA);
					SetLastError(ERROR_RECV_DATA);
					ret = false;
					loop = false;
				}
				break;

			case SM_SENDDATA:

				GetCommandData(mainBuf,maxTx,&bytes);	// from the next file
				if(sendOK)
					sendOK = FALSE;
				else
				if(sendResponse)
					sendResponse = FALSE;
				else
				if(sendTime)
					sendTime = FALSE;
				SetFlags();
				sendZCOMMANDDATA();
				sendCommand16(bytes, ZCRCW);
				state = SM_RECVZACK;
				break;

			case SM_RECVDATA:

				getZMHeader();
				err=GetLastError();
				if(ALLOK && (headerType == ZDATA))
				{
					receiveData();
					if(ALLOK)
					{
						SetFlags();
						sendZACK();
						if(sendResponse || haveControl || sendOK)
							state = SM_SENDDATA;
						else
							state = SM_RECVDATA;
					}
					else
					{
						logFile(false, ERROR_RECV_DATA);
						SetLastError(ERROR_RECV_DATA);
						ret = false;
						loop = false;
					}
				}
				else
				if(ALLOK && (headerType == ZCOMPL))
				{
					loop = FALSE;
				}
				else
				if(ALLOK && (headerType == ZCAN))
				{
					logFile(false, ERROR_RECV_ZCAN);
					SetLastError(ERROR_RECV_ZCAN);
					ret = false;
					loop = false;
				}
				else
				{
					logFile(false, ERROR_RECV_DATA);
					SetLastError(ERROR_RECV_DATA);
					ret = false;
					loop = false;
				}
				break;


		}//switch
	}//while
	return ret;
}

//-----------------------------------------------------------------------------
bool CSpoofLPDlg::sendCommandSeq1()
//-----------------------------------------------------------------------------
{	//
	int state;
	DWORD err;
	bool loop;

	loop = true;
	state = SM_SENDZCOMMAND;
	sendfile = 1;
	haveControl = TRUE;
	sendResponse = FALSE;
	sendTime = FALSE;
	recvResponse = FALSE;
	recvOK = FALSE;
	sendOK = FALSE;
	changeBaud = FALSE;
	exitCommands = FALSE;
	baudChanged = FALSE;
	bool ret = true;
	bool sendit = true;

//logFileMessage("Started sendCommandSeq1()... ");
	while(ALLOK && loop && !cancel)
	{

		switch(state)
		{
			case SM_SENDZCOMMAND:
//logFileMessage("SM_SENDZCOMMAND: call sendZCommand...");
				GetCommandData(mainBuf,maxTx,&bytes);	// from the next file	// GWFhere
				sendZCOMMAND();
#if LOG_DEBUG == 1
sprintf(temp_buff,"\nSeq1 in SM_SENDZCOMMAND: Call sendZCOMMAND()...>>>>");
logFileMessage(temp_buff);
hexdump(sniffer_send_buff, sniffer_send_index, true);
#endif
				sendCommand16(bytes, ZCRCW);
#if LOG_DEBUG == 1
sprintf(temp_buff,"\nSeq1 in SM_SENDZCOMMAND: Call sendCommand16..>>>>");
logFileMessage(temp_buff);
hexdump(sniffer_send_buff, sniffer_send_index, true);
#endif
				state = SM_RECVZACK;
				break;

			case SM_RECVZACK:

    //        enable_receive_ch_sniffer(true);
//logFileMessage("SM_RECVZACK: Calling getZMHeader()....");
				getZMHeader();
#if LOG_DEBUG == 1
sprintf(temp_buff,"\nSeq1 in SM_RECVZACK: Called getZMHeader()...<<<<");
logFileMessage(temp_buff);
hexdump(sniffer_receive_buff, sniffer_receive_index, false);
#endif
    //        enable_receive_ch_sniffer(false);
				err=GetLastError();
#if LOG_DEBUG == 1
sprintf(temp_buff,"Seq1 in SM_RECVZACK: After getZMHeader() headerType = %d, GetLastError() = %d...", headerType, err);
logFileMessage(temp_buff);
#endif
				if(ALLOK && (headerType == ZACK))
				{
					if(recvOK)
						state = SM_RECVOK;
					else
					if(recvResponse)
						state = SM_RECVRESP;
					else
					if(sendOK)
						state = SM_SENDDATA;
					else
					if(sendResponse || haveControl || sendTime){
						state = SM_SENDDATA;
                        //("Recived response set state to SM_SENDATA");
                    }
					else
						state = SM_RECVDATA;

				}
				else
				if(exitCommands)
						loop = false;
				else
				if(ALLOK && (headerType == ZCAN))
				{
					logFile(false, ERROR_RECV_ZCAN);
					SetLastError(ERROR_RECV_ZCAN);
					ret = false;
					loop = false;
				}
				else
				{
					logFile(false, ERROR_RECV_ZACK);
					SetLastError(ERROR_RECV_ZACK);
					ret = false;
					loop = false;
				}
				break;

			case SM_RECVOK:

				recvOK = FALSE;

//logFileMessage("SM_RECVOK: Calling getZMHeader()....");
				getZMHeader();
#if LOG_DEBUG == 1
sprintf(temp_buff,"\nSeq1 in SM_RECVOK: Call getZMHeader()...<<<<");
logFileMessage(temp_buff);
hexdump(sniffer_receive_buff, sniffer_receive_index, false);
#endif
				err=GetLastError();
				if(ALLOK && (headerType == ZDATA))
				{
					receiveData();
					if(ALLOK)
					{
						if(error == 7 && sendfile == 5)
						{
							WriteBlock("\x18g", 2);
							sendOK = FALSE;
							return false;
						}
//logFileMessage("SM_RECVOK: calling sendZACK()....");
						sendZACK();
#if LOG_DEBUG == 1
logFileMessage("SendZACK >>>>") ;
hexdump(sniffer_send_buff, sniffer_send_index, true);
#endif
						if(recvResponse)
						{
							state = SM_RECVRESP;
						}
						else
						if(sendResponse || haveControl || sendOK || sendTime)
						{
							if(changeBaud)
							{
								if(!baudChanged)
								{
									changeBaudRate(115200);
									sendResponse = TRUE;
									baudChanged = TRUE;
								}
							}

							state = SM_SENDDATA;
						}
						else
						if(exitCommands)
							loop = false;
						else
						{
							state = SM_RECVDATA;
						}
					}
					else
					{
						logFile(false, ERROR_RECV_SCP_ECG1);
						SetLastError(ERROR_RECV_SCP_ECG1);
						ret = false;
						loop = false;
					}
				}
				else
				if(ALLOK && (headerType == ZCAN))
				{
					logFile(false, ERROR_RECV_ZCAN);
					SetLastError(ERROR_RECV_ZCAN);
					ret = false;
					loop = false;
				}
				else
				{
					logFile(false, ERROR_RECV_SCP_ECG1);
					SetLastError(ERROR_RECV_SCP_ECG1);
					ret = false;
					loop = false;
				}
				break;

			case SM_RECVRESP:

				recvResponse = FALSE;
//logFileMessage("SM_RECVRESP: Calling getZMHeader()....");
				getZMHeader();
#if LOG_DEBUG == 1
logFileMessage("\nSeq1 in SM_RECVRESP: Call getZMHeader()...<<<<");
hexdump(sniffer_receive_buff, sniffer_receive_index, false);
#endif
				err=GetLastError();
				if(ALLOK && (headerType == ZDATA))
				{
					receiveData();
#if LOG_DEBUG == 1
logFileMessage("\nSeq1 in SM_RECVRESP: receive data from Titan <<<<");
hexdump(sniffer_receive_buff, sniffer_receive_index, false);
#endif
					if(ALLOK)
					{
						SetFlags();
						if(error == 10 && debug2 == true)
						{
							WriteBlock("\x18g\x18g", 4);
							return false;
						}
						if(error == 11 && debug3 == true)
						{
							WriteBlock("\x18g", 2);
							return false;
						}
						sendZACK();
#if LOG_DEBUG == 1
logFileMessage("SendZACK >>>>") ;
hexdump(sniffer_send_buff, sniffer_send_index, true);
#endif
						if(sendResponse || haveControl || sendOK || sendTime)
							state = SM_SENDDATA;
						else
							state = SM_RECVDATA;
					}
					else
					{
						logFile(false, ERROR_RECV_SCP_ECG2);
						SetLastError(ERROR_RECV_SCP_ECG2);
						ret = false;
						loop = false;
					}
				}
				else
				if(ALLOK && (headerType == ZCAN))
				{
					logFile(false, ERROR_RECV_ZCAN);
					SetLastError(ERROR_RECV_ZCAN);
					ret = false;
					loop = false;
				}
				else
				{
					logFile(false, ERROR_RECV_SCP_ECG2);
					SetLastError(ERROR_RECV_SCP_ECG2);
					ret = false;
					loop = false;
				}
				break;

			case SM_SENDDATA:
#if LOG_DEBUG == 1
sprintf(temp_buff,"In SM_SENDDATA with error = %d", error);
logFileMessage(temp_buff);
#endif
				if(error == 4 && sendfile == 2)
				{
					WriteBlock("\x18g", 2);
					sendfile++;		// don't send the SCP-ECG I packet, send something else
					sendit = false;
				}

				if(error == 5 && sendfile == 3)
				{
					WriteBlock("\x18g", 2);
					sendfile++;
					sendit = false;
				}

				if(error == 6 && sendfile == 4)
				{
					WriteBlock("\x18g", 2);
					return false;
				}

				//****************************!!!!!!!!!!!!!
				//if(error == 7 && !sendOK && sendfile == 4)
				//{
				//	WriteBlock("\x18g\x18g", 2);	// This doesn't work!!!!!!!
				//	sendOK = FALSE;//!!!!!!!!!!!!!!!!!!!!!!
				//	return false;// there is a bug in the Titan gateway code
				//}
				//****************************!!!!!!!!!!!!!

				if(error == 8 && sendTime)
				{
					WriteBlock("\x18g", 2);
					return false;
				}

				if(error == 9 && sendTime && !sendOK)
				{
					WriteBlock("\x18g", 2);
					return false;
				}

				if(error == 12 && sendfile == 6 && debug4)
				{
					WriteBlock("\x18g", 2);		// THIS IS a PROBLEM TOO
					return false;
				}

				if(error == 13 && sendfile == 6 && debug4 && !sendOK)
				{
					WriteBlock("\x18g", 2);		// THIS IS a PROBLEM TOO
					return false;
				}

				GetCommandData(mainBuf,maxTx,&bytes);	// from the next file

				if(sendOK)
					sendOK = FALSE;
				else
				if(sendResponse)
					sendResponse = FALSE;
				else
				if(sendTime)
					sendTime = FALSE;

				SetFlags();

				if(sendit)
				{
//logFileMessage("\nCalling sendZCOMMANDDATA()....>>>>");
   // enable_send_ch_sniffer(true);
//logFileMessage(" SM_SENDDATA:: Calling sendZCOMMABDATA()....");
					sendZCOMMANDDATA();
#if LOG_DEBUG == 1
   hexdump(sniffer_send_buff, sniffer_send_index, true);
#endif
      // enable_send_ch_sniffer(true);
//   logFileMessage(" SM_SENDDATA:: Calling sendCommand16()....");
					sendCommand16(bytes, ZCRCW);
#if LOG_DEBUG == 1
   hexdump(sniffer_send_buff, sniffer_send_index, true);
#endif
  // enable_send_ch_sniffer(false);

				}

				if(debug5)
					return false;

				if(error == 4 && sendfile == 4)		// GetCommand data increased sendfile too!!
					return false;
				if(error == 5 && sendfile == 5)
					return false;
				if(error == 7 && sendfile == 6)
					return false;
				if(error == 9 && sendfile == 7)
					return false;

				state = SM_RECVZACK;
				break;

			case SM_RECVDATA:
//logFileMessage("SM_RECVDATA Calling getZMHeader())...");
				getZMHeader();
#if LOG_DEBUG == 1
sprintf(temp_buff,"\nSeq1 in SM_RECVDATA: Call getZMHeader()...<<<<");
logFileMessage(temp_buff);
hexdump(sniffer_receive_buff, sniffer_receive_index, false);
#endif
				err=GetLastError();
				if(ALLOK && (headerType == ZDATA))
				{
					receiveData();
					if(ALLOK)
					{
				        if(error == 11 && debug2 == true)
				        {
				        	WriteBlock("\x18g", 2);
				        	return false;
				        }

						SetFlags();
						sendZACK();

						if(sendResponse || haveControl || sendOK || sendTime)
							state = SM_SENDDATA;
						else
							state = SM_RECVDATA;
					}
					else
					{
						logFile(false, ERROR_RECV_DATA);
						SetLastError(ERROR_RECV_DATA);
						ret = false;
						loop = false;
					}
				}
				else
				if(ALLOK && (headerType == ZCAN))
				{
					logFile(false, ERROR_RECV_ZCAN);
					SetLastError(ERROR_RECV_ZCAN);
					ret = false;
					loop = false;
				}
				else
				{
					logFile(false, ERROR_RECV_DATA);
					SetLastError(ERROR_RECV_DATA);
					ret = false;
					loop = false;
				}
				break;


		}//switch
	}//while
	return true;
}

void CSpoofLPDlg::changeBaudRate(int rate)
{
	DCB dcb;
	memset(&dcb,0,sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

logFileMessage("Change baud rate from 38400 to 115200");
Sleep(300);
	GetCommState(port.hComm, &dcb);
	dcb.BaudRate = rate;
	SetCommState(port.hComm, &dcb);
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::receiveData()
//-----------------------------------------------------------------------------
{	//

	moreData = 1;
	while(ALLOK && !cancel)
	{
		if(moreData)
		{
			getData16();
			if(ALLOK)
			{
			}
		}
		else
			break;
	}
}



//-----------------------------------------------------------------------------
void CSpoofLPDlg::getData16()
//-----------------------------------------------------------------------------
{	//
	int quit;
	CRCXM crc;
	unsigned int theirCRC;
	//
	bufPos = mainBuf;
	bufTop = bufPos + 1024;
	quit = 0;
	crcxmInit(&crc);

	while(ALLOK && (bufPos < bufTop) && !quit && !cancel)
	{

		getNextDLECh();
		if(ALLOK)
		{
			if(gotSpecial)
			{
				if(ch != ZCRCG)
				{
					moreData = 0;
				}
				crcxmUpdate(&crc, ch);
				getNextDLECh();
				if(ALLOK)
				{
					theirCRC = ch;
					getNextDLECh();
				}

				if(ALLOK)
				{
					theirCRC = (theirCRC << 8) | ch;
					if(crcxmValue(&crc) != theirCRC)
					{
						TRACE("set error %s\n","ZMODEM_CRCXM");
						SetLastError(ZMODEM_CRCXM);
					}
					else
					{
						goodOffset += (bufPos - mainBuf);
						quit = 1;
					}
				}
			}
			else
			{
				crcxmUpdate(&crc, ch);
				*bufPos = (unsigned char)ch;
				bufPos++;
			}
		}
	}

	if(bufPos == bufTop)
	{
		TRACE("set error %s\n","ZMODEM_LONGSP");
		SetLastError(ZMODEM_LONGSP);
	}
}


//-----------------------------------------------------------------------------
void CSpoofLPDlg::Finish()
//-----------------------------------------------------------------------------
{
	if(!CloseHandle(m_hFile))
	{
		TRACE("error CloseHandle %lu\n",GetLastError());
	}
}

//-----------------------------------------------------------------------------
int CSpoofLPDlg::MakeFileInfo(unsigned char* buf)
//-----------------------------------------------------------------------------
{
	int cnt=0;

	strcpy((char*)buf,"filename");
	cnt = strlen((char*)buf) + 1;

	wsprintf((char*)buf + cnt, "%ld", m_Filesize);
	cnt = cnt + strlen((char*)buf + cnt);

	wsprintf((char*)buf + cnt, " 0 0 0 0 ");
	cnt = cnt + strlen((char*)buf + cnt);

	wsprintf((char*)buf + cnt, "%ld", m_Filesize);
	cnt = cnt + strlen((char*)buf + cnt) + 1;


	return cnt;
}

//-----------------------------------------------------------------------------
int CSpoofLPDlg::GetData(void *buffer,DWORD max,LPDWORD bytes)
//-----------------------------------------------------------------------------
{
	int rw=ZMODEMFILE_OK;
	//DWORD dwLastError;

	if(::ReadFile(m_hFile,buffer,max,bytes,NULL)==0)
	{
		TRACE("error ReadFile %lu\n",GetLastError());
		//ERROR_IO_PENDING  - 997
		//ERROR_NOACCESS - 998
		rw = ZMODEMFILE_ERROR;
	}
	else
	{
		if(*bytes == 0 ) //End of file reached
		{
			TRACE("reached end of file\n");
			rw=ZMODEMFILE_NOMOREDATA;
		}
		else if(max != *bytes)
		{
			//if(dwLastError!=ERROR_IO_PENDING)

			TRACE("try to read %lu bytes\n",max);
			TRACE("only %lu bytes read\n",*bytes);
			moreData = 0;	// GWFdirty
		}
		else
		{
			TRACE("%lu bytes read\n",*bytes);
		}
	}
	return(rw);
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendZDATA()
//-----------------------------------------------------------------------------
{
	if(bcrc32)
		frameType=ZBIN32;
	else
		frameType=ZBIN;
	headerType=ZDATA;
	headerData[0]=0;//goodOffset & 0xff;
	headerData[1]=0;//(goodOffset >> 8) & 0xff;
	headerData[2]=0;//(goodOffset >> 16) & 0xff;
	headerData[3]=0;//(goodOffset >> 24) & 0xff;
	sendHeader();
}



//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendData()
//-----------------------------------------------------------------------------
{	//
	char sptype;
	//unsigned long setProgress;
	//
	// send ZCRCG if more data, ZCRCE otherwise
	//sptype = moreData ? ZCRCG : ZCRCE;
	if(!moreData)// || goodOffset == (unsigned long)m_Filesize) what if the file size is a multiple of 1024 bytes?
		sptype = ZCRCE;
	else
	{
		if(error == 15 && goodOffset >= 4096)
			sptype = (char)0xFF;	// try and cause an erro
		else
			sptype = ZCRCG;
	}

	if(bcrc32)
	   sendData32(bytes,sptype);
	else
   		sendData16(bytes,sptype);
	goodOffset = goodOffset + bytes;



	// Titan still has to send the data to the server
	if(goodOffset < (m_Filesize * .9))
	{
		m_Progress1.SetPos(goodOffset);
	}
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendData32(int len,char frameend)
//-----------------------------------------------------------------------------
{
	unsigned long crc;

	crc32Init(&crc);
	for(int i=0;i<len;i++)
	{
		ch=mainBuf[i];
		crc32Update(&crc,ch);
   		sendDLEChar();
	}
	crc32Update(&crc,frameend);
  	crc = ~crc;
  	ch = ZDLE;
	sendChar();
	ch = frameend;
	sendChar();
	for(int j=0;j<4;j++)
	{
		ch = (unsigned char)crc;
		sendDLEChar();
     	crc >>= 8;
	}
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendData16(int len,char frameend)
//-----------------------------------------------------------------------------
{
	CRCXM	crcxm;

	crcxmInit(&crcxm);
	for(int i=0;i<len;i++)
	{
		ch=mainBuf[i];
		crcxmUpdate(&crcxm,ch);
   		sendDLEChar();
	}
	crcxmUpdate(&crcxm, frameend);

  	ch = ZDLE;
	sendChar();
	ch = frameend;
	sendChar();
	for(int j=0;j<2;j++)
	{
		ch = (unsigned char)crcxm;
		sendDLEChar();
     	crcxm >>= 8;
	}
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendZCOMMAND()
//-----------------------------------------------------------------------------
{
	if(bcrc32)
		frameType=ZBIN32;
	else
		frameType=ZBIN;
	headerType=ZCOMMAND;
	headerData[0]=0;//goodOffset & 0xff;
	headerData[1]=0;//(goodOffset >> 8) & 0xff;
	headerData[2]=0;//(goodOffset >> 16) & 0xff;
	headerData[3]=0;//(goodOffset >> 24) & 0xff;
	sendHeader();
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendZCOMMANDDATA()
//-----------------------------------------------------------------------------
{
	if(bcrc32)
		frameType=ZBIN32;
	else
		frameType=ZBIN;
	headerType=ZDATA;
	headerData[0]=0;//goodOffset & 0xff;
	headerData[1]=0;//(goodOffset >> 8) & 0xff;
	headerData[2]=0;//(goodOffset >> 16) & 0xff;
	headerData[3]=0;//(goodOffset >> 24) & 0xff;
	sendHeader();
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendCommand16(int len,char frameend)
//-----------------------------------------------------------------------------
{
	CRCXM	crcxm;

	crcxmInit(&crcxm);
	for(int i=0;i<len;i++)
	{
		ch=mainBuf[i];
		crcxmUpdate(&crcxm,ch);
   		sendDLEChar();
	}
	crcxmUpdate(&crcxm, frameend);

  	ch = ZDLE;
	sendChar();
	ch = frameend;
	sendChar();
	for(int j=0;j<2;j++)
	{
		ch = (unsigned char)crcxm;
		sendDLEChar();
     	crcxm >>= 8;
	}
}


//-----------------------------------------------------------------------------
void CSpoofLPDlg::getZMHeaderImmediate()
//-----------------------------------------------------------------------------
{
	m_bWait = false;
	getZMHeader();
	m_bWait = true;
}

//-----------------------------------------------------------------------------
void CSpoofLPDlg::sendZEOF()
//-----------------------------------------------------------------------------
{
	//frameType = ZHEX;
	frameType = ZBIN;
	headerType = ZEOF;
	headerData[0] = (unsigned char)(goodOffset);
	headerData[1] = (unsigned char)((goodOffset) >> 8);
	headerData[2] = (unsigned char)((goodOffset) >> 16);
	headerData[3] = (unsigned char)((goodOffset) >> 24);
	sendHeader();
}

void CSpoofLPDlg::createErrorMessage(int error, void *buf)
{

	sprintf((char *)buf, "ERROR %x: ", error);

	switch(error)
	{
		case ERROR_OPEN_PORT:
			strcat((char *)buf, "Could not open COM port");
			break;
		case ERROR_ZMODEM_INIT:
			strcat((char *)buf, "Error initializing ZMODEM");
			break;
		case ERROR_ZCOMMAND_1:
			strcat((char *)buf, "Error sending ZCOMMAND sequence 1");
			break;
		case ERROR_ZSEND_FILES:
			strcat((char *)buf, "Error sending file");
			break;
		case ERROR_ZMODEM_HEADER:
			strcat((char *)buf, "Received unexpected header or no header at all");
			break;
		case ERROR_ZCOMMAND_2:
			strcat((char *)buf, "Error sending ZCOMMAND sequence 2");
			break;
		case ERROR_ZFIN:
			strcat((char *)buf, "Did not receive ZFIN");
			break;
		case ERROR_RECV_ZACK:
			strcat((char *)buf, "Did not receive ZACK");
			break;
		case ERROR_RECV_SCP_ECG1:
			strcat((char *)buf, "Did not receive SCP-ECG 1");
			break;
		case ERROR_RECV_SCP_ECG2:
			strcat((char *)buf, "Did not receive SCP-ECG 2");
			break;
		case ERROR_RECV_DATA:
			strcat((char *)buf, "Did not receive data");
			break;
		case ERROR_ZFILE:
			strcat((char *)buf, "Error during ZFILE command");
			break;
		case ERROR_RECV_ZCAN:
			strcat((char *)buf, "Received ZCAN");
			break;
		default:
			strcat((char *)buf, "Unknown error");
			break;

	}


}

void CSpoofLPDlg::logFile(bool success, int error)
{
	CFile file;
	CFileException ex;
	BOOL bResult;

	if( !file.Open(".\\log.txt", CFile::modeWrite | CFile::modeCreate | CFile:: modeNoTruncate, &ex ) )
	{
		return;
	}

	/* we now go to the end of the file */
	bResult = FALSE;
	TRY
	{
		file.SeekToEnd();

		bResult = TRUE; // success
	}
	END_TRY

	if(!bResult)
	{
		return;
	}

	/*  formating the log file data here */
	char tem[1024];
	char size[512];
	CTime time = CTime::GetCurrentTime();
	sprintf(tem, "%02d-%02d-%04d %02d:%02d:%02d - ", time.GetDay(), time.GetMonth(), time.GetYear(), time.GetHour(), time.GetMinute(), time.GetSecond());
	if(success)
	{
		sprintf(size, "%d", m_Filesize);
		strcat(tem, size);
		strcat(tem, " bytes sent successfully\r\n");
	}
	else
	{
		createErrorMessage(error, size);
		strcat(tem, size);
		strcat(tem, "\r\n");
		// NOTE re-using size variable
		sprintf(size, "%d", ++fails);
		GetDlgItem(IDC_EDIT_FAIL)->SetWindowText(size);
	}

	/* write the data to log file */
	bResult = FALSE;
	TRY
	{
		file.Write(tem, strlen(tem));
		bResult = TRUE; // success
	}
	END_TRY

	if(!bResult)
	{
		return;
	}

	/*  make sure that we close the file when we finish the writing process */
	file.Flush();
	file.Close();
}


void CSpoofLPDlg::logFileMessage(char *message)
{
	CFile file;
	CFileException ex;
	BOOL bResult;

	if( !file.Open(".\\log.txt", CFile::modeWrite | CFile::modeCreate | CFile:: modeNoTruncate, &ex ) )
	{
		return;
	}

	/* we now go to the end of the file */
	bResult = FALSE;
	TRY
	{
		file.SeekToEnd();

		bResult = TRUE; // success
	}
	END_TRY

	if(!bResult)
	{
		return;
	}

	/*  formating the log file data here */
	char tem[1024];
//	char size[512];
	CTime time = CTime::GetCurrentTime();
	sprintf(tem, "%02d-%02d-%04d %02d:%02d:%02d - %s\r\n", time.GetDay(), time.GetMonth(), time.GetYear(), time.GetHour(), time.GetMinute(), time.GetSecond(), message);

	/* write the data to log file */
	bResult = FALSE;
	TRY
	{
		file.Write(tem, strlen(tem));
		bResult = TRUE; // success
	}
	END_TRY
SetLastError(0);
	if(!bResult)
	{
		return;
	}

	/*  make sure that we close the file when we finish the writing process */
	file.Flush();
	file.Close();
}

void CSpoofLPDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	CIniFile iniFile(".\\settings.ini", 1024);
	CString csTemp;
	int nTemp;

	GetDlgItem(IDC_EDIT_COM)->GetWindowText(csTemp);
	iniFile.WriteString("settings", "COM", csTemp);

	GetDlgItem(IDC_EDIT_DELAY)->GetWindowText(csTemp);
	iniFile.WriteString("settings", "DELAY", csTemp);

	GetDlgItem(IDC_EDIT_INIT_DELAY)->GetWindowText(csTemp);
	iniFile.WriteString("settings", "INITDELAY", csTemp);

	GetDlgItem(IDC_EDIT_OFF_DELAY)->GetWindowText(csTemp);
	iniFile.WriteString("settings", "OFFDELAY", csTemp);

	GetDlgItem(IDC_EDIT_ITERATIONS)->GetWindowText(csTemp);
	iniFile.WriteString("settings", "ITERATIONS", csTemp);

	if(m_chkBanner.GetCheck() == BST_CHECKED)
		iniFile.WriteString("settings", "BANNER", "YES");
	else
		iniFile.WriteString("settings", "BANNER", "NO");

	if(m_chkTimeStamp.GetCheck() == BST_CHECKED)
		iniFile.WriteString("settings", "TIMESTAMP", "YES");
	else
		iniFile.WriteString("settings", "TIMESTAMP", "NO");

	if(m_chkError.GetCheck() == BST_CHECKED)
		iniFile.WriteString("settings", "ERROR", "YES");
	else
		iniFile.WriteString("settings", "ERROR", "NO");

	nTemp = m_cmbError.GetCurSel() + 1;
	iniFile.WriteNumber("settings", "ERRORNO", nTemp);

	cleanUp();

	CDialog::OnClose();
}

void CSpoofLPDlg::cleanUp()
{
	if(port.m_bPortReady)
	{
		sendZCAN();
		port.clearDTR();
		port.ClosePort();
	}

	SetEvent(m_hCancelEvent);
	cancel = true;

//	GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BTN_OPEN)->EnableWindow(TRUE);

	SetLastError(ERROR_USER_CANCEL);
}

void CSpoofLPDlg::OnBtnCancel()
{
	cleanUp();
}


void CSpoofLPDlg::sendFileThread()
{
	AfxBeginThread(sendFile, this);
}



void CSpoofLPDlg::OnChkError()
{
	// TODO: Add your control notification handler code here
	setError();
}

void CSpoofLPDlg::setError()
{
	CString csTemp;

	m_cmbError.GetWindowText(csTemp);


	if(m_chkError.GetCheck() == BST_CHECKED)
		error = atoi(csTemp);		//m_cmbError.GetCurSel() - 1;
	else
		error = 0;
}

void CSpoofLPDlg::OnEditchangeCmbError()
{
	// TODO: Add your control notification handler code here
	// stupid me, this don't get triggered, its a list
}

void CSpoofLPDlg::resetDialog()
{
	GetDlgItem(IDC_BTN_OPEN)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_CANCEL)->EnableWindow(FALSE);
//	GetDlgItem(IDC_BTN_CLOSE)->EnableWindow(FALSE);
	GetDlgItem(IDC_LBL_STATUS)->SetWindowText("Idle");
	GetDlgItem(IDC_EDIT_COM)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_DELAY)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_INIT_DELAY)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_ITERATIONS)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_OFF_DELAY)->EnableWindow(TRUE);
}

void CSpoofLPDlg::OnSelchangeCmbError()
{
	// TODO: Add your control notification handler code here
	setError();
}

unsigned long CSpoofLPDlg::getBannerData()
{

	HANDLE hFile;
    DWORD  dwBytesRead = 0;
    //char   ReadBuffer[1024] = {0};


    hFile = CreateFile(".\\banner",               // file to open
                       GENERIC_READ,          // open for reading
                       FILE_SHARE_READ,       // share for reading
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL, // normal file
                       NULL);                 // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    // Read one character less than the buffer size to save room for
    // the terminating NULL character.

    if( FALSE == ReadFile(hFile, mainBuf, 1023, &dwBytesRead, NULL) )
    {
        CloseHandle(hFile);
        return false;
    }

    // This is the section of code that assumes the file is ANSI text.
    // Modify this block for other data types if needed.

    if (dwBytesRead == 0)
    {
        return false;
    }

    // It is always good practice to close the open file handles even though
    // the app will exit here and clean up open handles anyway.

    CloseHandle(hFile);


	if(timestamp)
	{
		char tem[128];
		CTime time = CTime::GetCurrentTime();
		sprintf(tem, "%04d%02d%02d:%02d%02d%02d", time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());
		CString csTemp, csReplace;
		int index, i, count;
		csTemp.Format("%s", mainBuf);
		csTemp.Replace("\n", "");	// just incase saved in notepad
		count = 0;
		for(i = 0; i < csTemp.GetLength(); i++)
		{
			if(csTemp.GetAt(i) == '\r')
				count++;
			if(count == 5)
			{
				i++;

				csReplace = csTemp.Mid(i);
				index = csReplace.Find("\r");
				csReplace = csReplace.Left(index);
				break;
			}
		}

		csTemp.Replace(csReplace, tem);
		csTemp.Replace("\n", "");

		memcpy(mainBuf, csTemp, csTemp.GetLength());
	}

	return dwBytesRead;
}

void CSpoofLPDlg::OnChkBanner()
{
	// TODO: Add your control notification handler code here
	if(m_chkBanner.GetCheck() == BST_CHECKED)
	{
		banner = true;
		m_chkTimeStamp.EnableWindow(TRUE);
	}
	else
	{
		banner = false;
		m_chkTimeStamp.EnableWindow(FALSE);
	}

}

void CSpoofLPDlg::OnChkTimeStamp()
{
	// TODO: Add your control notification handler code here
	if(m_chkTimeStamp.GetCheck() == BST_CHECKED)
		timestamp = true;
	else
		timestamp = false;
}

void CSpoofLPDlg::OnCheckHold()
{
	// TODO: Add your control notification handler code here
	UpdateData();
}
