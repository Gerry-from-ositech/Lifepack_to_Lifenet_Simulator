///////////////////////////////////////////
//Titan II Utility - Command Declarations//
///////////////////////////////////////////

#if !defined(AFX_INIFILE_H__59955693_F4CE_4E47_9D52_EEA1B6E8F706__INCLUDED_)
#define AFX_INIFILE_H__59955693_F4CE_4E47_9D52_EEA1B6E8F706__INCLUDED_
#if _MSC_VER > 1000
#pragma once
#endif

//Declare the IniFile class, and its attributes
class CIniFile : public CObject
{
public:
	CIniFile(LPCTSTR lpIniFileName, INT iMaxStringLength);
	virtual ~CIniFile();

// Attributes
protected:
	CString		m_strFileName;
	const INT	m_MAXSTRLEN;

// Implementation	
public:
	CString	GetIniFileName();
	void	SetIniFileName(LPCTSTR lpIniFileName);
	BOOL	GetStatus(CFileStatus& rStatus);
	BOOL	GetString(LPCTSTR lpSection, LPCTSTR lpKey, CString& strRet, LPCTSTR strDefault);
	BOOL	GetStringKeyNum(LPCTSTR lpSection, int lpKey, CString& strRet, LPCTSTR strDefault);
	UINT	GetInt(LPCTSTR lpSection, LPCTSTR lpKey, INT iDefaultValue);	
	FLOAT	GetFloat(LPCTSTR lpSection, LPCTSTR lpKey, FLOAT fDefaultValue);
	BOOL	GetStruct(LPCTSTR lpSection, LPCTSTR lpKey, LPVOID lpRetStruct, UINT iSizeStruct);
	void	GetSectionNames(CStringList& lstSectionNames);
	int		GetSectionNamesAmnt();
	BOOL	WriteSection(LPCTSTR lpSection, LPCTSTR lpData); 
	BOOL	WriteString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpString);
	BOOL	WriteNumber(LPCTSTR lpSection, LPCTSTR lpKey, INT iValue);
	BOOL	WriteNumber(LPCTSTR lpSection, LPCTSTR lpKey, FLOAT fValue);
	BOOL	WriteStruct(LPCTSTR lpSection, LPCTSTR lpKey, LPVOID lpStruct, UINT iSizeStruct);
	BOOL	RemoveKey(LPCTSTR lpSection, LPCTSTR lpKey);
protected:
	BOOL	GetString(LPCTSTR lpSection, LPCTSTR lpKey, CString& strRet, LPCTSTR strDefault, DWORD iSize);
};

#endif
