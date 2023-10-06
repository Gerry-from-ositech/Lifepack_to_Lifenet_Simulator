//-----------------------------------------------------------------------------
// project:		ZModem
// author:		Frank Weiler, Genshagen, Germany
// version:		0.91
// date:		October 11, 2000
// email:		frank@weilersplace.de
// copyright:	This Software is OpenSource.
// file:		ZModemCore.h
//-----------------------------------------------------------------------------

#if !defined(AFX_ZMODEMCORE_H__6A43214A_9C2E_11D4_8639_F6B82A27C85A__INCLUDED_)
#define AFX_ZMODEMCORE_H__6A43214A_9C2E_11D4_8639_F6B82A27C85A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "zmodemdef.h"
#include "crcxm.h"
#include "crc32.h"

// macros
#define ALLOK (GetLastError()==NO_ERROR)
#define needsDLE(x) (((x) == ZDLE) || ((x) == 0x10) || ((x) == 0x11) || ((x) == 0x13)) //masking special ZModem chars

// states of the zmodem state machine
#define SM_SENDZDATA		0	//sending / receiving file information
#define SM_SENDZEOF			1	//finishing file
#define SM_SENDDATA			2	//data transfer is running
#define SM_ACTIONRPOS		3	//reposition the filepointer during sending or receiving
#define SM_WAITZRINIT		4	//waiting for initalisation finish
#define SM_SENDZCOMMAND		5	//sending command header
#define SM_SENDCOMMAND		6	//sending command header data
#define SM_SENDZCOMMANDDATA	7	//sending command data header
#define SM_SENDCOMMANDDATA	8	//sending command data
#define SM_RECVZACK			10	//receive a ZACK packet
#define SM_RECVOK			11	//receive SCP-ECG OK packet
#define SM_RECVRESP			12	//receive a response, i.e. DST time sync
#define SM_RECVDATA			13	//receive data, Titan is in control of the bus

// constants
#define ZMCORE_MAXTX 1024	//sending/receiving max. 1024 bytes over com-port in one step

// errors used by this implementation
#define ZMODEM_INIT					0x20000001	//zmodem-initialization failed
#define ZMODEM_POS					0x20000002	//force reposition
#define ZMODEM_ZDATA				0x20000003	//
#define ZMODEM_CRCXM				0x20000004	//16-bit-checksum error
#define ZMODEM_LONGSP				0x20000005	//too long subpaket recieved
#define ZMODEM_CRC32				0x20000006	//32-bit-checksum error
#define ZMODEM_FILEDATA				0x20000007	//filedata has errors or missing
#define ZMODEM_BADHEX				0x20000008	//unexpected hex-char received (in a hex header)
#define ZMODEM_TIMEOUT				0x20000009	//by name
#define ZMODEM_GOTZCAN				0x2000000A	//cancel recieved (form other side)
#define ZMODEM_ABORTFROMOUTSIDE		0x2000000B	//user break
#define ZMODEM_ERROR_FILE			0x2000000C	//file handling error (during open, create, read, write)
#define ZMODEM_ZCOMMAND				0x2000000D	// SendCommandSeqx returned false

#define ERROR_OPEN_PORT				0x2000000E
#define ERROR_ZMODEM_INIT			0x2000000F
#define ERROR_ZCOMMAND_1			0x20000010
#define ERROR_ZSEND_FILES			0x20000011
#define ERROR_ZMODEM_HEADER			0x20000012
#define ERROR_ZCOMMAND_2			0x20000013
#define ERROR_ZFIN					0x20000014
#define ERROR_RECV_ZACK				0x20000015
#define ERROR_RECV_SCP_ECG1			0x20000016
#define ERROR_RECV_SCP_ECG2			0x20000017
#define ERROR_RECV_DATA				0x20000018
#define ERROR_ZFILE					0x20000019
#define ERROR_RECV_ZCAN				0x2000001A
#define ERROR_USER_CANCEL			0x2000001B
#define ERROR_CAUSE_ERROR			0x2000001C
#define ERROR_RECV_ZABORT			0x2000001D


#endif // !defined(AFX_ZMODEMCORE_H__6A43214A_9C2E_11D4_8639_F6B82A27C85A__INCLUDED_)
