// Registry.h

#ifndef __REGISTRY_H__
#define __REGISTRY_H__

#include <windows.h>

class CRegistry
{

public:
	CRegistry();
	CRegistry(HKEY, const char *);
	~CRegistry();

	BOOL Open(HKEY, const char *);
	BOOL Close(void);

	BOOL ReadDWORD(const char *, DWORD *, DWORD *pdwLastError = NULL);
	BOOL ReadString(const char *, LPVOID, int, DWORD *pdwLastError = NULL);

	BOOL WriteDWORD(const char *, DWORD, DWORD *pdwLastError = NULL);
	BOOL WriteString(const char *, LPVOID, DWORD *pdwLastError = NULL);
	BOOL Write(const char *, LPVOID, DWORD, int);

	static BOOL CreateKey(HKEY, const char *, const char *);
	static BOOL DeleteKey(HKEY, const char *);

	BOOL QueryInfo();
	BOOL EnumerateValues(FILE *fp);
	BOOL EnumerateSubkeys(FILE *fp);
	void DisplayFileTime(FILE *fp);
	void EnumerateBinaryData(FILE *fp);
	void GetNets(FILE *fp);

	DWORD cSubKeys;
	WCHAR achKey[255];
	HKEY m_hKey;
	FILETIME ftLastWriteTime;

	char dt[800];
	char info[800];

protected:
	HKEY openhKey;
	char path[500];
	DWORD m_dwLastError;

	BOOL Read(const char *, LPVOID);
	void createDateTimeString(char dst[]);

	DWORD m_dwSize;

	WCHAR achClass[255];
	DWORD cchClassName;
	DWORD cbMaxSubKey, cchMaxClass, cValues, cchMaxValue, cbMaxValueData, cbSecurityDescriptor;
	WCHAR achValue[500];
	DWORD cchValue;

	void showUSBDevices(HKEY hk, char *path, char *subkey, FILE *fp);

};

#endif
