// SpoofLPDlg.h : header file
//

#if !defined(AFX_SPOOFLPDLG_H__11B2461C_1516_424D_851C_0EF6B614C7D7__INCLUDED_)
#define AFX_SPOOFLPDLG_H__11B2461C_1516_424D_851C_0EF6B614C7D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SerialCom.h"

#define ZMODEMFILE_OK 0
#define ZMODEMFILE_NOMOREDATA 1
#define ZMODEMFILE_ERROR -1



/////////////////////////////////////////////////////////////////////////////
// CSpoofLPDlg dialog

class CSpoofLPDlg : public CDialog
{
// Construction
public:
	CSpoofLPDlg(CWnd* pParent = NULL);	// standard constructor

	CSerialCom  port;
	int ch;
	unsigned char frameType;
	unsigned char headerType;
	int headerData[4];
	int gotHeader;
	bool m_bWait;
	int gotSpecial;
	bool bcrc32,bcanfdx;
	CStringArray* m_Filelist;
	DWORD goodOffset;
	HANDLE m_hFile;
	CString m_Filename;
	unsigned long m_Filesize;
	int moreData;
	DWORD maxTx;
	DWORD bytes;
	int sendfile;
	int filesSent;
	bool cancel;
	int fails;
	HANDLE m_hCancelEvent;
	bool haveControl;		// do I have control of the bus, if not, receive a packet
	bool sendResponse;		// do I have to send the next data file
	bool sendOK;			// do I have to send an OK packet
	bool sendTime;
	bool recvResponse;		// do I expect to receive a response
	bool recvOK;			// do I expect to receive SCP-ECG S OK
	bool changeBaud;		// does the baud need to change
	bool baudChanged;
	bool exitCommands;		// go back and let ZFILE start the file transfer
	bool threadStarted;
	int error;
	int banner;
	int timestamp;
	char temp_buff[400];

	bool send_ch_sniffer;
    unsigned char sniffer_send_buff[2000];
    int sniffer_send_index;
    char error_buff[1024 * 3];
	bool receive_ch_sniffer;
    unsigned char sniffer_receive_buff[2000];
    int sniffer_receive_index;

	void hexdump(void *ptr, int buflen, bool isSend);
	void enable_receive_ch_sniffer(bool state);
	void enable_send_ch_sniffer(bool state);
    void put_ch_outbuff(unsigned char c);
    void put_ch_inbuff(unsigned char c);

	void sendrz();
	DWORD WriteBlock(void* buf,DWORD max);
	DWORD WriteBuffer(void *buffer, DWORD num);
	void sendHeader();
	void sendHexHeader();
	void sendZRQINIT();
	void sendChar();
	void sendHexChar();
	void sendBinHeader();
	void sendBin32Header();
	void sendDLEChar();
	void sendChar(int c);
	void getZMHeader();
	void getNextCh();
	void getBinaryHeader();
	void getBin32Header();
	void getHexHeader();
	void GetBlock(void *buffer,DWORD max,LPDWORD actual);
	void GetBlockImm(void *buffer,DWORD max,LPDWORD actual);
	void getNextDLECh();
	void getNextHexCh();
	DWORD ReadBuffer(void *buffer, DWORD num);
	bool SetupReadEvent(LPOVERLAPPED lpOverlappedRead,void* lpszInputBuffer,
					           DWORD dwSizeofBuffer,LPDWORD lpnNumberOfBytesRead);
	bool HandleReadEvent(LPOVERLAPPED lpOverlappedRead,void* lpszInputBuffer,
						         DWORD dwSizeofBuffer,LPDWORD lpnNumberOfBytesRead);
	bool sendFiles();
	void sendZACK();
	void sendZFIN();
	void sendOO();
	bool Open(CString filename,DWORD mode);
	void sendZFILE();
	void sendFILEINFO();
	void SetPos(DWORD offset);
	//UINT sendFile(LPVOID pParam);
	void Finish();
	int MakeFileInfo(unsigned char* buf);
	int GetData(void *buffer,DWORD max,LPDWORD bytes);
	void sendZDATA();
	void sendData();
	void sendData32(int len,char frameend);
	void sendData16(int len,char frameend);
	void getZMHeaderImmediate();
	void sendZEOF();
	bool sendCommandSeq1();
	void sendZCOMMAND();
	void sendCommand16(int len,char frameend);
	int GetCommandData(void *buffer,DWORD max,LPDWORD bytes);
	void sendZCOMMANDDATA();
	void receiveData();
	void getData16();
	void SetFlags();
	void changeBaudRate(int rate);
	bool sendCommandSeq2();
	void Pump();
	void logFile(bool success, int error);
	void logFileMessage(char *message);
	void createErrorMessage(int error, void *buf);
	void cleanUp();
	void sendZCAN();
	void sendFileThread();
	void sendPacket();
	void setError();
	void resetDialog();
	unsigned long getBannerData();
	CString GetSoftwareVersion();
	void Pump2(int time);


// Dialog Data
	//{{AFX_DATA(CSpoofLPDlg)
	enum { IDD = IDD_SPOOFLP_DIALOG };
	CButton	m_chkTimeStamp;
	CComboBox	m_cmbError;
	CButton	m_chkError;
	CButton	m_chkBanner;
	CProgressCtrl	m_Progress1;
	BOOL	m_bHold;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpoofLPDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSpoofLPDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnOpen();
	afx_msg void OnBtnClose();
	afx_msg void OnClose();
	afx_msg void OnBtnCancel();
	afx_msg void OnChkError();
	afx_msg void OnEditchangeCmbError();
	afx_msg void OnSelchangeCmbError();
	afx_msg void OnChkBanner();
	afx_msg void OnChkTimeStamp();
	afx_msg void OnCheckHold();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPOOFLPDLG_H__11B2461C_1516_424D_851C_0EF6B614C7D7__INCLUDED_)
