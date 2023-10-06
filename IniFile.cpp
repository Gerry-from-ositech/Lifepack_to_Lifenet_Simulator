/////////////////////////////////////////////
//Titan II Utility - Functions and commands//
/////////////////////////////////////////////

#include "stdafx.h"
//#include "IniFileClass.h"
#include "IniFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CIniFile::CIniFile(LPCTSTR	strIniFileName,INT iMaxStringLength): m_MAXSTRLEN(iMaxStringLength){ SetIniFileName(strIniFileName); }
CIniFile::~CIniFile(){ }
CString	CIniFile::GetIniFileName(){ return m_strFileName ; }
void CIniFile::SetIniFileName(LPCTSTR lpIniFileName){ m_strFileName = lpIniFileName; }

BOOL CIniFile::GetStatus(CFileStatus& rStatus){
	BOOL bRetVal = FALSE;
	bRetVal = CFile::GetStatus(m_strFileName, rStatus);
	return bRetVal;
}

BOOL CIniFile::WriteSection(LPCTSTR lpSection,LPCTSTR lpData){
	BOOL bRetVal = FALSE;
	bRetVal = ::WritePrivateProfileSection(lpSection, lpData, m_strFileName);
	memset(&lpData, 0, sizeof(lpData));
	return bRetVal;
}

BOOL CIniFile::WriteString(LPCTSTR lpSection,LPCTSTR lpKey,LPCTSTR lpString){
	ASSERT(lpString == NULL || strlen(lpString) < m_MAXSTRLEN);
	return ::WritePrivateProfileString(lpSection, lpKey, lpString, m_strFileName);
}

BOOL CIniFile::WriteNumber(LPCTSTR lpSection,LPCTSTR lpKey,INT iValue){
	CString	str;
	str.Format("%d", iValue);
	return WriteString(lpSection, lpKey, str);
}

BOOL CIniFile::WriteStruct(LPCTSTR lpSection,LPCTSTR lpKey,LPVOID lpStruct,UINT iSizeStruct){
	BOOL bRetVal = FALSE;
	bRetVal = ::WritePrivateProfileStruct(lpSection, lpKey, lpStruct, iSizeStruct, m_strFileName);
	return bRetVal;
}

BOOL CIniFile::WriteNumber(LPCTSTR	lpSection,LPCTSTR lpKey,FLOAT fValue){
	CString	str;
	str.Format("%f", fValue);
	return WriteString(lpSection, lpKey, str);
}

BOOL CIniFile::RemoveKey(LPCTSTR lpSection,LPCTSTR lpKey){ return WriteString(lpSection, lpKey, NULL); }

UINT CIniFile::GetInt(LPCTSTR lpSection,LPCTSTR lpKey,const INT iDefaultValue){
	UINT iRet = ::GetPrivateProfileInt(lpSection, lpKey, iDefaultValue, m_strFileName);
	return iRet;
}

BOOL CIniFile::GetString(LPCTSTR lpSection,LPCTSTR lpKey,CString& strRet,LPCTSTR lpDefault){
	ASSERT(lpDefault != NULL);
	ASSERT(strlen(lpDefault) < m_MAXSTRLEN);
	return CIniFile::GetString(lpSection, lpKey, strRet, lpDefault, m_MAXSTRLEN);
}

BOOL CIniFile::GetStringKeyNum(LPCTSTR lpSection,int lpKey,CString& strRet,LPCTSTR lpDefault){
	ASSERT(lpDefault != NULL);
	ASSERT(strlen(lpDefault) < m_MAXSTRLEN);
	CString str;
	str.Format("%d",lpKey);
	return CIniFile::GetString(lpSection, str, strRet, lpDefault, m_MAXSTRLEN);
}

BOOL CIniFile::GetString(LPCTSTR lpSection,LPCTSTR lpKey,CString& strRet,LPCTSTR lpDefault,const DWORD iSize){	
	LPTSTR pBuffer;
	TRY{ pBuffer = strRet.GetBuffer(iSize); }
	CATCH(CMemoryException, pExc){ THROW(pExc); return FALSE; }
	END_CATCH
	memcpy(pBuffer + iSize - 2, "xx",2);
	DWORD iRet = ::GetPrivateProfileString(lpSection, lpKey, lpDefault, pBuffer, iSize, m_strFileName);
	ASSERT(iRet < iSize);
	BOOL bRet = (memcmp(pBuffer + iSize - 2, "\0\0", 2) != 0);	// check the last 2 characters of the buffer.
	strRet.ReleaseBuffer();	
	return bRet;
}

FLOAT CIniFile::GetFloat(LPCTSTR lpSection,LPCTSTR lpKey,const FLOAT fDefaultValue){
	const int MAXFLOATDIGS = 512;
	CString strRet, strDefault;
	strDefault.Format("%f", fDefaultValue);
	BOOL bRet = CIniFile::GetString(lpSection, lpKey, strRet, strDefault, MAXFLOATDIGS);
	return ( bRet ? atof(strRet) : fDefaultValue );
}

BOOL CIniFile::GetStruct(LPCTSTR lpSection,LPCTSTR lpKey,LPVOID	lpRetStruct,const UINT iSizeStruct){
	ASSERT(lpRetStruct != NULL);
	ASSERT(iSizeStruct > 0);
	return ::GetPrivateProfileStruct(lpSection, lpKey, lpRetStruct, iSizeStruct, m_strFileName);
}

void CIniFile::GetSectionNames(CStringList& lstSectionNames){
	LPTSTR lpRetBuff = new CHAR[m_MAXSTRLEN];
	DWORD iRetVal = ::GetPrivateProfileSectionNames(lpRetBuff, m_MAXSTRLEN, m_strFileName);
	for (LPCTSTR p = lpRetBuff; *p != '\0'; p += strlen(p) + 1){ lstSectionNames.AddTail(p); }	
	delete [] lpRetBuff;
}

int CIniFile::GetSectionNamesAmnt(){
	int counter=0;
	LPTSTR lpRetBuff = new CHAR[m_MAXSTRLEN];
	DWORD iRetVal = ::GetPrivateProfileSectionNames(lpRetBuff, m_MAXSTRLEN, m_strFileName);
	for (LPCTSTR p = lpRetBuff; *p != '\0'; p += strlen(p) + 1){ counter++; }	
	delete [] lpRetBuff;
	return counter;
}
