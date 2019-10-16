// Registry.cpp

#include "stdafx.h"
#include "Registry.h"

#include <atlbase.h>

CRegistry::CRegistry()
{
	m_hKey = NULL;
}

CRegistry::CRegistry(HKEY hKey, const char *lpszSubKey)
{
	Open(hKey, lpszSubKey);
}

CRegistry::~CRegistry()
{
	Close();
}

BOOL CRegistry::Open(HKEY hKey, const char *lpszSubKey)
{
	USES_CONVERSION;

	Close();

	strcpy(path, lpszSubKey);
	openhKey = hKey;

	int ret = RegOpenKeyEx(hKey, A2W(lpszSubKey), 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &m_hKey);

	if (ret != ERROR_SUCCESS)
	{
		return(FALSE);
	}

	return(TRUE);

}

BOOL CRegistry::Close(void)
{
	BOOL bRet = TRUE;

	if (m_hKey == NULL)
	{
		return(FALSE);
	}

	bRet = (::RegCloseKey(m_hKey) == ERROR_SUCCESS);
	m_hKey = NULL;

	return(bRet);

}

BOOL CRegistry::Read(const char *lpszValueName, LPVOID lpReturnBuffer)
{
	USES_CONVERSION;

	if (m_hKey == NULL)
	{
		return(FALSE);
	}

	DWORD dwSize = m_dwSize;

	memset(lpReturnBuffer, 0, dwSize);

	BOOL bRet = (::RegQueryValueEx(m_hKey, A2W(lpszValueName), NULL, NULL, (unsigned char *)lpReturnBuffer, &dwSize) == ERROR_SUCCESS);

	m_dwLastError = GetLastError();

	return(bRet);

}

BOOL CRegistry::ReadDWORD(const char *lpszValueName, DWORD *pdwData, DWORD *pdwLastError)
{

	if (m_hKey == NULL)
	{
		return(FALSE);
	}

	m_dwSize = sizeof(DWORD);

	BOOL bRet = Read(lpszValueName, pdwData);

	if (pdwLastError != NULL)
	{
		*pdwLastError = m_dwLastError;
	}

	return(bRet);

}

BOOL CRegistry::ReadString(const char *lpszValueName, LPVOID lpReturnBuffer, int nSize, DWORD *pdwLastError)
{

	if (m_hKey == NULL)
	{
		return(FALSE);
	}

	m_dwSize = nSize;

	BOOL bRet = Read(lpszValueName, lpReturnBuffer);

	if (pdwLastError != NULL)
	{
		*pdwLastError = m_dwLastError;
	}

	return(bRet);

}

BOOL CRegistry::WriteDWORD(const char *lpszValueName, DWORD dwData, DWORD *pdwLastError)
{

	if (m_hKey == NULL)
	{
		return(FALSE);
	}

	BOOL bRet = Write(lpszValueName, &dwData, REG_DWORD, sizeof(DWORD));

	if (pdwLastError != NULL)
	{
		*pdwLastError = m_dwLastError;
	}

	return(bRet);

}

BOOL CRegistry::WriteString(const char *lpszValueName, LPVOID lpData, DWORD *pdwLastError)
{

	if (m_hKey == NULL)
	{
		return(FALSE);
	}

	BOOL bRet = Write(lpszValueName, lpData, REG_SZ, (DWORD)strlen((const char *)lpData) + 1);

	if (pdwLastError != NULL)
	{
		*pdwLastError = m_dwLastError;
	}

	return(bRet);

}

BOOL CRegistry::Write(const char *lpszValueName, LPVOID lpData, DWORD dwType, int nSize)
{
	USES_CONVERSION;

	if (m_hKey == NULL)
	{
		return(FALSE);
	}

	DWORD dwSize = 500;
	BOOL bRet = (::RegSetValueEx(m_hKey, A2W(lpszValueName), 0, dwType, (unsigned char *)lpData, nSize) == ERROR_SUCCESS);

	m_dwLastError = GetLastError();

	return(bRet);

}

BOOL CRegistry::CreateKey(HKEY hKey, const char *lpszSubKey, const char *lpszClass)
{
	USES_CONVERSION;

	HKEY hOpenedKey;
	DWORD dwDisposition;
	DWORD dwLastError;

	BOOL bRet = (::RegCreateKeyEx(hKey, A2W(lpszSubKey), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hOpenedKey, &dwDisposition) == ERROR_SUCCESS);
	if (bRet) ::RegCloseKey(hOpenedKey);
	dwLastError = GetLastError();

	return(bRet);

}

BOOL CRegistry::DeleteKey(HKEY hKey, const char *lpszSubKey)
{
	USES_CONVERSION;

	BOOL bRet;
	DWORD dwLastError;

	bRet = (::RegDeleteKey(hKey, A2W(lpszSubKey)) == ERROR_SUCCESS);
	dwLastError = GetLastError();

	return(bRet);

}

BOOL CRegistry::QueryInfo()
{

	if (m_hKey == NULL)
	{
		return FALSE;
	}

	cSubKeys = 0;
	cchClassName = 255;

	int result = RegQueryInfoKey(
		m_hKey,                  // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time

	return result == ERROR_SUCCESS;
}

void CRegistry::createDateTimeString(char dst[])
{
	SYSTEMTIME st, local;
	FileTimeToSystemTime(&ftLastWriteTime, &st);
	SystemTimeToTzSpecificLocalTime(NULL, &st, &local);
	sprintf(dst, "%02d/%02d/%d  %02d:%02d", local.wMonth, local.wDay, local.wYear, local.wHour, local.wMinute);
}

BOOL CRegistry::EnumerateValues(FILE *fp)
{

	if (m_hKey == NULL)
	{
		return FALSE;
	}

	if (cValues > 0)
	{
		printf("Subkey:%s\n", path);
		if (fp != NULL)
		{
			static char header[] = "Subkey,Last Modified\r\n";
			fwrite(header, sizeof(char), strlen(header), fp);

			createDateTimeString(dt);
			sprintf(info, "%s,%s\r\n", path, dt);
			fwrite(info, sizeof(char), strlen(info), fp);
		}
	}

	if (cValues > 0)
	{
		static char header2[] = "Name,Value\r\n";
		fwrite(header2, sizeof(char), strlen(header2), fp);
	}

	for (int i = 0; i < (int)cValues; i++)
	{
		achValue[0] = 0;
		cchValue = 500;
		RegEnumValue(m_hKey, i,
			achValue,
			&cchValue,
			NULL,
			NULL,
			NULL,
			NULL);

		DWORD dwSize;
		char returnBuffer[500];
		RegQueryValueEx(m_hKey, achValue, NULL, NULL, (unsigned char *)returnBuffer, &dwSize);

		printf("   Name:%s Value:%s\n", achValue, returnBuffer);

		if (fp != NULL)
		{
			sprintf(info, "%s,%s\r\n", achValue, returnBuffer);
			fwrite(info, sizeof(char), strlen(info), fp);
		}
	}

	return TRUE;
}

void CRegistry::DisplayFileTime(FILE *fp)
{
	if (m_hKey == NULL || cValues <= 0)
	{
		return;
	}

	createDateTimeString(dt);
	printf("%s\n", dt);
}

BOOL CRegistry::EnumerateSubkeys(FILE *fp)
{
	USES_CONVERSION;

	if (m_hKey == NULL || cSubKeys <= 0)
	{
		return FALSE;
	}

	if (fp != NULL)
	{
		static char header[] = "Type,Vendor,Product,Revision,Serial,Time Last Written\r\n";
		fwrite(header, sizeof(char), strlen(header), fp);
	}

	for (int i = 0; i<(int)cSubKeys; i++)
	{
		DWORD cbName;
		cbName = 500;
		RegEnumKeyEx(m_hKey, i,
			achKey,
			&cbName,
			NULL,
			NULL,
			NULL,
			&ftLastWriteTime);

		char fullPath[200];
		sprintf(fullPath, "%s\\%s", path, achKey);

		showUSBDevices(openhKey, fullPath, W2A(achKey), fp);
	}

	return TRUE;
}

void crackSubkeyToParts(char *subkey, char *type, char *vendor, char *product, char *revision)
{
	type[0] = vendor[0] = product[0] = revision[0] = 0;

	char *tmp = strstr(subkey, "&");
	if (tmp == NULL)
	{
		return;
	}
	*tmp = 0;
	strcpy(type, subkey);

	tmp++;
	char *start = tmp;
	tmp = strstr(start, "&");
	if (tmp == NULL)
	{
		return;
	}
	*tmp = 0;
	start += 4;
	strcpy(vendor, start);

	tmp++;
	start = tmp;
	tmp = strstr(start, "&");
	if (tmp == NULL)
	{
		return;
	}
	*tmp = 0;
	start += 5;
	strcpy(product, start);

	tmp++;
	start = tmp;
	start += 4;
	strcpy(revision, start);
}

void CRegistry::showUSBDevices(HKEY hk, char *path, char *subkey, FILE *fp)
{
	CRegistry reg;
	if (!reg.Open(hk, (const char *)path))
	{
		return;
	}

	reg.QueryInfo();
	if (reg.cSubKeys > 0)
	{
		for (int i = 0; i < (int)reg.cSubKeys; i++)
		{
			DWORD cbName;
			cbName = 500;
			RegEnumKeyEx(reg.m_hKey, i,
				reg.achKey,
				&cbName,
				NULL,
				NULL,
				NULL,
				&reg.ftLastWriteTime);

			char saveMe[200];
			strcpy(saveMe, subkey);

			reg.createDateTimeString(reg.dt);
			char type[50], vendor[50], product[50], revision[50];
			crackSubkeyToParts(subkey, type, vendor, product, revision);
			printf("Type:%s,Vendor:%s,Product:%s,Revision:%s,Serial:%s,Last:%s\n", type, vendor, product, revision, reg.achKey, reg.dt);

			strcpy(subkey, saveMe);

			if (fp != NULL)
			{
				sprintf(reg.info, "%s,%s,%s,%s,%s,%s\r\n", type, vendor, product, revision, reg.achKey, reg.dt);
				fwrite(reg.info, sizeof(char), strlen(reg.info), fp);
			}
		}
	}

	reg.Close();
}

void CRegistry::EnumerateBinaryData(FILE *fp)
{
	USES_CONVERSION;

	if (m_hKey == NULL || cValues <= 0)
	{
		return;
	}

	for (int i = 0; i < (int)cValues; i++)
	{
		achValue[0] = 0;
		cchValue = 500;
		RegEnumValue(m_hKey, i,
			achValue,
			&cchValue,
			NULL,
			NULL,
			NULL,
			NULL);

		if (_stricmp(W2A(achValue), "MRUListEx") == 0)
		{
			continue;
		}

		DWORD dwSize;
		char returnBuffer[500];
		RegQueryValueEx(m_hKey, achValue, NULL, NULL, (unsigned char *)returnBuffer, &dwSize);

		char filename[200];
		memset(filename, 0, sizeof(filename));

		int j = 0;
		short *tmp = (short *)returnBuffer;
		while (*tmp != 0)
		{
			filename[j] = (char)*tmp;
			j++;
			tmp++;
		}

		printf("  %s\n", filename);

		if (fp != NULL)
		{
			fwrite(filename, sizeof(char), strlen(filename), fp);
			fwrite("\r\n", sizeof(char), 2, fp);
		}
	}
}

void CRegistry::GetNets(FILE *fp)
{

	if (m_hKey == NULL || cSubKeys <= 0)
	{
		return;
	}

	for (int i = 0; i < (int)cSubKeys; i++)
	{
		DWORD cbName;
		cbName = 500;
		RegEnumKeyEx(m_hKey, i,
			achKey,
			&cbName,
			NULL,
			NULL,
			NULL,
			&ftLastWriteTime);

		char fullPath[200];
		sprintf(fullPath, "%s\\%s", path, achKey);

		CRegistry reg;
		if (reg.Open(openhKey, fullPath))
		{
			char retBuffer[100];
			reg.ReadString("ProfileName", retBuffer, sizeof(retBuffer));
			reg.Close();

			printf("Network:%s\r\n", retBuffer);

			if (fp != NULL)
			{
				fwrite(retBuffer, sizeof(char), strlen(retBuffer), fp);
				fwrite("\r\n", sizeof(char), 2, fp);
			}
		}
	}

}

