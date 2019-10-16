//////////////////////////////////////////////////////////////////////
//
//  CNtRegistry.cpp: Implementation of the CNtRegistry class.
//
//////////////////////////////////////////////////////////////////////
//
// File           : NtRegistry.cpp
// Version        : Found in the header file
// Function       : Source file of the NT Native Registry API classes.
// Copyright      : (c) Daniel Madden, Sr.
//
// Author         : Daniel Madden Sr.
// Date           : Jun 3, 2004
//
// Notes          : Ideas for this class comes from RegHide, a program 
//					written by Mark Russinovich.  See below link 
//					(http://www.sysinternals.com)
//
//					The Class below was created by combining the 
//					Native Registry APIs (found in the book by
//					Gary Nebbett - see below), the reghide program
//					ideas found at SysInternals web site and the
//					"Look & Feel" of the nice CRegistry class 
//					by Robert Pittenger, which can be found below:  
//
//					http://www.codeproject.com/system/registry.asp.
//
//					A good book to read: 
// 
//					Windows NT/2000 Native API Reference
//					ISBN:	1-57870-199-6
//					Author:	Gary Nebbett
//
//////////////////////////////////////////////////////////////////////
//
// Copyright © 2004-2006 Daniel Madden
//
// This grants you ("Licensee") a non-exclusive, royalty free, 
// licence to use, modify and redistribute this software in source and binary 
// code form, provided that i) this copyright notice and licence appear on all 
// copies of the software; and ii) Licensee does not utilize the software in a 
// manner which is disparaging to Daniel Madden, Sr..
//
// This software is provided "AS IS," without a warranty of any kind. ALL
// EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING 
// ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
// OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED. JETBYTE LIMITED AND ITS LICENSORS 
// SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF 
// USING, MODIFYING OR DISTRIBUTING THE SOFTWARE OR ITS DERIVATIVES. IN NO 
// EVENT WILL JETBYTE LIMITED BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, 
// OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE 
// DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY, ARISING 
// OUT OF THE USE OF OR INABILITY TO USE SOFTWARE, EVEN IF JETBYTE LIMITED 
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
//////////////////////////////////////////////////////////////////////

#include "Stdafx.h"
#include "NtRegistry.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CNtRegistry::CNtRegistry()
{
	InitNtRegistry();
}

CNtRegistry::~CNtRegistry()
{
	if (m_hMachineReg) {
		NtClose(m_hMachineReg);
	}
}

//////////////////////////////////////////////////////////////////////
//
//	Function:	InitNtRegistry
//
//  Purpose:	Initialize class variables
//
//  Arguments:	None
//
//  Returns:	None
//				
//////////////////////////////////////////////////////////////////////
void CNtRegistry::InitNtRegistry()
{
	CString csErr;

	////////////////////////////////////////////
	// First off, let's make it possible ONLY 
	// for Windows 2000 or greater
	try
	{
		if (!IsW2KorBetter()) {
			throw "MUST be Windows 2000 or greater!";
		}
	}
	catch (char* str)
	{
		csErr.Format(_T("CNtRegistry Exception raised: %s"), str);
		return;
	}
	//
	////////////////////////////////////////////


	//
	if (!LocateNTDLLEntryPoints(csErr)) {
		MessageBox(NULL, csErr, _T("CNtRegistry::ClearKeys()"), MB_ICONERROR);
	}

	// UserMode = 0
	m_ntModeType = 0;

	m_bHidden = FALSE;
	m_bLazyWrite = TRUE;
	m_NtStatus = STATUS_SUCCESS;

	// HKCU is default, so get the Users SID and make it default.
	if (LookupSID(m_csSID)) {
		m_csRootPath.Format(_T("\\Registry\\User\\%s"), m_csSID);
	}
	// If you can't get the Users SID for some reason,
	// then default to HKLM
	else {
		m_csRootPath = _T("\\Registry\\Machine");
	}

	// Set it to the "Root-Path" from above
	m_csCurrentPath = m_csRootPath;

	//
	m_hMachineReg = 0x00000000;
	m_csMachineName = _T("");

	//
	ANSI_STRING asKey;
	RtlZeroMemory(&asKey, sizeof(asKey));
	RtlInitAnsiString(&asKey, (LPCSTR)(LPCTSTR)m_csCurrentPath);

	RtlZeroMemory(&m_usKeyName, sizeof(m_usKeyName));

	RtlAnsiStringToUnicodeString(&m_usKeyName, &asKey, TRUE);
	m_usLength = m_usKeyName.Length;
}

//////////////////////////////////////////////////////////////////////
//
//	Function:	LocateNTDLLEntryPoints
//
//  Purpose:	Loads and finds the entry points we need in NTDLL.DLL 
//
//  Arguments:	IN CString csErr - Reason it failed (if returned FALSE).
//
//  Returns:	BOOL - Success/Failure.
//
//////////////////////////////////////////////////////////////////////
BOOL CNtRegistry::LocateNTDLLEntryPoints(CString& csErr)
{
	csErr = _T("");

	HINSTANCE hinstStub = GetModuleHandle(_T("ntdll.dll"));
	if (hinstStub) {
		//
		NtOpenThread = (LPNTOPENTHREAD)GetProcAddress(hinstStub, "NtOpenThread");
		if (!NtOpenThread) {
			csErr = _T("Could not find NtOpenThread entry point in NTDLL.DLL");
			return FALSE;
		}
		NtCreateKey = (LPNTCREATEKEY)GetProcAddress(hinstStub, "NtCreateKey");
		if (!NtCreateKey) {
			csErr = _T("Could not find NtCreateKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtOpenKey = (LPNTOPENKEY)GetProcAddress(hinstStub, "NtOpenKey");
		if (!NtOpenKey) {
			csErr = _T("Could not find NtOpenKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtFlushKey = (LPNTFLUSHKEY)GetProcAddress(hinstStub, "NtFlushKey");
		if (!NtFlushKey) {
			csErr = _T("Could not find NtFlushKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtDeleteKey = (LPNTDELETEKEY)GetProcAddress(hinstStub, "NtDeleteKey");
		if (!NtDeleteKey) {
			csErr = _T("Could not find NtDeleteKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtQueryKey = (LPNTQUERYKEY)GetProcAddress(hinstStub, "NtQueryKey");
		if (!NtQueryKey) {
			csErr = _T("Could not find NtQueryKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtEnumerateKey = (LPNTENUMERATEKEY)GetProcAddress(hinstStub, "NtEnumerateKey");
		if (!NtEnumerateKey) {
			csErr = _T("Could not find NtEnumerateKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtClose = (LPNTCLOSE)GetProcAddress(hinstStub, "NtClose");
		if (!NtClose) {
			csErr = _T("Could not find NtClose entry point in NTDLL.DLL");
			return FALSE;
		}
		NtSetValueKey = (LPNTSETVALUEKEY)GetProcAddress(hinstStub, "NtSetValueKey");
		if (!NtSetValueKey)
		{
			csErr = _T("Could not find NTSetValueKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtSetInformationKey = (LPNTSETINFORMATIONKEY)GetProcAddress(hinstStub, "NtSetInformationKey");
		if (!NtSetInformationKey)
		{
			csErr = _T("Could not find NtSetInformationKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtQueryValueKey = (LPNTQUERYVALUEKEY)GetProcAddress(hinstStub, "NtQueryValueKey");
		if (!NtQueryValueKey)
		{
			csErr = _T("Could not find NtQueryValueKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtEnumerateValueKey = (LPNTENUMERATEVALUEKEY)GetProcAddress(hinstStub, "NtEnumerateValueKey");
		if (!NtEnumerateValueKey)
		{
			csErr = _T("Could not find NtEnumerateValueKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtDeleteValueKey = (LPNTDELETEVALUEKEY)GetProcAddress(hinstStub, "NtDeleteValueKey");
		if (!NtDeleteValueKey)
		{
			csErr = _T("Could not find NtDeleteValueKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtRenameKey = (LPNTRENAMEKEY)GetProcAddress(hinstStub, "NtRenameKey");
		if (!NtDeleteValueKey)
		{
			csErr = _T("Could not find NtRenameKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtQueryMultipleValueKey = (LPNTQUERYMULTIPLEVALUEKEY)GetProcAddress(hinstStub, "NtQueryMultipleValueKey");
		if (!NtQueryMultipleValueKey)
		{
			csErr = _T("Could not find NtQueryMultipleValueKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtNotifyChangeKey = (LPNTNOTIFYCHANGEKEY)GetProcAddress(hinstStub, "NtNotifyChangeKey");
		if (!NtNotifyChangeKey)
		{
			csErr = _T("Could not find NtNotifyChangeKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtCreateFile = (LPNTCREATEFILE)GetProcAddress(hinstStub, "NtCreateFile");
		if (!NtCreateFile)
		{
			csErr = _T("Could not find NtCreateFile entry point in NTDLL.DLL");
			return FALSE;
		}
		NtOpenProcessToken = (LPNTOPENPROCESSTOKEN)GetProcAddress(hinstStub, "NtOpenProcessToken");
		if (!NtOpenProcessToken)
		{
			csErr = _T("Could not find NtOpenProcessToken entry point in NTDLL.DLL");
			return FALSE;
		}
		NtAdjustPrivilegesToken = (LPNTADJUSTPRIVILEGESTOKEN)GetProcAddress(hinstStub, "NtAdjustPrivilegesToken");
		if (!NtAdjustPrivilegesToken)
		{
			csErr = _T("Could not find NtAdjustPrivilegesToken entry point in NTDLL.DLL");
			return FALSE;
		}
		NtQueryInformationToken = (LPNTQUERYINFORMATIONTOKEN)GetProcAddress(hinstStub, "NtQueryInformationToken");
		if (!NtQueryInformationToken)
		{
			csErr = _T("Could not find NtQueryInformationToken entry point in NTDLL.DLL");
			return FALSE;
		}
		RtlAllocateHeap = (LPRTLALLOCATEHEAP)GetProcAddress(hinstStub, "RtlAllocateHeap");
		if (!RtlAllocateHeap)
		{
			csErr = _T("Could not find RtlAllocateHeap entry point in NTDLL.DLL");
			return FALSE;
		}
		RtlFreeHeap = (LPRTLFREEHEAP)GetProcAddress(hinstStub, "RtlFreeHeap");
		if (!RtlFreeHeap)
		{
			csErr = _T("Could not find RtlFreeHeap entry point in NTDLL.DLL");
			return FALSE;
		}
		NtRestoreKey = (LPNTRESTOREKEY)GetProcAddress(hinstStub, "NtRestoreKey");
		if (!NtRestoreKey)
		{
			csErr = _T("Could not find NtRestoreKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtSaveKey = (LPNTSAVEKEY)GetProcAddress(hinstStub, "NtSaveKey");
		if (!NtSaveKey)
		{
			csErr = _T("Could not find NtSaveKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtLoadKey = (LPNTLOADKEY)GetProcAddress(hinstStub, "NtLoadKey");
		if (!NtLoadKey)
		{
			csErr = _T("Could not find NtLoadKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtLoadKey2 = (LPNTLOADKEY2)GetProcAddress(hinstStub, "NtLoadKey2");
		if (!NtLoadKey2)
		{
			csErr = _T("Could not find NtLoadKey2 entry point in NTDLL.DLL");
			return FALSE;
		}
		NtReplaceKey = (LPNTREPLACEKEY)GetProcAddress(hinstStub, "NtReplaceKey");
		if (!NtReplaceKey)
		{
			csErr = _T("Could not find NtReplaceKey entry point in NTDLL.DLL");
			return FALSE;
		}
		NtUnloadKey = (LPNTUNLOADKEY)GetProcAddress(hinstStub, "NtUnloadKey");
		if (!NtUnloadKey)
		{
			csErr = _T("Could not find NtUnloadKey entry point in NTDLL.DLL");
			return FALSE;
		}

		RtlInitString = (LPRTLINITSTRING)GetProcAddress(hinstStub, "RtlInitString");
		RtlInitAnsiString = (LPRTLINITANSISTRING)GetProcAddress(hinstStub, "RtlInitAnsiString");
		RtlInitUnicodeString = (LPRTLINITUNICODESTRING)GetProcAddress(hinstStub, "RtlInitUnicodeString");
		RtlAnsiStringToUnicodeString = (LPRTLANSISTRINGTOUNICODESTRING)GetProcAddress(hinstStub, "RtlAnsiStringToUnicodeString");
		RtlUnicodeStringToAnsiString = (LPRTLUNICODESTRINGTOANSISTRING)GetProcAddress(hinstStub, "RtlUnicodeStringToAnsiString");
		RtlFreeString = (LPRTLFREESTRING)GetProcAddress(hinstStub, "RtlFreeString");
		RtlFreeAnsiString = (LPRTLFREEANSISTRING)GetProcAddress(hinstStub, "RtlFreeAnsiString");
		RtlFreeUnicodeString = (LPRTLFREEUNICODESTRING)GetProcAddress(hinstStub, "RtlFreeUnicodeString");
		if (RtlInitString && RtlInitAnsiString && RtlInitUnicodeString &&
			RtlAnsiStringToUnicodeString && RtlUnicodeStringToAnsiString &&
			RtlFreeString && RtlFreeAnsiString && RtlFreeUnicodeString)
		{
			return FALSE;
		}

		//_NtOpenSection = (LPNTOPENSECTION)GetProcAddress(hinstStub, "NtOpenSection");
		//_NtMapViewOfSection = (LPNTMAPVIEWOFSECTION)GetProcAddress(hinstStub, "NtMapViewOfSection");
		//_NtUnmapViewOfSection = (LPNTUNMAPVIEWOFSECTION)GetProcAddress(hinstStub, "NtUnmapViewOfSection");
		//_NtQuerySystemInformation = (LPNTQUERYSYSTEMINFORMATION)GetProcAddress(hinstStub, "ZwQuerySystemInformation");
	}
	else
	{
		csErr = DisplayError(GetLastError());
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
//
// Convenience output routine ;-)
//				
//////////////////////////////////////////////////////////////////////
void CNtRegistry::Output(CString csMsg, DWORD Buttons /*MB_OK*/)
{
	MessageBox(NULL, csMsg, _T("CNtRegistry: Message..."), Buttons);
}

//////////////////////////////////////////////////////////////////////
//
//	Function:	DisplayError
//
//  Purpose:	Display the Error according to the error # passed...I don't 
//				understand just numbers ;)
//
//  Arguments:	DWORD dwLastError - Obvious...
//
//  Returns:	CString - Error message.
//
//////////////////////////////////////////////////////////////////////
CString CNtRegistry::DisplayError(DWORD dwError)
{
	LPVOID lpMessageBuffer = NULL;

	// Load the NTDLL.dll specifically
	HMODULE hNTDLL = LoadLibrary(_T("NTDLL.DLL"));

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_FROM_HMODULE, hNTDLL,
		dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMessageBuffer, 0, NULL);

	CString csReturn = _T("");
	CString csErrorMessage = (LPCTSTR)lpMessageBuffer;

	if (lpMessageBuffer != NULL) {
		LocalFree(lpMessageBuffer);
		csReturn.Format(L"Error( %u [0x%8XL] ) - %s", dwError, dwError, csErrorMessage.Left(csErrorMessage.GetLength() - 2));
	}
	else {
		csReturn.Format(L"Error( %u [0x%8XL] ) - No NT Error Data was returned??", dwError, dwError);
	}

	FreeLibrary(hNTDLL);

	return csReturn;

}

//////////////////////////////////////////////////////////////////////
//
//	Function:	GetTextualSid
//
//  Purpose:	Get the textual Security Identifier (SID) for the 
//				current user.
//
//				NOTE:  This function us called from the LookupSID function
//
//  Arguments:	(IN)  PSID		- Binary SID.
//				(OUT) LPSTR		- Buffer for Textual SID.
//				(IN)  LPDWORD	- Required/provided buffer size.
//
//  Returns:	BOOL - Success/Failure.
//
//////////////////////////////////////////////////////////////////////
BOOL CNtRegistry::GetTextualSid(PSID pSid, LPWSTR szTextualSid, LPDWORD dwBufferLen)
{
	PSID_IDENTIFIER_AUTHORITY psia;
	DWORD dwSubAuthorities;
	DWORD dwSidRev = SID_REVISION;
	DWORD dwCounter;
	DWORD dwSidSize;

	BOOL bReturn = FALSE;
	try {
		{
			//
			// Test if SID passed in is valid.
			if (!IsValidSid(pSid)) {
				goto leave;
			}

			// Obtain SidIdentifierAuthority.
			psia = GetSidIdentifierAuthority(pSid);

			// Obtain sidsubauthority count.
			dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

			// Compute buffer length.
			// S-SID_REVISION- + identifierauthority- + subauthorities- + NULL
			dwSidSize = (15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

			// Check provided buffer length.
			// If not large enough, indicate proper size and setlasterror
			if (*dwBufferLen < dwSidSize) {
				*dwBufferLen = dwSidSize;
				SetLastError(ERROR_INSUFFICIENT_BUFFER);
				m_NtStatus = ERROR_INSUFFICIENT_BUFFER;
				goto leave;
			}

			// Prepare S-SID_REVISION-.
			dwSidSize = wsprintf(szTextualSid, TEXT("S-%lu-"), dwSidRev);

			// Prepare SidIdentifierAuthority.
			if ((psia->Value[0] != 0) || (psia->Value[1] != 0)) {
				dwSidSize += wsprintf(szTextualSid + lstrlen(szTextualSid),
					TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
					(USHORT)psia->Value[0],
					(USHORT)psia->Value[1],
					(USHORT)psia->Value[2],
					(USHORT)psia->Value[3],
					(USHORT)psia->Value[4],
					(USHORT)psia->Value[5]);

			}
			else {
				dwSidSize += wsprintf(szTextualSid + lstrlen(szTextualSid),
					TEXT("%lu"),
					(ULONG)(psia->Value[5]) +
					(ULONG)(psia->Value[4] << 8) +
					(ULONG)(psia->Value[3] << 16) +
					(ULONG)(psia->Value[2] << 24));
			}

			// Loop through SidSubAuthorities.
			for (dwCounter = 0; dwCounter < dwSubAuthorities; dwCounter++) {
				dwSidSize += wsprintf(szTextualSid + dwSidSize, TEXT("-%lu"), *GetSidSubAuthority(pSid, dwCounter));
			}
			bReturn = TRUE;
			//
		} leave:;
	}
	catch (...) {}

	return bReturn;
}

//////////////////////////////////////////////////////////////////////
//
//	Function:	LookupSID
//
//  Purpose:	Lookup the textual Security Identifier (SID) for the 
//				current user.
//
//				NOTE:  This calls the GetTextualSid function above.
//
//  Arguments:	(IN/OUT)  CString - Location to place the SID.
//
//  Returns:	BOOL - Success/Failure.
//
//////////////////////////////////////////////////////////////////////
BOOL CNtRegistry::LookupSID(CString &csSID)
{
	USES_CONVERSION;

	HANDLE		hToken = NULL;
	PTOKEN_USER	ptgUser = NULL;
	DWORD		cbBuffer = 0;
	LPSTR		szTextualSid = NULL;
	DWORD		cbSid = 36;
	TOKEN_INFORMATION_CLASS tic = TokenUser;

	CString csError = _T("Error");
	csSID = csError;

	// Obtain current process token.
	m_NtStatus = NtOpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
	if (!NT_SUCCESS(m_NtStatus)) {
		Output(DisplayError(m_NtStatus), MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// Obtain user identified by current process's access token.
	// Basically, query info in the token ;-)
	m_NtStatus = NtQueryInformationToken(hToken,
		tic,
		ptgUser,
		cbBuffer,
		&cbBuffer);
	if (!NT_SUCCESS(m_NtStatus)) {
		//
		ptgUser = (PTOKEN_USER)RtlAllocateHeap(GetProcessHeap(), HEAP_ZERO_MEMORY, cbBuffer);
		if (!ptgUser) {
			Output(DisplayError(m_NtStatus), MB_OK | MB_ICONERROR);
			goto SIDEnd;
		}

		m_NtStatus = NtQueryInformationToken(hToken,
			tic,
			ptgUser,
			cbBuffer,
			&cbBuffer);
		if (!NT_SUCCESS(m_NtStatus)) {
			Output(DisplayError(m_NtStatus), MB_OK | MB_ICONERROR);
			goto SIDEnd;
		}
	}

	cbSid = 128;
	szTextualSid = (LPSTR)RtlAllocateHeap(GetProcessHeap(), HEAP_ZERO_MEMORY, cbSid);
	if (!szTextualSid) {
		Output(DisplayError(m_NtStatus), MB_OK | MB_ICONERROR);
		goto SIDEnd;
	}

	WCHAR _szTextualSid[200];
	cbSid = 200;
	// Obtain the textual representation of the SID.
	if (!GetTextualSid(ptgUser->User.Sid,	// user binary Sid
		_szTextualSid,		// buffer for TextualSid
		&cbSid))			// size/required buffer
	{
		m_NtStatus = GetLastError();
		Output(DisplayError(m_NtStatus), MB_OK | MB_ICONERROR);
		goto SIDEnd;
	}

	// the TextualSid representation.
	csSID.Format(_T("%s"), _szTextualSid);


SIDEnd:

	// Free resources.
	if (hToken) {
		NtClose(hToken);
	}

	if (ptgUser) {
		RtlFreeHeap(GetProcessHeap(), 0, ptgUser);
	}

	if (szTextualSid) {
		RtlFreeHeap(GetProcessHeap(), 0, szTextualSid);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
//
// The following code can be used to enable or disable the
// backup privilege. By making the indicated substitutions, you can
// also use this code to enable or disable the restore privilege 
//
// Use the following to enable the privilege:
//   hr = EnablePrivilege(SE_BACKUP_NAME, TRUE);
//
// Use the following to disable the privilege:
//   hr = EnablePrivilege(SE_BACKUP_NAME, FALSE);
//
//////////////////////////////////////////////////////////////////////
NTSTATUS CNtRegistry::EnablePrivilege(CString csPrivilege, BOOL bEnable)
{
	TOKEN_PRIVILEGES NewState;
	HANDLE			 hToken = NULL;
	NTSTATUS		 NtStatus = STATUS_SUCCESS;

	// Open the process token for this process.
	NtStatus = NtOpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_QUERY_SOURCE,
		&hToken);
	if (!NT_SUCCESS(NtStatus)) {
		return NtStatus;
	}

	// Get the local unique id for the privilege. This
	// is a Win32 API :-\ I couldn't find a Native one.
	LUID luid;
	if (!LookupPrivilegeValue(NULL,
		(LPCTSTR)csPrivilege,
		&luid))
	{
		NtClose(hToken);
		return (NTSTATUS)ERROR_FUNCTION_FAILED;
	}

	// Assign values to the TOKEN_PRIVILEGE structure.
	NewState.PrivilegeCount = 1;
	NewState.Privileges[0].Luid = luid;
	NewState.Privileges[0].Attributes = (bEnable ? SE_PRIVILEGE_ENABLED : 0);

	// Adjust the token privilege.
	NtStatus = NtAdjustPrivilegesToken(hToken,
		FALSE,
		&NewState,
		sizeof(NewState),
		(PTOKEN_PRIVILEGES)NULL,
		0);
	// Close the handle.
	NtClose(hToken);

	return NtStatus;
}

//////////////////////////////////////////////////////////////////////
//
//	Function:	CreateNewFile
//
//  Purpose:	used to make avaiable registry keys and values stored in  
//				Hive File.
//
//  Arguments:	(IN)  CString	- Root key to load.
//				(IN)  CString	- Hive file to load.
//
//				Privilege: SE_BACKUP_NAME
//
//  Returns:	BOOL - Success/Failure.
//				
//////////////////////////////////////////////////////////////////////
BOOL CNtRegistry::CreateNewFile(CString csFile)
{
	USES_CONVERSION;

	ASSERT(csFile.GetLength() > 0);

	if (csFile.Left(4) != _T("\\??\\")) {
		csFile.Insert(0, _T("\\??\\"));
	}

	ANSI_STRING asFile;
	RtlZeroMemory(&asFile, sizeof(asFile));
	RtlInitAnsiString(&asFile, W2A(csFile));

	UNICODE_STRING usFileName;
	RtlZeroMemory(&usFileName, sizeof(usFileName));

	RtlAnsiStringToUnicodeString(&usFileName, &asFile, TRUE);
	if (m_bHidden) {
		usFileName.MaximumLength = usFileName.Length += 2;
	}


	OBJECT_ATTRIBUTES FileObjectAttributes;
	InitializeObjectAttributes(&FileObjectAttributes,
		&usFileName,
		OBJ_CASE_INSENSITIVE,
		NULL, NULL);

	HANDLE hFile = NULL;
	IO_STATUS_BLOCK ioSB;

	// Open the requested file, exercising the backup privilege
	m_NtStatus = NtCreateFile(&hFile,
		GENERIC_WRITE,
		&FileObjectAttributes,
		&ioSB,
		0,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OVERWRITE_IF,
		FILE_NON_DIRECTORY_FILE,
		NULL, 0);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}

	NtClose(hFile);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
//
// Pretty self explanitory
//				
//////////////////////////////////////////////////////////////////////
CString CNtRegistry::GetCurrentPath(void)
{
	// Must have something in there...
	if (m_csCurrentPath.GetLength() < 1)
		m_csCurrentPath = m_csRootPath;

	return m_csCurrentPath;
}

//////////////////////////////////////////////////////////////////////
//
// Make sure the string includes the "\registry\ ... "
//
//////////////////////////////////////////////////////////////////////
CString CNtRegistry::CheckRegFullPath(CString csPath)
{
	CString csFullPath = _T("");
	CString csTempPath = csPath;

	csTempPath.MakeLower();

	// This is so we know where we stand.
	if (csTempPath.Left(10) != _T("\\registry\\")) {
		//
		csFullPath = GetRootPath();

		// Ok, let's build the full key ...
		if (csTempPath[0] != '\\') {
			csFullPath += _T("\\");
		}

		// Append it...
		csFullPath += csPath;

		// Check if it ends in a slash...if so, take it out!!
		if (csFullPath.Right(1) == _T("\\")) {
			CString csTest(csFullPath);
			csFullPath = csTest.Left(csTest.GetLength() - 1);
		}
	}
	else {
		csFullPath = csPath;
	}

	// put it into a CString Array...
	m_tokenEx.Split(csFullPath, _T("\\"));

	return csFullPath;
}

void CNtRegistry::SetPathVars(CString csKey)
{
	USES_CONVERSION;

	// Make sure the "FullKey" is given...
	CString csFullKey = CheckRegFullPath(csKey);

	ANSI_STRING asKey;
	RtlZeroMemory(&asKey, sizeof(asKey));
	RtlInitAnsiString(&asKey, W2A(csFullKey));

	RtlZeroMemory(&m_usKeyName, sizeof(m_usKeyName));

	RtlAnsiStringToUnicodeString(&m_usKeyName, &asKey, TRUE);
	if (IsKeyHidden(csFullKey)) {
		m_bHidden = TRUE;
		m_usKeyName.MaximumLength = m_usKeyName.Length += 2;
	}
	else {
		m_bHidden = FALSE;
	}
	m_usLength += (USHORT)m_usKeyName.Length;
}

//////////////////////////////////////////////////////////////////////
//
// Pretty self explanitory
//				
//////////////////////////////////////////////////////////////////////
CString CNtRegistry::GetRootPathFor(HKEY hRoot) const
{
	// Defaults to HKCU :-)
	CString csRootPath = _T("");
	if (hRoot == HKEY_LOCAL_MACHINE) {
		csRootPath = _T("\\Registry\\Machine");
	}
	else if (hRoot == HKEY_CLASSES_ROOT) {
		csRootPath = _T("\\Registry\\Machine\\SOFTWARE\\Classes");
	}
	else if (hRoot == HKEY_CURRENT_CONFIG) {
		csRootPath = _T("\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles\\Current");
	}
	else if (hRoot == HKEY_USERS) {
		csRootPath = _T("\\Registry\\User");
	}
	else {
		csRootPath.Format(_T("\\Registry\\User\\%s"), m_csSID);
	}
	return csRootPath;
}

CString CNtRegistry::GetRootKeyString(void) const
{
	// Defaults to HKCU :-)
	CString csRootKey = _T("");
	if (m_hRoot == HKLM) {
		csRootKey = _T("HKEY_LOCAL_MACHINE");
	}
	else if (m_hRoot == HKCR) {
		csRootKey = _T("HKEY_CLASSES_ROOT");
	}
	else if (m_hRoot == HKCC) {
		csRootKey = _T("HKEY_CURRENT_CONFIG");
	}
	else if (m_hRoot == HKU) {
		csRootKey = _T("HKEY_USERS");
	}
	else {
		csRootKey = _T("HKEY_CURRENT_USER");
	}
	return csRootKey;
}

CString CNtRegistry::GetShortRootKeyString(void) const
{
	// Defaults to HKCU :-)
	CString csRootKey = _T("");
	if (m_hRoot == HKLM) {
		csRootKey = _T("HKLM");
	}
	else if (m_hRoot == HKCR) {
		csRootKey = _T("HKCR");
	}
	else if (m_hRoot == HKCC) {
		csRootKey = _T("HKCC");
	}
	else if (m_hRoot == HKU) {
		csRootKey = _T("HKU");
	}
	else {
		csRootKey = _T("HKCU");
	}
	return csRootKey;
}


//////////////////////////////////////////////////////////////////////
//
//	Function:	SetRootKey
//
//  Purpose:	Set the root key for the class.
//
//				NOTE:  Below is the conversion of keys passed in.
//
//				HKEY_LOCAL_MACHINE:	is converted to =>  \Registry\Machine.
//				HKEY_CLASSES_ROOT:	is converted to =>  \Registry\Machine\SOFTWARE\Classes.
//				HKEY_USERS:			is converted to =>  \Registry\User.
//				HKEY_CURRENT_USER:	is converted to =>  \Registry\User\User_SID
//									where User_SID is the current user's 
//									security identifier (SID).
//
//  Arguments:	(IN)  HKEY - Root key.
//
//  Returns:	BOOL - Success/Failure.
//
//////////////////////////////////////////////////////////////////////
BOOL CNtRegistry::SetRootKey(HKEY hKey)
{
	USES_CONVERSION;

	// Set some variables
	m_csRootPath = GetRootPathFor(hKey);
	m_hRoot = hKey;
	m_csCurrentPath = m_csRootPath;

	ANSI_STRING asKey;
	RtlZeroMemory(&asKey, sizeof(asKey));
	RtlInitAnsiString(&asKey, W2A(m_csCurrentPath));

	RtlZeroMemory(&m_usKeyName, sizeof(m_usKeyName));

	RtlAnsiStringToUnicodeString(&m_usKeyName, &asKey, TRUE);
	if (m_bHidden) {
		m_usKeyName.MaximumLength = m_usKeyName.Length += 2;
	}
	m_usLength = m_usKeyName.Length;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
//
//	Function:	SetKey
//
//  Purpose:	Call SetKey to make a specified key the current key. Key is the 
//				name of the key to open. If Key is null, the CurrentKey property 
//				is set to the key specified by the RootKey property.
//
//				CanCreate specifies whether to create the specified key if it does 
//				not exist. If CanCreate is True, the key is created if necessary.
//
//				Key is opened or created with the security access value KEY_ALL_ACCESS. 
//				OpenKey only creates non-volatile keys, A non-volatile key is stored in 
//				the registry and is preserved when the system is restarted. 
//
//				OpenKey returns True if the key is successfully opened or created 
//
//  Arguments:	(IN)  CString	- Key to Set.
//				(IN)  BOOL		- Create the key if it doesn't exist?
//				(IN)  BOOL		- Save as the current key?
//
//  Returns:	BOOL - Success/Failure.
//
//////////////////////////////////////////////////////////////////////
BOOL CNtRegistry::SetKey(HKEY hRoot, CString csKey, BOOL bCanCreate, BOOL bCanSaveCurrentKey)
{
	return (SetRootKey(hRoot) && SetKey(csKey, bCanCreate, bCanSaveCurrentKey));
}

BOOL CNtRegistry::SetKey(CString csKey, BOOL bCanCreate, BOOL bCanSaveCurrentKey)
{
	// Obvious
	CString csFullKey = CheckRegFullPath(csKey);

	// Obvious
	SetPathVars(csFullKey);

	// Obvious
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	// Obvious
	HANDLE hKey = NULL;
	if (bCanCreate) {
		//
		m_dwDisposition = 0;
		// if the key doesn't exist, create it
		m_NtStatus = NtCreateKey(&hKey,
			KEY_ALL_ACCESS,
			&ObjectAttributes,
			0,
			NULL,
			REG_OPTION_NON_VOLATILE,
			&m_dwDisposition);
		//
		if (!NT_SUCCESS(m_NtStatus)) {
			return FALSE;
		}
	}
	else
	{
		// otherwise, open the key without creating it
		m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
		if (!NT_SUCCESS(m_NtStatus)) {
			//Output(DisplayError(m_NtStatus));
			return FALSE;
		}
	}
	////
	//if (m_hMachineReg) {
	//	memcpy(&m_hMachineReg,&hKey,sizeof(HANDLE));
	//}

	// Obvious
	if (bCanSaveCurrentKey) {
		m_csCurrentPath = csFullKey;
	}

	// Obvious
	if (!m_bLazyWrite) {
		NtFlushKey(hKey);
	}

	// Obvious
	if (hKey) {
		NtClose(hKey);
	}

	return TRUE;
}

/****************************************************************************
**
**	Function:	CreateKey
**
**  Purpose:	Use CreateKey to add a new key to the registry.
**				Key is the name of the key to create. Key must be
**				an absolute name. An absolute key
**				begins with a backslash (\) and is a subkey of
**				the root key.
**
**  Arguments:	(IN)  CString	- Name of the key to create.
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::CreateKey(CString csKey)
{
	return SetKey(csKey, TRUE, TRUE);
}


/****************************************************************************
**
**	Function:	CreateHiddenKey
**
**  Purpose:	Call SetKey to make a specified key the current key. Key is the
**				name of the key to open. If Key is null, the CurrentKey property
**				is set to the key specified by the RootKey property.
**
**				OpenKey returns True if the key is successfully opened or created
**
**  Arguments:	(IN)  CString	- Hidden key to create.
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::CreateHiddenKey(CString csKey)
{
	USES_CONVERSION;

	// Make sure the "FullKey" is given...
	CString csFullKey = CheckRegFullPath(csKey);

	ANSI_STRING asKey;
	RtlZeroMemory(&asKey, sizeof(asKey));
	RtlInitAnsiString(&asKey, W2A(csFullKey));

	UNICODE_STRING usKeyName;
	RtlZeroMemory(&usKeyName, sizeof(usKeyName));

	RtlAnsiStringToUnicodeString(&usKeyName, &asKey, TRUE);
	usKeyName.MaximumLength = usKeyName.Length += 2;

	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	m_dwDisposition = 0;

	HANDLE hKey = NULL;
	//
	// if the key doesn't exist, create it
	m_NtStatus = NtCreateKey(&hKey,
		KEY_ALL_ACCESS,
		&ObjectAttributes,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		&m_dwDisposition);
	//
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}

	if (!m_bLazyWrite) {
		NtFlushKey(hKey);
	}
	NtClose(hKey);
	return TRUE;
}

/****************************************************************************
**
**	Function:	RenameKey
**
**  Purpose:	Call RenameKey to rename a specific key
**
**  Arguments:	(IN)  CString	- Name of the key to rename.
**				(IN)  CString	- Name of the new key name.
**
**  Returns:	BOOL - Success/Failure.
**
**  NOTE:		The "csNewKeyName" can ONLY contain the name NO SLASHES !!!
**
****************************************************************************/
BOOL CNtRegistry::RenameKey(CString csFullKey, CString csNewKeyName)
{
	USES_CONVERSION;

	//
	if (!SetKey(csFullKey, FALSE, TRUE)) {
		return FALSE;
	}

	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}

	//if (!IsWinXP()) {
	//
	ANSI_STRING asNewKey;
	RtlZeroMemory(&asNewKey, sizeof(asNewKey));

	RtlInitAnsiString(&asNewKey, W2A(csNewKeyName));

	UNICODE_STRING ReplacementName;
	RtlZeroMemory(&ReplacementName, sizeof(ReplacementName));

	RtlAnsiStringToUnicodeString(&ReplacementName, &asNewKey, TRUE);
	if (m_bHidden) {
		ReplacementName.MaximumLength = ReplacementName.Length += 2;
	}

	m_NtStatus = NtRenameKey(hKey, &ReplacementName);
	if (!NT_SUCCESS(m_NtStatus)) {
		// Let's close this now
		NtClose(hKey);
		return FALSE;
	}
	//}
	//else {
	//	// Let's close this now...we don't need it anymore
	//	NtClose(hKey);
	//	//
	//	int nBackSlash = csFullKey.ReverseFind('\\');
	//	if (nBackSlash == -1) {
	//		return FALSE;
	//	}

	//	CString csNewName = csFullKey.Left(nBackSlash);
	//	csNewName += _T("\\");
	//	csNewName += csNewKeyName;

	//	if (CopyKeys(csFullKey,csNewName)) {
	//		if (!DeleteKeysRecursive(csFullKey)) {
	//			return FALSE;
	//		}
	//	}
	//	else {
	//		return FALSE;
	//	}
	//}
	return TRUE;
}

/****************************************************************************
**
**	Function:	DeleteKey
**
**  Purpose:	Call DeleteKey to remove a specified key and its associated data,
**				if any. Returns FALSE if there are subkeys.  Subkeys must be
**				explicitly deleted by separate calls to DeleteKey.
**
**  Arguments:	(IN)  CString	- Name of the key to delete.
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::DeleteKey(CString csKey)
{
	USES_CONVERSION;

	CString csFullKey = CheckRegFullPath(csKey);

	ANSI_STRING asKey;
	RtlZeroMemory(&asKey, sizeof(asKey));
	RtlInitAnsiString(&asKey, W2A(csFullKey));

	UNICODE_STRING usKeyName;
	RtlZeroMemory(&usKeyName, sizeof(usKeyName));

	RtlAnsiStringToUnicodeString(&usKeyName, &asKey, TRUE);
	if (IsKeyHidden(csFullKey)) {
		usKeyName.MaximumLength = usKeyName.Length += 2;
	}

	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}

	m_NtStatus = NtDeleteKey(hKey);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}
	return TRUE;
}

/****************************************************************************
**
**	Function:	DeleteKeysRecursive
**
**  Purpose:	Call this function to recursively delete subkeys.
**
**  Arguments:	(IN) - Current path to the key to be deleted.
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::DeleteKeysRecursive(CString csKey)
{
	USES_CONVERSION;

	// make sure we have something
	if (m_csRootPath.GetLength() < 1 || csKey.GetLength() < 1) {
		return FALSE;
	}

	// Below we are setting up the path and
	// putting it into a Wide-Char String
	CString csNewPath(m_csRootPath);

	csNewPath.MakeLower();
	csKey.MakeLower();

	if (csNewPath.Left(15) == csKey.Left(15)) {
		csNewPath = csKey;
	}
	else {
		if (csKey[0] != '\\') {
			csNewPath += _T("\\");
		}
		csNewPath += csKey;
	}

	if (csNewPath.Right(1) == _T("\\")) {
		CString csTest(csNewPath);
		csNewPath = csTest.Left(csTest.GetLength() - 1);
	}

	BOOL bHidden = FALSE;
	if (IsKeyHidden(csNewPath)) {
		bHidden = TRUE;
	}

	ANSI_STRING asKey;
	RtlZeroMemory(&asKey, sizeof(asKey));
	RtlInitAnsiString(&asKey, W2A(csNewPath));

	UNICODE_STRING usKeyName;
	RtlZeroMemory(&usKeyName, sizeof(usKeyName));

	RtlAnsiStringToUnicodeString(&usKeyName, &asKey, TRUE);
	if (bHidden) {
		usKeyName.MaximumLength = usKeyName.Length += 2;
	}

	// Init the Obj Attirbutes...
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	// Let's do it
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus))
	{
		// if we are here, somethings wrong??
		return FALSE;
	}

	ULONG resultLength;
	CHAR szSubkeyInfo[1024];

	CString csSubkey, csNewSubkey;

	// Had trouble with
	// Key was opened OK, now let's scan for subkeys
	while ((m_NtStatus = NtEnumerateKey(hKey, 0, KeyBasicInformation, szSubkeyInfo, sizeof(szSubkeyInfo), &resultLength)) == STATUS_SUCCESS)
	{
		// Copy the Data into the structure so we can use it
		PKEY_BASIC_INFORMATION tInfo = (PKEY_BASIC_INFORMATION)szSubkeyInfo;
		// put the subkey name into a variable
		csSubkey = tInfo->Name;
		// now create a new subkey that we need to check
		csNewSubkey.Format(_T("%s\\%s"), csKey, csSubkey.Left(tInfo->NameLength / 2));

		// Now, search for more keys 
		if (!DeleteKeysRecursive(csNewSubkey)) {
			break; // Something failed...break out of for loop
		}
	}

	// Delete the current key
	m_NtStatus = NtDeleteKey(hKey);
	if (!NT_SUCCESS(m_NtStatus)) {
		// Couldn't delete it, so we 
		// have to close it correctly...
		NtClose(hKey);
		return FALSE;
	}
	// All went OK
	return TRUE;
}

/****************************************************************************
**
**	Function:	KeyExists
**
**  Purpose:	Call KeyExists to determine if a key of a
**				specified name exists.
**				Key is the name of the key for which to search.
**				Key must be an absolute name. An absolute key
**				begins with a backslash (\) and is a subkey of
**				the root key.
**
**  Arguments:	(IN)  CString	- Key to check.
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::KeyExists(CString csKey)
{
	return SetKey(csKey, FALSE, FALSE);
}

/****************************************************************************
**
**	Function:	GetSubKeyList
**
**  Purpose:	Call this function to get an array of all the subkeys.
**
**  Arguments:	(IN) - Current path to the key to be enumerated.
**				(IN/OUT) - CStringArray that will receive all the subkey names.
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::GetSubKeyList(CStringArray &csaSubkeys)
{
	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}

	ULONG resultLength;
	CHAR szKeyInfo[1024];
	UINT i = 0;

	CString csSubkey;

	// Scan for subkeys
	while ((m_NtStatus = NtEnumerateKey(hKey, i, KeyBasicInformation, szKeyInfo, sizeof(szKeyInfo), &resultLength)) == STATUS_SUCCESS)
	{
		PKEY_BASIC_INFORMATION tInfo = (PKEY_BASIC_INFORMATION)szKeyInfo;
		csSubkey = tInfo->Name;
		csaSubkeys.Add(csSubkey.Left(tInfo->NameLength / 2));
		i++;
	}

	m_NtStatus = NtClose(hKey);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}
	return TRUE;
}

/****************************************************************************
**
**	Function:	IsKeyHidden
**
**  Purpose:	Call this function to check if it is a hidden subkey.
**
**  Arguments:	(IN) - FULL path to the key to be checked.
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::IsKeyHidden(CString csKey)
{
	USES_CONVERSION;

	// Make sure the "FullKey" is given...
	CString csFullKey = CheckRegFullPath(csKey);

	ANSI_STRING asKey;
	RtlZeroMemory(&asKey, sizeof(asKey));
	RtlInitAnsiString(&asKey, W2A(csFullKey));

	UNICODE_STRING usKeyName;
	RtlZeroMemory(&usKeyName, sizeof(usKeyName));

	RtlAnsiStringToUnicodeString(&usKeyName, &asKey, TRUE);
	usKeyName.Length += 2;
	usKeyName.MaximumLength += 2;

	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}
	NtClose(hKey);
	return TRUE;
}

/****************************************************************************
**
**	Function:	FindHiddenKeys
**
**  Purpose:	Call this function to find hidden subkeys.
**
**  Arguments:	(IN) - Current path to the key to be tested.
**				(IN) - Perform the test recursively?
**				(IN/OUT) - Fills the CStringArray with the findings.
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::FindHiddenKeys(CString csKey, BOOL bRecursive, CStringArray& csaResults)
{
	USES_CONVERSION;

	//
	CString csFullKey = CheckRegFullPath(csKey);

	// Check if it is a "Hidden" key (ending in a "double-NULL")
	CString csMsg = _T("");
	if (IsKeyHidden(csFullKey)) {
		csMsg.Format(_T("%s"), csKey);
		csaResults.Add(csMsg);
	}

	ANSI_STRING asKey;
	RtlZeroMemory(&asKey, sizeof(asKey));
	RtlInitAnsiString(&asKey, W2A(csFullKey));

	UNICODE_STRING usKeyName;
	RtlZeroMemory(&usKeyName, sizeof(usKeyName));

	RtlAnsiStringToUnicodeString(&usKeyName, &asKey, TRUE);

	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (NT_SUCCESS(m_NtStatus)) {
		//
		ULONG resultLength;
		CHAR szSubkeyInfo[1024];
		UINT i = 0;

		CString csSubkey, csNewSubkey;

		// Scan for subkeys
		while ((m_NtStatus = NtEnumerateKey(hKey, i, KeyBasicInformation, szSubkeyInfo, sizeof(szSubkeyInfo), &resultLength)) == STATUS_SUCCESS)
		{
			PKEY_BASIC_INFORMATION tInfo = (PKEY_BASIC_INFORMATION)szSubkeyInfo;
			csSubkey = tInfo->Name;

			csNewSubkey.Format(_T("%s\\%s"), csKey, csSubkey.Left(tInfo->NameLength / 2));
			if (bRecursive) {
				// Search for more keys 
				FindHiddenKeys(csNewSubkey, bRecursive, csaResults);
			}
			else {
				//
				if (IsKeyHidden(csNewSubkey)) {
					//
					csMsg.Format(_T("%s\\%s"), csKey, csNewSubkey);
					csaResults.Add(csMsg);
				}
			}

			// next
			i++;
		}
		NtClose(hKey);
	}
	return TRUE;
}


/****************************************************************************
**
**	Function:	Search for Keys/Values Recursively
**
**  Purpose:	.
**
**  Arguments:	(IN)	 - String to search for.
**				(IN)	 - The Registry Key to start with.
**				(IN/OUT) - Fills the CStringArray with the findings.
**				(IN)	 - Type of search.
**							1 = Key search
**							2 = Value search
**							3 = Both Key/Value search
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::Search(CString csString,
	CString csStartKey,
	CStringArray& csaResults,
	int nRegSearchType /* = 3 */,
	BOOL bCaseSensitive /* = TRUE*/)
{
	USES_CONVERSION;

	if (csString.GetLength() < 1) {
		return FALSE;
	}

	//
	SetPathVars(csStartKey);

	CString csFullKey = CheckRegFullPath(csStartKey);
	CString csTmpKey = csFullKey;

	ANSI_STRING asKey;
	RtlZeroMemory(&asKey, sizeof(asKey));
	RtlInitAnsiString(&asKey, W2A(csFullKey));

	UNICODE_STRING usKeyName;
	RtlZeroMemory(&usKeyName, sizeof(usKeyName));

	RtlAnsiStringToUnicodeString(&usKeyName, &asKey, TRUE);

	if (m_bHidden) {
		usKeyName.Length += 2;
		usKeyName.MaximumLength += 2;
	}

	//
	if (!bCaseSensitive) {
		csTmpKey.MakeLower();
		csString.MakeLower();
	}

	//
	CString csFound = _T("");
	int nFound = -1;

	if (nRegSearchType == 1 || nRegSearchType == 3) {
		// We are searching in Keys, so check the Key
		nFound = csTmpKey.Find(csString, 0);
	}

	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		// Can't go any further :-(
		return FALSE;
	}
	else {
		//
		if (nFound >= 0) {
			// Found it, so add it to the array.
			csaResults.Add(csStartKey);
			// If we here, it means that we are just 
			// searching in Keys, so we need to "goto" 
			// the Exit point...
			goto End_It;
		}

		ULONG resultLength;
		CHAR szValueInfo[8192];
		UINT i = 0;

		CString csValue, csTmp;

		if (nRegSearchType > 1) {
			//
			// Scan for values
			while ((m_NtStatus = NtEnumerateValueKey(hKey, i, KeyValueFullInformation, szValueInfo, sizeof(szValueInfo), &resultLength)) == STATUS_SUCCESS)
			{
				csFound.Empty();

				PKEY_VALUE_FULL_INFORMATION tInfo = (PKEY_VALUE_FULL_INFORMATION)szValueInfo;
				csTmp = tInfo->Name;
				csValue = csTmp.Left(tInfo->NameLength / 2);
				csValue.MakeLower();
				if (csValue == _T("")) {
					csValue = _T("(Default)");
				}

				if (csValue.Find(csString, 0) >= 0) {
					csFound.Format(_T("%s[%s]"), csStartKey, csValue);
					csaResults.Add(csFound);
				}

				if (tInfo->Type == REG_SZ ||
					tInfo->Type == REG_EXPAND_SZ ||
					tInfo->Type == REG_MULTI_SZ)
				{
					CString csValueName;
					csValueName = csTmp.Mid((int)tInfo->NameLength / 2, (int)tInfo->DataLength / 2);
					csValueName.MakeLower();
					if (csValueName.Find(csString, 0) >= 0) {
						//
						csFound.Format(_T("%s[%s]:[%s]"), csStartKey, csValue, csValueName);
						csaResults.Add(csFound);
					}
				}

				// next
				i++;

			} // while
		} // if()

		CHAR szSubkeyInfo[1024];

		CString csSubkey, csNewSubkey;

		i = 0;
		// Scan for subkeys
		while ((m_NtStatus = NtEnumerateKey(hKey, i, KeyBasicInformation, szSubkeyInfo, sizeof(szSubkeyInfo), &resultLength)) == STATUS_SUCCESS)
		{
			PKEY_BASIC_INFORMATION tInfo = (PKEY_BASIC_INFORMATION)szSubkeyInfo;
			csSubkey = tInfo->Name;

			//
			csNewSubkey.Format(_T("%s\\%s"), csStartKey, csSubkey.Left(tInfo->NameLength / 2));

			// Search for more recursively ...
			Search(csString, csNewSubkey, csaResults, nRegSearchType);

			// next
			i++;

		} // while
	}

End_It:

	if (hKey) {
		NtClose(hKey);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
//
//	Function:	CopyKeys
//
//  Purpose:	To copy keys from one location to another in the registry. 
//
//  Arguments:	IN Source (Full Text Key)
//				IN Target (Full Text Key)
//
//  Returns:	BOOL - Success/Failure.
//				
//////////////////////////////////////////////////////////////////////
BOOL CNtRegistry::CopyKeys(CString csSource, CString csTarget, BOOL bRecursively)
{
	CStringArray	csaValueNames;
	CStringArray	csaSubKeys;
	CString			csTrgKey = csTarget;
	CString			csSrcKey = csSource;

	int nSrcSubKey = csSource.ReverseFind('\\');
	CString csSrcSubKey = csSource.Mid(nSrcSubKey);
	csTrgKey += csSrcSubKey;

	BOOL bSrcHidden = IsKeyHidden(csSrcKey);

	// Create Target Key
	if (bSrcHidden) {
		// 
		if (!CreateHiddenKey(csTrgKey)) {
			return FALSE;
		}
	}
	else {
		// 
		if (!CreateKey(csTrgKey)) {
			return FALSE;
		}
	}

	// Set the root for the source key
	SetKey(csSrcKey, FALSE, TRUE);

	//
	// We have been through the values...now let's look at the Keys!!
	csaSubKeys.RemoveAll();
	if (GetSubKeyList(csaSubKeys)) {
		//
		for (int n = 0; n<csaSubKeys.GetSize(); n++) {
			//
			CString csKeyName = csaSubKeys.GetAt(n);
			CString csNewSource = csSrcKey;
			csNewSource += _T("\\");
			csNewSource += csKeyName;
			// recurse with the new subkey
			if (bRecursively) {
				if (!CopyKeys(csNewSource, csTrgKey, bRecursively)) {
					return FALSE;
				}
			}
		}
	}

	// Set the root for the source key
	SetKey(csSrcKey, FALSE, TRUE);

	// Ok, we are still on the "Source" key
	// 
	// Get the list of value names
	csaValueNames.RemoveAll();
	if (GetValueList(csaValueNames)) {
		//
		// Go through the names
		for (int n = 0; n<csaValueNames.GetSize(); n++) {
			//
			CString csValueName = csaValueNames.GetAt(n);

			SetKey(csSrcKey, FALSE, TRUE);

			// Get the "Type" and "DataLength" for the value
			int nSize = 0;
			DWORD dwRegType = GetValueInfo(csValueName, nSize);

			// Read/Write functions now accept SubKeys...see below
			switch (dwRegType) {
				//
				// A 4-byte numerical value
			case REG_DWORD:
			{
				DWORD dw = ReadDword(csSrcKey, csValueName, 0);
				WriteDword(csTrgKey, csValueName, dw);
			}
			break;

			// A zero-terminated Unicode string
			case REG_SZ:
			{
				CString cs = ReadString(csSrcKey, csValueName, _T("ERR"));
				WriteString(csTrgKey, csValueName, cs);
			}
			break;

			// A zero-terminated Unicode string, containing unexpanded
			// references to environment variables, such as "%PATH%"
			case REG_EXPAND_SZ:
			{
				CString cs = ReadString(csSrcKey, csValueName, _T("ERR"));
				WriteExpandString(csTrgKey, csValueName, cs);
			}
			break;

			// An array of zero-terminated strings, terminated by another zero
			case REG_MULTI_SZ:
			{
				CStringArray csaStrings;
				ReadMultiString(csSrcKey, csValueName, csaStrings);
				WriteMultiString(csTrgKey, csValueName, csaStrings);
			}
			break;

			// REG_DWORD_LITTLE_ENDIAN  A 4-byte numerical value whose least significant byte is at the lowest address 
			// REG_DWORD_BIG_ENDIAN A 4-byte numerical value whose least significant byte is at the highest address 
			// REG_LINK				A Unicode string naming a symbolic link. This type is 
			//						irrelevant to device and intermediate drivers 
			// REG_NONE				Data with no particular type 
			// REG_RESOURCE_LIST	A device driver's list of hardware resources, used by the driver 
			//						or one of the physical devices it controls, in the \ResourceMap tree 
			// REG_RESOURCE_REQUIREMENTS_LIST	A device driver's list of possible hardware resources 
			//						it or one of the physical devices it controls can use, from which 
			//						the system writes a subset into the \ResourceMap tree 
			// REG_FULL_RESOURCE_DESCRIPTOR		A list of hardware resources that a physical device 
			// REG_BINARY			Binary data in any form.
			default:
			{
				UINT uiLength = 0;
				UCHAR* uc = ReadBinary(csSrcKey, csValueName, uiLength);
				WriteBinary(csTrgKey, csValueName, uc, uiLength);
			}
			break;

			}
		}
	}
	return TRUE;
}

/****************************************************************************
**
**	Function:	GetSubKeyCount
**
**  Purpose:	Call this function to determine the number of subkeys.
**
**  Arguments:	Nothing.
**
**  Returns:	int - Subkey count (-1 on error).
**
****************************************************************************/
int CNtRegistry::GetSubKeyCount()
{
	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return -1;
	}

	WCHAR buffer[256];
	KEY_FULL_INFORMATION *info = (KEY_FULL_INFORMATION *)buffer;

	DWORD dwResultLength;
	m_NtStatus = NtQueryKey(hKey,
		KeyFullInformation,
		buffer,
		sizeof(buffer),
		&dwResultLength);

	NtClose(hKey);
	if (!NT_SUCCESS(m_NtStatus)) {
		return -1;
	}
	return (int)info->SubKeys;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

/****************************************************************************
**
**	Function:	ValueExists
**
**  Purpose:	Call ValueExists to determine if a particular key exists in
**				the registry. Calling Value Exists is especially useful before
**				calling other NtRegistry methods that operate only on existing
**				keys.
**
**				Name is the name of the data value for which to check.
**				ValueExists returns True if a match if found, False otherwise.
**
**  Arguments:	(IN)  CString	- Name of the value to check.
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::ValueExists(CString csName)
{
	USES_CONVERSION;

	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}

	if (csName == _T("(Default)")) {
		csName = _T("");
	}

	ANSI_STRING asName;
	RtlZeroMemory(&asName, sizeof(asName));
	RtlInitAnsiString(&asName, W2A(csName));

	UNICODE_STRING ValueName;
	RtlZeroMemory(&ValueName, sizeof(ValueName));

	RtlAnsiStringToUnicodeString(&ValueName, &asName, TRUE);
	if (m_bHidden) {
		ValueName.MaximumLength = ValueName.Length += 2;
	}

	WCHAR buffer[8192];
	DWORD dwSize;
	m_NtStatus = NtQueryValueKey(hKey,
		&ValueName,
		KeyValueFullInformation,
		buffer,
		sizeof(buffer),
		&dwSize);

	NtClose(hKey);
	if (!NT_SUCCESS(m_NtStatus)) {
		//Output(DisplayError(m_NtStatus));
		return FALSE;
	}
	return TRUE;
}

/****************************************************************************
**
**	Function:	RenameValue
**
**  Purpose:	Call DeleteValue to remove a specific data value
**				associated with the current key. Name is string
**				containing the name of the value to delete. Keys can contain
**				multiple data values, and every value associated with a key
**				has a unique name.
**
**  Arguments:	(IN)  CString	- Name of the value to delete.
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::RenameValue(CString csOldName, CString csNewName)
{
	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}

	//
	BOOL bReturn = FALSE;

	//
	CString csVal = _T("");
	CStringArray csaVal;
	DWORD dwVal = 0;
	UCHAR* ucVal = NULL;
	UINT uiLength = 0;

	int nSize = 0;
	DWORD dwType = GetValueInfo(csOldName, nSize);
	if (dwType != REG_NONE) {
		switch (dwType) {

		case REG_DWORD:
			dwVal = ReadDword(csOldName, 0);
			if (WriteDword(csNewName, dwVal)) {
				bReturn = DeleteValue(csOldName);
			}
			break;

		case REG_SZ:
			csVal = ReadString(csOldName, _T("ERR"));
			//				if (csVal != _T("ERR")) {
			if (WriteString(csNewName, csVal)) {
				bReturn = DeleteValue(csOldName);
			}
			//				}
			break;

		case REG_EXPAND_SZ:
			csVal = ReadString(csOldName, _T("ERR"));
			//				if (csVal != _T("ERR")) {
			if (WriteExpandString(csNewName, csVal)) {
				bReturn = DeleteValue(csOldName);
			}
			//				}
			break;

		case REG_MULTI_SZ:
			if (ReadMultiString(csOldName, csaVal)) {
				if (WriteMultiString(csNewName, csaVal)) {
					bReturn = DeleteValue(csOldName);
				}
			}
			break;

		default:
			ucVal = ReadBinary(csOldName, uiLength);
			//				if (ucVal[0] != 'E' && ucVal[1] != 'R' && ucVal[2] != 'R') {
			if (WriteBinary(csNewName, ucVal, uiLength)) {
				bReturn = DeleteValue(csOldName);
			}
			//				}
			break;

		}
	}

	NtClose(hKey);

	return bReturn;
}


//////////////////////////////////////////////////////////////////////
//
//	Function:	CopyValues
//
//  Purpose:	To copy values from one location to another in the registry. 
//
//  Arguments:	IN Source (Full Text Key)
//				IN Target (Full Text Key)
//				IN ValueName
//
//  Returns:	BOOL - Success/Failure.
//				
//////////////////////////////////////////////////////////////////////
BOOL CNtRegistry::CopyValues(CString csSource, CString csTarget, CString csValueName, CString csNewValueName)
{
	CString			csTrgKey = csTarget;
	CString			csSrcKey = csSource;

	// Set the root for the source key
	SetKey(csSrcKey, FALSE, TRUE);

	if (ValueExists(csValueName)) {
		//
		// Get the "Type" and "DataLength" for the value
		int nSize = 0;
		DWORD dwRegType = GetValueInfo(csValueName, nSize);

		//
		BOOL bSrcHidden = IsKeyHidden(csSrcKey);

		// Create Target Key
		if (bSrcHidden) {
			// 
			if (!CreateHiddenKey(csTrgKey)) {
				return FALSE;
			}
		}
		else {
			// 
			if (!CreateKey(csTrgKey)) {
				return FALSE;
			}
		}

		switch (dwRegType) {
			//
			// A 4-byte numerical value
		case REG_DWORD:
		{
			DWORD dw = ReadDword(csSrcKey, csValueName, 0);
			return WriteDword(csTrgKey, csNewValueName, dw);
		}

		// A zero-terminated Unicode string
		case REG_SZ:
		{
			CString cs = ReadString(csSrcKey, csValueName, _T("ERR"));
			return WriteString(csTrgKey, csNewValueName, cs);
		}

		// A zero-terminated Unicode string, containing unexpanded
		// references to environment variables, such as "%PATH%"
		case REG_EXPAND_SZ:
		{
			CString cs = ReadString(csSrcKey, csValueName, _T("ERR"));
			return WriteExpandString(csTrgKey, csNewValueName, cs);
		}

		// An array of zero-terminated strings, terminated by another zero
		case REG_MULTI_SZ:
		{
			CStringArray csaStrings;
			ReadMultiString(csSrcKey, csValueName, csaStrings);
			return WriteMultiString(csTrgKey, csNewValueName, csaStrings);
		}

		// REG_DWORD_LITTLE_ENDIAN  A 4-byte numerical value whose least significant byte is at the lowest address 
		// REG_DWORD_BIG_ENDIAN A 4-byte numerical value whose least significant byte is at the highest address 
		// REG_LINK				A Unicode string naming a symbolic link. This type is 
		//						irrelevant to device and intermediate drivers 
		// REG_NONE				Data with no particular type 
		// REG_RESOURCE_LIST	A device driver's list of hardware resources, used by the driver 
		//						or one of the physical devices it controls, in the \ResourceMap tree 
		// REG_RESOURCE_REQUIREMENTS_LIST	A device driver's list of possible hardware resources 
		//						it or one of the physical devices it controls can use, from which 
		//						the system writes a subset into the \ResourceMap tree 
		// REG_FULL_RESOURCE_DESCRIPTOR		A list of hardware resources that a physical device 
		// REG_BINARY			Binary data in any form.
		default:
		{
			UINT uiLength = 0;
			UCHAR* uc = ReadBinary(csSrcKey, csValueName, uiLength);
			return WriteBinary(csTrgKey, csNewValueName, uc, uiLength);
		}

		}
	}
	return FALSE;
}

/****************************************************************************
**
**	Function:	DeleteValue
**
**  Purpose:	Call DeleteValue to remove a specific data value
**				associated with the current key. Name is string
**				containing the name of the value to delete. Keys can contain
**				multiple data values, and every value associated with a key
**				has a unique name.
**
**  Arguments:	(IN)  CString	- Name of the value to delete.
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::DeleteValue(CString csName)
{
	USES_CONVERSION;

	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}

	// Set the path string
	ANSI_STRING asName;
	RtlZeroMemory(&asName, sizeof(asName));
	RtlInitAnsiString(&asName, W2A(csName));

	UNICODE_STRING ValueName;
	RtlZeroMemory(&ValueName, sizeof(ValueName));

	RtlAnsiStringToUnicodeString(&ValueName, &asName, TRUE);

	m_NtStatus = NtDeleteValueKey(hKey, &ValueName);

	NtClose(hKey);
	if (NT_SUCCESS(m_NtStatus)) {
		return TRUE;
	}
	return FALSE;
}

/********************************************************************************
**
**	Function:	GetValueInfo
**
**  Purpose:	Call GetValueInfo to determine the type/size of a data value
**				associated with the current key. ValueName is a string
**				containing the name of the data value to query.  On success,
**				GetDataType returns the type of the data value.
**				On failure, GetValueInfo returns REG_NONE
**
**  Arguments:	(IN)	 CString	- Value name.
**				(IN/OUT) int		- Value to get the size.
**
**  Returns:	DWORD - Reg value type (see below).
**
**				REG_XXX Type Value:
**				=================================================================
**				REG_BINARY -	Binary data in any form
**				REG_DWORD -		A 4-byte numerical value
**				REG_DWORD_LITTLE_ENDIAN - A 4-byte numerical value whose least
**								significant byte is at the lowest
**								address
**				REG_DWORD_BIG_ENDIAN - A 4-byte numerical value whose least
**								significant byte is at the highest
**								address
**				REG_EXPAND_SZ - A zero-terminated Unicode string, containing
**								unexpanded references to environment variables,
**								such as "%PATH%"
**				REG_LINK -		A Unicode string naming a symbolic link. This type
**								is irrelevant to device and intermediate drivers
**				REG_MULTI_SZ -	An array of zero-terminated strings, terminated by
**								another zero
**
**				REG_NONE -		Data with no particular type
**				REG_SZ -		A zero-terminated Unicode string
**				REG_RESOURCE_LIST - A device driver's list of hardware resources,
**								used by the driver or one of the physical
**								devices it controls, in the \ResourceMap tree
**				REG_RESOURCE_REQUIREMENTS_LIST - A device driver's list of possible
**								hardware resources it or one of the
**								physical devices it controls can
**								use, from which the system writes a
**								subset into the \ResourceMap tree
**
**				REG_FULL_RESOURCE_DESCRIPTOR - A list of hardware resources that a
**								physical device is using, detected
**								and written into the \HardwareDescription
**								tree by the system
**
**********************************************************************************/
DWORD CNtRegistry::GetValueInfo(CString csName, int& nSize)
{
	USES_CONVERSION;

	// Set it to zero...
	nSize = 0;

	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return REG_NONE;
	}

	//	BOOL bDefault = FALSE;
	if (csName == _T("(Default)")) {
		csName = _T("");
		//		bDefault = TRUE;
	}

	// Set the path string
	ANSI_STRING asName;
	RtlZeroMemory(&asName, sizeof(asName));
	RtlInitAnsiString(&asName, W2A(csName));

	UNICODE_STRING ValueName;
	RtlZeroMemory(&ValueName, sizeof(ValueName));

	RtlAnsiStringToUnicodeString(&ValueName, &asName, TRUE);

	BYTE *Buffer = NULL;
	DWORD dwDataSize = 0;

	KEY_VALUE_FULL_INFORMATION *info;// = (KEY_VALUE_PARTIAL_INFORMATION *)buffer;

	m_NtStatus = STATUS_SUCCESS;

	if (NtQueryValueKey(hKey, &ValueName, KeyValueFullInformation,
		NULL, 0, &dwDataSize) == STATUS_BUFFER_OVERFLOW)
	{
		do {
			Buffer = (BYTE*)HeapAlloc(GetProcessHeap(), 0, dwDataSize + 1024 + sizeof(WCHAR));
			if (!Buffer) {
				NtClose(hKey);
				return REG_NONE;
			}

			m_NtStatus = NtQueryValueKey(hKey, &ValueName, KeyValueFullInformation,
				Buffer, dwDataSize, &dwDataSize);

		} while (m_NtStatus == STATUS_BUFFER_OVERFLOW);
	}
	else
	{
		//if (bDefault) {
		//	nSize = 0;
		//	NtClose(hKey);
		//	return REG_SZ;
		//}

		Buffer = (BYTE*)HeapAlloc(GetProcessHeap(), 0, dwDataSize + 1024);
		if (!Buffer) {
			NtClose(hKey);
			return REG_NONE;
		}

		m_NtStatus = NtQueryValueKey(hKey,
			&ValueName,
			KeyValueFullInformation,
			Buffer,
			dwDataSize,
			&dwDataSize);

	}

	info = (KEY_VALUE_FULL_INFORMATION *)Buffer;

	NtClose(hKey);

	if (!NT_SUCCESS(m_NtStatus)) {
		//		Output(DisplayError(m_NtStatus));
		return REG_NONE;
	}

	nSize = info->DataLength / 2;

	return info->Type;
}


/****************************************************************************
**
**	Function:	GetValueCount
**
**  Purpose:	Call this function to determine the number of values.
**
**  Arguments:	Nothing.
**
**  Returns:	int - Subkey count (-1 on error).
**
****************************************************************************/
int CNtRegistry::GetValueCount()
{
	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return -1;
	}

	WCHAR buffer[256];
	KEY_FULL_INFORMATION *info = (KEY_FULL_INFORMATION *)buffer;

	DWORD dwResultLength;
	m_NtStatus = NtQueryKey(hKey,
		KeyFullInformation,
		buffer,
		sizeof(buffer),
		&dwResultLength);

	NtClose(hKey);
	if (!NT_SUCCESS(m_NtStatus)) {
		return -1;
	}
	return (int)info->Values;
}

/****************************************************************************
**
**	Function:	GetValueList
**
**  Purpose:	Call this function to get an array of all the values.
**
**  Arguments:	(IN) - Current path to the value to be enumerated.
**				(IN/OUT) - CStringArray that will receive all the value names.
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::GetValueList(CStringArray &csaValues)
{
	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}

	ULONG resultLength;
	CHAR szValueInfo[1024];
	UINT i = 0;

	CString csValue;

	NTSTATUS NtStatus;
	// Scan for subkeys
	while ((NtStatus = NtEnumerateValueKey(hKey, i, KeyValueBasicInformation, szValueInfo, sizeof(szValueInfo), &resultLength)) == STATUS_SUCCESS)
	{
		PKEY_VALUE_BASIC_INFORMATION tInfo = (PKEY_VALUE_BASIC_INFORMATION)szValueInfo;
		csValue = tInfo->Name;
		csaValues.Add(csValue.Left(tInfo->NameLength / 2));
		i++;
	}

	NtClose(hKey);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
//
// NtRegistry data reading functions
//
//////////////////////////////////////////////////////////////////////////////

/****************************************************************************
**
**	Function:	ReadBinary
**
**  Purpose:	Call this function to read binary entries in the registry.
**
**  Arguments:	(IN) - Name of the value to be read.
**				(IN) - In the event of failure, this is what it will return.
**
**  Returns:	CString - Binary formatted string.
**
****************************************************************************/
UCHAR* CNtRegistry::ReadBinary(CString csKey, CString csName, UINT& uiLength)
{
	USES_CONVERSION;

	DWORD dwType = REG_BINARY;
	DWORD dwSize;

	int n = 0;
	static UCHAR pRetErr[3] = { 'E', 'R', 'R' };
	uiLength = 3;

	int nSize = 0;


	// Make sure the "Key" is set...
	SetPathVars(csKey);

	// make sure it is the proper type .... putting a lot here until
	// I make the corresponding "ReadXXXXX" for it ;-)
	dwType = GetValueInfo(csName, nSize);
	if (dwType != REG_RESOURCE_LIST) {
		if (dwType != REG_FULL_RESOURCE_DESCRIPTOR) {
			if (dwType != REG_RESOURCE_REQUIREMENTS_LIST) {
				if (dwType != REG_NONE) {
					if (dwType != REG_QWORD) {
						if (dwType != REG_BINARY) {
							return pRetErr;
						}
					}
				}
			}
		}
	}

	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_READ, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return pRetErr;
	}

	//
	if (csName == _T("(Default)")) {
		csName = _T("");
	}

	// Set the path string
	ANSI_STRING asName;
	RtlZeroMemory(&asName, sizeof(asName));
	RtlInitAnsiString(&asName, W2A(csName));

	UNICODE_STRING ValueName;
	RtlZeroMemory(&ValueName, sizeof(ValueName));

	RtlAnsiStringToUnicodeString(&ValueName, &asName, TRUE);

	//
	WCHAR buffer[4096];
	KEY_VALUE_PARTIAL_INFORMATION *info = (KEY_VALUE_PARTIAL_INFORMATION *)buffer;

	m_NtStatus = NtQueryValueKey(hKey,
		&ValueName,
		KeyValuePartialInformation,
		&buffer,
		sizeof(buffer),
		&dwSize);

	NtClose(hKey);

	if (!NT_SUCCESS(m_NtStatus)) {
		return pRetErr;
	}

	//	BOOL bIsPWCHAR = TRUE;
	UCHAR *pBinary = (UCHAR*)info->Data;
	static UCHAR pReturn[8192];

	for (n = 0; n < (int)info->DataLength; n++) {
		//
		if (n>8190) {
			break;
		}
		pReturn[n] = pBinary[n];
	}

	uiLength = n;

	return pReturn;
}


/****************************************************************************
**
**	Function:	ReadString
**
**  Purpose:	Call this function to read string entries in the registry.
**
**  Arguments:	(IN) - Name of the value to be read.
**				(IN) - In the event of failure, this is what it will return.
**
**  Returns:	CString - String.
**
****************************************************************************/
CString CNtRegistry::ReadString(CString csKey, CString csName, CString csDefault)
{
	USES_CONVERSION;

	DWORD dwType = REG_SZ;
	int nSize = 0;
	BYTE *Buffer = NULL;
	DWORD dwDataSize = 0;
	int n = 0;

	HANDLE hKey = NULL;


	CString csTmp = _T("");
	CString csReturn = csDefault;


	// Make sure the "Key" is set...
	SetPathVars(csKey);

	// make sure it is the proper type
	if (csName == _T("")) {
		dwType = REG_SZ;
	}
	else {
		dwType = GetValueInfo(csName, nSize);
		if (dwType != REG_SZ && dwType != REG_EXPAND_SZ) {
			goto Quit;
		}
	}

	csReturn += _T(": IN");

	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	m_NtStatus = NtOpenKey(&hKey, KEY_READ, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		goto Quit;
	}

	//	BOOL bDefault = FALSE;
	if (csName == _T("(Default)")) {
		csName = _T("");
		//		bDefault = TRUE;
	}

	// Set the path string
	ANSI_STRING asName;
	RtlZeroMemory(&asName, sizeof(asName));
	RtlInitAnsiString(&asName, W2A(csName));

	UNICODE_STRING ValueName;
	RtlZeroMemory(&ValueName, sizeof(ValueName));

	RtlAnsiStringToUnicodeString(&ValueName, &asName, TRUE);

	KEY_VALUE_PARTIAL_INFORMATION *info;// = (KEY_VALUE_PARTIAL_INFORMATION *)buffer;

	m_NtStatus = STATUS_SUCCESS;
	if (NtQueryValueKey(hKey, &ValueName, KeyValuePartialInformation,
		NULL, 0, &dwDataSize) == STATUS_BUFFER_OVERFLOW)
	{
		do {
			Buffer = (BYTE*)HeapAlloc(GetProcessHeap(), 0, dwDataSize + 1024 + sizeof(WCHAR));
			if (!Buffer) {
				csReturn += _T(": do { HeapAlloc() } Error!!");
				goto Quit;
			}

			m_NtStatus = NtQueryValueKey(hKey, &ValueName, KeyValuePartialInformation,
				Buffer, dwDataSize, &dwDataSize);

		} while (m_NtStatus == STATUS_BUFFER_OVERFLOW);
	}
	else
	{
		Buffer = (BYTE*)HeapAlloc(GetProcessHeap(), 0, dwDataSize + 1024);
		if (!Buffer) {
			csReturn += _T(": HeapAlloc() Error!!");
			goto Quit;
		}

		m_NtStatus = NtQueryValueKey(hKey,
			&ValueName,
			KeyValuePartialInformation,
			Buffer,
			dwDataSize,
			&dwDataSize);

	}

	if (!NT_SUCCESS(m_NtStatus)) {
		goto Quit;
	}

	info = (KEY_VALUE_PARTIAL_INFORMATION *)Buffer;

	csReturn = _T("");
	for (n = 0; n<(int)(info->DataLength); n++)
	{
		char sz[2];
		//#if _MSC_VER > 1200
		//		sprintf_s(sz,2,"%c",info->Data[n]);
		//#else
		_snprintf(sz, 2, "%c", info->Data[n]);
		//#endif
		csReturn += sz;
	}

Quit:

	if (hKey) {
		NtClose(hKey);
	}
	if (Buffer) {
		HeapFree(GetProcessHeap(), 0, Buffer);
	}
	return csReturn;

}


/****************************************************************************
**
**	Function:	ReadMultiString
**
**  Purpose:	Call this function to read "multi" string entries in the registry.
**
**  Arguments:	(IN)	 - Name of the value to be read.
**				(IN/OUT) - CStringArray that will receive the entry
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::ReadMultiString(CString csKey, CString csName, CStringArray& csaReturn)
{
	USES_CONVERSION;

	//
	DWORD dwType = REG_MULTI_SZ;
	DWORD dwSize = 255;

	csaReturn.RemoveAll();

	int nSize = 0;


	// Make sure the "Key" is set...
	SetPathVars(csKey);

	// make sure it is the proper type
	dwType = GetValueInfo(csName, nSize);
	if (dwType != REG_MULTI_SZ) {
		return FALSE;
	}

	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_READ, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}

	if (csName == _T("(Default)")) {
		csName = _T("");
	}

	// Set the path string
	ANSI_STRING asName;
	RtlZeroMemory(&asName, sizeof(asName));
	RtlInitAnsiString(&asName, W2A(csName));

	UNICODE_STRING ValueName;
	RtlZeroMemory(&ValueName, sizeof(ValueName));

	RtlAnsiStringToUnicodeString(&ValueName, &asName, TRUE);

	//
	WCHAR buffer[4096];
	KEY_VALUE_PARTIAL_INFORMATION *info = (KEY_VALUE_PARTIAL_INFORMATION *)buffer;

	m_NtStatus = NtQueryValueKey(hKey,
		&ValueName,
		KeyValuePartialInformation,
		buffer,
		sizeof(buffer),
		&dwSize);

	NtClose(hKey);

	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}

	CString csReturn = _T("");
	CString csTmp = _T("");
	for (int i = 6; i<(int)(info->DataLength / 2) + 6; i++)
	{
		// if a NULL, then this ends the current string
		if (buffer[i] == '\0')
		{
			csaReturn.Add(csReturn);
			csReturn = _T("");

			// if another NULL, then this ends all
			if (buffer[i + 1] == '\0')
				break;

			// Start with the new string
			continue;
		}

		char sz[2];
		//#if _MSC_VER > 1200
		//		sprintf_s(sz,2,"%c",buffer[i]);
		//#else
		_snprintf(sz, 2, "%c", buffer[i]);
		//#endif
		csReturn += sz;
	}
	return TRUE;
}


/////////////////////////////////////////////////////////
//
//
//
//
/////////////////////////////////////////////////////////

DWORD CNtRegistry::ReadDword(CString csKey, CString csName, DWORD dwDefault)
{
	KEY_VALUE_PARTIAL_INFORMATION* info = NULL;

	if (ReadValue(csKey, csName, REG_DWORD, &info)) {
		dwDefault = (DWORD&)info->Data;
	}
	return dwDefault;
}

/****************************************************************************
**  Purpose:	See "ReadValue" for explination
****************************************************************************/
int CNtRegistry::ReadInt(CString csKey, CString csName, int nDefault)
{
	KEY_VALUE_PARTIAL_INFORMATION* info = NULL;

	if (ReadValue(csKey, csName, REG_DWORD, &info)) {
		nDefault = (int&)info->Data;
	}
	return nDefault;
}

/****************************************************************************
**  Purpose:	See "ReadValue" for explination
****************************************************************************/
BOOL CNtRegistry::ReadBool(CString csKey, CString csName, BOOL bDefault)
{
	KEY_VALUE_PARTIAL_INFORMATION* info = NULL;

	if (ReadValue(csKey, csName, REG_DWORD, &info)) {
		bDefault = (BOOL&)info->Data;
	}
	return bDefault;
}

/****************************************************************************
**  Purpose:	See "ReadValue" for explination
****************************************************************************/
COleDateTime CNtRegistry::ReadDateTime(CString csKey, CString csName, COleDateTime dtDefault)
{
	KEY_VALUE_PARTIAL_INFORMATION* info = NULL;

	if (ReadValue(csKey, csName, REG_BINARY, &info)) {
		dtDefault = (COleDateTime&)info->Data;
	}
	return dtDefault;
}

/****************************************************************************
**  Purpose:	See "ReadValue" for explination
****************************************************************************/
double CNtRegistry::ReadFloat(CString csKey, CString csName, double fDefault)
{
	KEY_VALUE_PARTIAL_INFORMATION* info = NULL;

	if (ReadValue(csKey, csName, REG_BINARY, &info)) {
		fDefault = (double&)info->Data;
	}
	return fDefault;
}

/****************************************************************************
**  Purpose:	See "ReadValue" for explination
****************************************************************************/
COLORREF CNtRegistry::ReadColor(CString csKey, CString csName, COLORREF crDefault)
{
	KEY_VALUE_PARTIAL_INFORMATION* info = NULL;

	if (ReadValue(csKey, csName, REG_BINARY, &info)) {
		crDefault = (COLORREF&)info->Data;
	}
	return crDefault;
}

/****************************************************************************
**  Purpose:	See "ReadValue" for explination
****************************************************************************/
BOOL CNtRegistry::ReadFont(CString csKey, CString csName, CFont* pValue)
{
	KEY_VALUE_PARTIAL_INFORMATION* info = NULL;

	BOOL bReturn = ReadValue(csKey, csName, REG_BINARY, &info);
	if (bReturn) {
		memcpy(pValue, info->Data, sizeof(CFont));
		pValue->Detach();
		LOGFONT lf;
		pValue->CreateFontIndirect(&lf);
	}
	else {
		pValue = (CFont*)NULL;
	}
	return bReturn;
}

/****************************************************************************
**  Purpose:	See "ReadValue" for explination
****************************************************************************/
BOOL CNtRegistry::ReadPoint(CString csKey, CString csName, CPoint* pValue)
{
	KEY_VALUE_PARTIAL_INFORMATION* info = NULL;

	BOOL bReturn = ReadValue(csKey, csName, REG_BINARY, &info);
	if (bReturn) {
		memcpy(pValue, info->Data, sizeof(CPoint));
	}
	else {
		pValue = (CPoint*)NULL;
	}
	return bReturn;
}

/****************************************************************************
**  Purpose:	See "ReadValue" for explination
****************************************************************************/
BOOL CNtRegistry::ReadSize(CString csKey, CString csName, CSize* pValue)
{
	KEY_VALUE_PARTIAL_INFORMATION* info = NULL;

	BOOL bReturn = ReadValue(csKey, csName, REG_BINARY, &info);
	if (bReturn) {
		memcpy(pValue, info->Data, sizeof(CSize));
	}
	else {
		pValue = (CSize*)NULL;
	}
	return bReturn;
}

/****************************************************************************
**  Purpose:	See "ReadValue" for explination
****************************************************************************/
BOOL CNtRegistry::ReadRect(CString csKey, CString csName, CRect* pValue)
{
	KEY_VALUE_PARTIAL_INFORMATION* info = NULL;

	BOOL bReturn = ReadValue(csKey, csName, REG_BINARY, &info);
	if (bReturn) {
		memcpy(pValue, info->Data, sizeof(CRect));
	}
	else {
		pValue = (CRect*)NULL;
	}
	return bReturn;
}

/****************************************************************************
**
**	Function:	ReadValue
**
**  Purpose:	Call this function to read entries in the registry.
**
**  Arguments:	(IN) - Name of the value.
**				(IN) - Registry Type of value (i.e. REG_SZ).
**				(IN) - Data Returned.
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::ReadValue(CString csKey, CString csName, DWORD dwRegType, KEY_VALUE_PARTIAL_INFORMATION** retInfo)
{
	USES_CONVERSION;

	DWORD dwSize = 0;
	DWORD dwType = REG_NONE;
	int nSize = 0;


	// Make sure the "Key" is set...
	SetPathVars(csKey);

	// make sure it is the proper type
	dwType = GetValueInfo(csName, nSize);
	if (dwType != dwRegType) {
		return FALSE;
	}

	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_READ, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}

	if (csName == _T("(Default)")) {
		csName = _T("");
	}

	// Set the path string
	ANSI_STRING asName;
	RtlZeroMemory(&asName, sizeof(asName));
	RtlInitAnsiString(&asName, W2A(csName));

	UNICODE_STRING ValueName;
	RtlZeroMemory(&ValueName, sizeof(ValueName));

	RtlAnsiStringToUnicodeString(&ValueName, &asName, TRUE);

	//
	WCHAR infoBuffer[256];
	memset(infoBuffer, 0, 256);
	*retInfo = (KEY_VALUE_PARTIAL_INFORMATION *)infoBuffer;

	m_NtStatus = NtQueryValueKey(hKey,
		&ValueName,
		KeyValuePartialInformation,
		&infoBuffer,
		sizeof(infoBuffer),
		&dwSize);

	NtClose(hKey);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}
	return TRUE;
}




//////////////////////////////////////////////////////////////////////////////
// NtRegistry data writting functions
//////////////////////////////////////////////////////////////////////////////

/****************************************************************************
**  Purpose:	See "WriteValueString" for explination
****************************************************************************/
BOOL CNtRegistry::WriteString(CString csKey, CString csName, CString csValue)
{
	return WriteValueString(csKey, csName, (LPCTSTR)csValue, csValue.GetLength(), REG_SZ);
}

/****************************************************************************
**  Purpose:	See "WriteValueString" for explination
****************************************************************************/
BOOL CNtRegistry::WriteExpandString(CString csKey, CString csName, CString csValue)
{
	return WriteValueString(csKey, csName, (LPCTSTR)csValue, csValue.GetLength(), REG_EXPAND_SZ);
}

/****************************************************************************
**  Purpose:	See "WriteValueString" for explination
****************************************************************************/
BOOL CNtRegistry::WriteMultiString(CString csKey, CString csName, CStringArray& csaValue)
{
	int nLength = 0;
	char lpszValue[1024];

	CString csTmp = _T("");
	for (int n = 0; n<csaValue.GetSize(); n++) {
		//
		csTmp = csaValue.GetAt(n);
		for (int i = 0; i<csTmp.GetLength(); i++)
		{
			lpszValue[nLength] = csTmp[i];
			nLength++;
		}
		lpszValue[nLength++] = '\0';
	}

	return WriteValueString(csKey, csName, (LPCTSTR)lpszValue, nLength, REG_MULTI_SZ);
}


/****************************************************************************
**
**	Function:	WriteValueString
**
**  Purpose:	Call this function to write entries in the registry.
**
**  Arguments:	(IN) - Name of the value.
**				(IN) - Value written to the registry.
**				(IN) - Length of Value to write.
**				(IN) - Registry Type value.
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::WriteValueString(CString csKey, CString csName, LPCTSTR lpszValue, int nLength, DWORD dwRegType)
{
	USES_CONVERSION;

	// Make sure the "Key" is set...
	SetPathVars(csKey);

	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}

	if (csName == _T("(Default)")) {
		csName = _T("");
	}

	ANSI_STRING asName;
	RtlZeroMemory(&asName, sizeof(asName));
	RtlInitAnsiString(&asName, W2A(csName));

	UNICODE_STRING ValueName;
	RtlZeroMemory(&ValueName, sizeof(ValueName));

	RtlAnsiStringToUnicodeString(&ValueName, &asName, TRUE);

	WCHAR wszValue[1024];

	//
	int n = 0;
	for (n = 0; n<nLength; n++) {
		wszValue[n] = (WCHAR)lpszValue[n];
	}
	wszValue[n++] = L'\0';

	m_NtStatus = NtSetValueKey(hKey,
		&ValueName,
		0,
		dwRegType,		// i.e. REG_MULTI_SZ
		wszValue,
		(ULONG)nLength * sizeof(WCHAR));

	if (!NT_SUCCESS(m_NtStatus)) {
		NtClose(hKey);
		return FALSE;
	}

	if (!m_bLazyWrite) {
		NtFlushKey(hKey);
	}

	NtClose(hKey);

	return TRUE;
}


/****************************************************************************
**  Purpose:	See "WriteValue" for explination
****************************************************************************/
BOOL CNtRegistry::WriteBinary(CString csKey, CString csName, UCHAR* pValue, UINT uiLength)
{
	return WriteValue(csKey, csName, pValue, (ULONG)uiLength, REG_BINARY);
}

/****************************************************************************
**  Purpose:	See "WriteValue" for explination
****************************************************************************/
BOOL CNtRegistry::WriteBool(CString csKey, CString csName, BOOL bValue)
{
	return WriteValue(csKey, csName, &bValue, (ULONG)sizeof(bValue), REG_DWORD);
}

/****************************************************************************
**  Purpose:	See "WriteValue" for explination
****************************************************************************/
BOOL CNtRegistry::WriteInt(CString csKey, CString csName, int nValue)
{
	return WriteValue(csKey, csName, &nValue, (ULONG)sizeof(nValue), REG_DWORD);
}

/****************************************************************************
**  Purpose:	See "WriteValue" for explination
****************************************************************************/
BOOL CNtRegistry::WriteDword(CString csKey, CString csName, DWORD dwValue)
{
	return WriteValue(csKey, csName, &dwValue, (ULONG)sizeof(dwValue), REG_DWORD);
}

/****************************************************************************
**  Purpose:	See "WriteValue" for explination
****************************************************************************/
BOOL CNtRegistry::WriteDateTime(CString csKey, CString csName, COleDateTime dtValue)
{
	return WriteValue(csKey, csName, &dtValue, (ULONG)sizeof(dtValue), REG_BINARY);
}

/****************************************************************************
**  Purpose:	See "WriteValue" for explination
****************************************************************************/
BOOL CNtRegistry::WriteFloat(CString csKey, CString csName, double fValue)
{
	return WriteValue(csKey, csName, &fValue, (ULONG)sizeof(fValue), REG_BINARY);
}

/****************************************************************************
**  Purpose:	See "WriteValue" for explination
****************************************************************************/
BOOL CNtRegistry::WriteColor(CString csKey, CString csName, COLORREF crValue)
{
	return WriteValue(csKey, csName, &crValue, (ULONG)sizeof(crValue), REG_BINARY);
}

/****************************************************************************
**  See "WriteValue" for explination
****************************************************************************/
BOOL CNtRegistry::WriteFont(CString csKey, CString csName, CFont* pFont)
{
	return WriteValue(csKey, csName, pFont, (ULONG)sizeof(pFont), REG_BINARY);
}

/****************************************************************************
**  See "WriteValue" for explination
****************************************************************************/
BOOL CNtRegistry::WritePoint(CString csKey, CString csName, CPoint* pPoint)
{
	return WriteValue(csKey, csName, pPoint, (ULONG)sizeof(pPoint), REG_BINARY);
}

/****************************************************************************
**  See "WriteValue" for explination
****************************************************************************/
BOOL CNtRegistry::WriteSize(CString csKey, CString csName, CSize* pSize)
{
	return WriteValue(csKey, csName, pSize, (ULONG)sizeof(CSize), REG_BINARY);
}

/****************************************************************************
**  See "WriteValue" for explination
****************************************************************************/
BOOL CNtRegistry::WriteRect(CString csKey, CString csName, CRect* pRect)
{
	return WriteValue(csKey, csName, pRect, (ULONG)sizeof(pRect), REG_BINARY);
}

/****************************************************************************
**
**	Function:	WriteValue
**
**  Purpose:	Call this function to write entries in the registry.
**
**  Arguments:	(IN) - Name of the value.
**				(IN) - Value to be written.
**				(IN) - Value size.
**				(IN) - Registry Type value.
**
**  Returns:	BOOL - Success/Failure.
**
****************************************************************************/
BOOL CNtRegistry::WriteValue(CString csKey, CString csName, PVOID pValue, ULONG ulValueLength, DWORD dwRegType)
{
	USES_CONVERSION;

	// Make sure the "Key" is set...
	SetPathVars(csKey);

	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	//
	HANDLE hKey = NULL;
	m_NtStatus = NtOpenKey(&hKey, KEY_ALL_ACCESS, &ObjectAttributes);
	if (!NT_SUCCESS(m_NtStatus)) {
		return FALSE;
	}

	if (csName == _T("(Default)")) {
		csName = _T("");
	}

	ANSI_STRING asName;
	RtlZeroMemory(&asName, sizeof(asName));
	RtlInitAnsiString(&asName, W2A(csName));

	UNICODE_STRING ValueName;
	RtlZeroMemory(&ValueName, sizeof(ValueName));

	RtlAnsiStringToUnicodeString(&ValueName, &asName, TRUE);

	//
	// I do this (new and delete []) because I was getting
	// a lot of extra characters in the data (in the registry).
	//
	UCHAR *puc = new UCHAR[ulValueLength];
	memset(puc, 0, ulValueLength);
	memcpy(puc, pValue, ulValueLength);

	m_NtStatus = NtSetValueKey(hKey,
		&ValueName,
		0,
		dwRegType,		// i.e. REG_BINARY
		puc,
		ulValueLength);

	// Clean up...
	delete[] puc;

	if (!NT_SUCCESS(m_NtStatus)) {
		NtClose(hKey);
		return FALSE;
	}

	if (!m_bLazyWrite) {
		NtFlushKey(hKey);
	}

	NtClose(hKey);

	return TRUE;
}

int CNtRegistry::IsFile(CString csName)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind;

	hFind = FindFirstFile(csName, &wfd);
	if (hFind == INVALID_HANDLE_VALUE) {
		return -1;
	}

	// If NOT a directory...
	if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
		// This is a MUST
		FindClose(hFind);
		return 1;
	}

	// This is a MUST
	FindClose(hFind);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
void* CNtRegistry::GetAdministrorMemberGrpSid(void)
{
	SID_IDENTIFIER_AUTHORITY ntauth = SECURITY_NT_AUTHORITY;
	void* psid = NULL;
	__try
	{
		if (!AllocateAndInitializeSid(&ntauth, 2,
			SECURITY_BUILTIN_DOMAIN_RID,
			DOMAIN_ALIAS_RID_ADMINS,
			0, 0, 0, 0, 0, 0, &psid))
			return NULL;
	}
	__finally
	{
	}
	return psid;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CNtRegistry::IsAdministrorMember(void)
{
	BOOL bIsAdmin = FALSE;
	HANDLE htok = 0;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &htok)) {
		MessageBox(NULL, _T("IsAdminstratorMember OpenProcessToken Failed"), L"", MB_OK);
		return FALSE;
	}

	DWORD cb = 0;
	TOKEN_GROUPS* ptg = NULL;
	void* pAdminSid = NULL;
	SID_AND_ATTRIBUTES* it = NULL;
	SID_AND_ATTRIBUTES* end = NULL;

	BOOL bSuccess = FALSE;
	__try {
		//
		GetTokenInformation(htok, TokenGroups, 0, 0, &cb);
		ptg = (TOKEN_GROUPS*)LocalAlloc(LPTR, cb);

		if (!GetTokenInformation(htok, TokenGroups, ptg, cb, &cb)) {
			__leave;
		}

		pAdminSid = GetAdministrorMemberGrpSid();
		end = ptg->Groups + ptg->GroupCount;
		for (it = ptg->Groups; end != it; ++it) {
			if (EqualSid(it->Sid, pAdminSid)) {
				break;
			}
		}
		bIsAdmin = end != it;
		FreeSid(pAdminSid);
		pAdminSid = NULL;

		LocalFree(ptg);
		ptg = NULL;

		CloseHandle(htok);
		htok = NULL;
		bSuccess = TRUE;
	}
	__finally
	{
		if (pAdminSid)	{ FreeSid(pAdminSid); }
		if (ptg)			{ LocalFree(ptg); }
		if (htok)		{ CloseHandle(htok); }
	}
	return bSuccess && bIsAdmin;
}

///////////////////////////////////////////////////////////////////////////////
// IsWinXP 
///////////////////////////////////////////////////////////////////////////////
BOOL CNtRegistry::IsWinXP(void)
{

	return IsWindowsVistaOrGreater();

#if 0
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi))) {
		//
		// If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO *)&osvi)) {
			return FALSE; //err
		}
	}

	return (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// IsW2KorBetter 
///////////////////////////////////////////////////////////////////////////////
BOOL CNtRegistry::IsW2KorBetter(void)
{
	return IsWindowsVersionOrGreater(5, 0, 0);
#if 0
	OSVERSIONINFO OSVersionInfo;
	OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&OSVersionInfo)) {
		return ((OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
			&& (OSVersionInfo.dwMajorVersion >= 5));
	}
	return FALSE;
#endif
}

void CNtRegistry::ShowPermissionsDlg(HWND hwnd)
{
	//
	if (hwnd == NULL) {
		return;
	}

	// Maintains information about the object whose security we are editing
	ObjInf info;// = { 0 };

	// Fill the rest of the info structure
	FillKeyInfo(&info);

	//
	EnablePrivilege(SE_SECURITY_NAME, TRUE);

	// Create instance of class derived from interface ISecurityInformation 
	CPermissionsSecurity* pSec = new CPermissionsSecurity(&info);

	// Common dialog box for ACL editing
	EditSecurity(hwnd, pSec);
	if (pSec != NULL) {
		pSec->Release();
	}
}

void CNtRegistry::FillKeyInfo(ObjInf* pInfo)
{
	//
	//	HKEY_USERS			= USERS
	//	HKEY_CURRENT_USER	= CURRENT_USER
	//	HKEY_LOCAL_MACHINE	= MACHINE
	//	HKEY_CLASSES_ROOT	= CLASSES_ROOT
	//	HKEY_CURRENT_CONFIG	= MACHINE\SYSTEM\CurrentControlSet\Hardware Profiles\Current
	//
	CString csKeyName = _T("");
	HKEY hkey = GetRootKey();

	pInfo->m_hHandle = (HANDLE)hkey;

	if (hkey == HKEY_LOCAL_MACHINE) {
		lstrcpy(pInfo->m_szParentName, _T("MACHINE"));
		csKeyName += _T("MACHINE");
		csKeyName += m_csCurrentPath.Mid(m_csRootPath.GetLength());
	}
	else if (hkey == HKEY_USERS) {
		lstrcpy(pInfo->m_szParentName, _T("USERS"));
		csKeyName += _T("USERS");
		csKeyName += m_csCurrentPath.Mid(m_csRootPath.GetLength());
	}
	else if (hkey == HKEY_CURRENT_USER) {
		lstrcpy(pInfo->m_szParentName, _T("CURRENT_USER"));
		csKeyName += _T("CURRENT_USER");
		csKeyName += m_csCurrentPath.Mid(m_csRootPath.GetLength());
	}
	else if (hkey == HKEY_CLASSES_ROOT) {
		lstrcpy(pInfo->m_szParentName, _T("CLASSES_ROOT"));
		csKeyName += _T("CLASSES_ROOT");
		csKeyName += m_csCurrentPath.Mid(m_csRootPath.GetLength());
	}
	else if (hkey == HKEY_CURRENT_CONFIG) {
		lstrcpy(pInfo->m_szParentName, _T("MACHINE"));
		csKeyName += _T("MACHINE");
		csKeyName += m_csCurrentPath.Mid(18);
	}

	int nSlash = 0;
	for (int n = m_csCurrentPath.GetLength() - 1; n >= 0; n--) {
		//
		if (m_csCurrentPath[n] == '\\') {
			//
			nSlash = n;
			break;
		}
	}
	lstrcpy(pInfo->m_szName, csKeyName);

	// Copy the object's name into the info block for building the title text
	lstrcpy(pInfo->m_szObjectName, m_csCurrentPath.Mid(nSlash + 1));

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

BOOL CNtRegistry::SaveKey(CString csKey, CString csHiveFile)
{
	return SaveRestoreKey(csKey, csHiveFile, TRUE);
}

BOOL CNtRegistry::RestoreKey(CString csKey, CString csHiveFile)
{
	return SaveRestoreKey(csKey, csHiveFile, FALSE);
}

//////////////////////////////////////////////////////////////////////
//
//	Function:	SaveRestoreKey
//
//  Purpose:	Used to save/restore registry keys and values to/from a
//				Hive File.
//
//  Arguments:	(IN)  CString	- Root key to save/restore.
//				(IN)  CString	- Hive file to save/restore to/from.
//				(IN)  BOOL		- Which function to perform.
//
//	Privilege:	SE_BACKUP_NAME  (save)
//	Privilege:	SE_RESTORE_NAME (restore)
//
//  Returns:	BOOL - Success/Failure.
//				
//////////////////////////////////////////////////////////////////////
BOOL CNtRegistry::SaveRestoreKey(CString csKey, CString csHiveFile, BOOL bSaveKey)
{
	USES_CONVERSION;

	ASSERT(csHiveFile != _T(""));

	HANDLE hFile = NULL;
	HANDLE hKey = NULL;

	BOOL bSuccess = TRUE;

	// Enable the restore privilege
	if (bSaveKey) {
		m_NtStatus = EnablePrivilege(SE_BACKUP_NAME, TRUE);
	}
	else {
		m_NtStatus = EnablePrivilege(SE_RESTORE_NAME, TRUE);
	}
	// Check the results
	if (!NT_SUCCESS(m_NtStatus)) {
		bSuccess = FALSE;
		goto end_it;
	}

	//
	if (csHiveFile.Left(4) != _T("\\??\\")) {
		csHiveFile.Insert(0, _T("\\??\\"));
	}

	ANSI_STRING asFile;
	RtlZeroMemory(&asFile, sizeof(asFile));
	RtlInitAnsiString(&asFile, W2A(csHiveFile));

	UNICODE_STRING usFileName;
	RtlZeroMemory(&usFileName, sizeof(usFileName));

	RtlAnsiStringToUnicodeString(&usFileName, &asFile, TRUE);

	OBJECT_ATTRIBUTES FileObjectAttributes;
	InitializeObjectAttributes(&FileObjectAttributes, &usFileName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	IO_STATUS_BLOCK ioSB;
	if (bSaveKey) {
		// Open the requested file, exercising the backup privilege
		m_NtStatus = NtCreateFile(&hFile,
			GENERIC_WRITE,
			&FileObjectAttributes,
			&ioSB,
			0,
			FILE_ATTRIBUTE_ARCHIVE,
			FILE_SHARE_READ,
			FILE_OVERWRITE_IF,
			FILE_NON_DIRECTORY_FILE,
			NULL, 0);
	}
	else {
		// Open the requested file, exercising the restore privilege
		m_NtStatus = NtCreateFile(&hFile,
			GENERIC_READ,
			&FileObjectAttributes,
			&ioSB,
			0,
			FILE_ATTRIBUTE_ARCHIVE,
			FILE_SHARE_READ,
			FILE_OPEN,
			FILE_NON_DIRECTORY_FILE,
			NULL, 0);
	}
	// Check the results
	if (!NT_SUCCESS(m_NtStatus)) {
		bSuccess = FALSE;
		goto end_it;
	}

	// Make sure the "Key" is set...
	SetPathVars(csKey);

	//
	OBJECT_ATTRIBUTES ObjectAttributes;
	InitializeObjectAttributes(&ObjectAttributes, &m_usKeyName, OBJ_CASE_INSENSITIVE, m_hMachineReg, NULL);

	m_dwDisposition = 0;

	// if the key doesn't exist, create it
	if (bSaveKey) {
		m_NtStatus = NtCreateKey(&hKey,
			KEY_ALL_ACCESS,
			&ObjectAttributes,
			0,
			NULL,
			REG_OPTION_BACKUP_RESTORE,
			&m_dwDisposition);
	}
	else {
		m_NtStatus = NtCreateKey(&hKey,
			KEY_WRITE,
			&ObjectAttributes,
			0,
			NULL,
			REG_OPTION_BACKUP_RESTORE,
			&m_dwDisposition);
	}
	// Check the results
	if (!NT_SUCCESS(m_NtStatus)) {
		bSuccess = FALSE;
		goto end_it;
	}

	if (bSaveKey) {
		m_NtStatus = NtSaveKey(hKey, hFile);
	}
	else {
		m_NtStatus = NtRestoreKey(hKey, hFile, (ULONG)REG_FORCE_RESTORE);
	}
	// Check the results
	if (!NT_SUCCESS(m_NtStatus)) {
		bSuccess = FALSE;
	}

end_it:

	// Now remove the privilege (back to its prior state).
	// This is not strictly necessary, since we're going
	// to terminate the process anyway, and the token we've
	// adjusted will be destroyed, but in real life you might
	// have processes that do a little more than this :-)
	if (bSaveKey) {
		EnablePrivilege(SE_BACKUP_NAME, FALSE);
	}
	else {
		EnablePrivilege(SE_RESTORE_NAME, FALSE);
	}

	if (hFile) {
		NtClose(hFile);
	}

	if (hKey) {
		NtClose(hKey);
	}

	if (!bSuccess) {
		Output(DisplayError(m_NtStatus), MB_OK | MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}



////
//// NOTE:
//// KeyObjectAttributes->RootDirectory specifies the handle to the parent key and
//// KeyObjectAttributes->Name specifies the name of the key to load.
//// Flags can be 0 or REG_NO_LAZY_FLUSH.
//// 
//NTSTATUS STDCALL
//NtLoadKey2 (IN POBJECT_ATTRIBUTES KeyObjectAttributes,
//            IN POBJECT_ATTRIBUTES FileObjectAttributes,
//            IN ULONG Flags)
//{
//	POBJECT_NAME_INFORMATION NameInfo;
//	PUNICODE_STRING NamePointer;
//	PUCHAR Buffer;
//	ULONG BufferSize;
//	ULONG Length;
//	NTSTATUS Status;
//
//	//PAGED_CODE();
//
//	//DPRINT ("NtLoadKey2() called\n");
//
//#if 0
//	if (!SeSinglePrivilegeCheck (SeRestorePrivilege, ExGetPreviousMode ()))
//		return STATUS_PRIVILEGE_NOT_HELD;
//#endif
//
//	if (FileObjectAttributes->RootDirectory != NULL)
//	{
//		BufferSize = sizeof(OBJECT_NAME_INFORMATION) + MAX_PATH * sizeof(WCHAR);
//		Buffer = ExAllocatePool (NonPagedPool, BufferSize);
//		if (Buffer == NULL)
//			return STATUS_INSUFFICIENT_RESOURCES;
//
//		Status = ZwQueryObject (FileObjectAttributes->RootDirectory,
//								ObjectNameInformation,
//								Buffer,
//								BufferSize,
//								&Length);
//		if (!NT_SUCCESS(Status))
//		{
//			//DPRINT1 ("NtQueryObject() failed (Status %lx)\n", Status);
//			ExFreePool (Buffer);
//			return Status;
//		}
//
//		NameInfo = (POBJECT_NAME_INFORMATION)Buffer;
//		//DPRINT ("ObjectPath: '%wZ'  Length %hu\n",&NameInfo->Name, NameInfo->Name.Length);
//
//		NameInfo->Name.MaximumLength = MAX_PATH * sizeof(WCHAR);
//		if (FileObjectAttributes->ObjectName->Buffer[0] != L'\\')
//		{
//			RtlAppendUnicodeToString (&NameInfo->Name, L"\\");
//			//DPRINT ("ObjectPath: '%wZ'  Length %hu\n", &NameInfo->Name, NameInfo->Name.Length);
//		}
//		RtlAppendUnicodeStringToString (&NameInfo->Name, FileObjectAttributes->ObjectName);
//
//		//DPRINT ("ObjectPath: '%wZ'  Length %hu\n", &NameInfo->Name, NameInfo->Name.Length);
//		NamePointer = &NameInfo->Name;
//	}
//	else
//	{
//		if (FileObjectAttributes->ObjectName->Buffer[0] == L'\\')
//		{
//			Buffer = NULL;
//			NamePointer = FileObjectAttributes->ObjectName;
//		}
//		else
//		{
//			BufferSize = sizeof(OBJECT_NAME_INFORMATION) + MAX_PATH * sizeof(WCHAR);
//			Buffer = ExAllocatePool (NonPagedPool, BufferSize);
//			if (Buffer == NULL)
//				return STATUS_INSUFFICIENT_RESOURCES;
//
//			NameInfo = (POBJECT_NAME_INFORMATION)Buffer;
//			NameInfo->Name.MaximumLength = MAX_PATH * sizeof(WCHAR);
//			NameInfo->Name.Length = 0;
//			NameInfo->Name.Buffer = (PWSTR)((ULONG_PTR)Buffer + sizeof(OBJECT_NAME_INFORMATION));
//			NameInfo->Name.Buffer[0] = 0;
//
//			RtlAppendUnicodeToString (&NameInfo->Name, L"\\");
//			RtlAppendUnicodeStringToString (&NameInfo->Name, FileObjectAttributes->ObjectName);
//			NamePointer = &NameInfo->Name;
//		}
//	}
//
//	//DPRINT ("Full name: '%wZ'\n", NamePointer);
//
////	// Acquire hive lock
////	KeEnterCriticalRegion();
////	ExAcquireResourceExclusiveLite(&CmiRegistryLock, TRUE);
//
//	Status = NtLoadKey2 (KeyObjectAttributes, NamePointer, Flags);
//	if (!NT_SUCCESS (Status))
//	{
//		//DPRINT1 ("CmiLoadHive() failed (Status %lx)\n", Status);
//	}
//
////	// Release hive lock
////	ExReleaseResourceLite(&CmiRegistryLock);
////	KeLeaveCriticalRegion();
//
//	if (Buffer != NULL)
//		ExFreePool (Buffer);
//
//	return Status;
//}



// NTSTATUS STDCALL
// NtSetInformationKey (IN HANDLE KeyHandle,
//                      IN KEY_SET_INFORMATION_CLASS KeyInformationClass,
//                      IN PVOID KeyInformation,
//                      IN ULONG KeyInformationLength)
// {
//   PKEY_OBJECT KeyObject;
//   NTSTATUS Status;
//   REG_SET_INFORMATION_KEY_INFORMATION SetInformationKeyInfo;
//   REG_POST_OPERATION_INFORMATION PostOperationInfo;
// 
//   PAGED_CODE();
// 
//   // Verify that the handle is valid and is a registry key
//   Status = ObReferenceObjectByHandle (KeyHandle,
//                                       KEY_SET_VALUE,
//                                       CmiKeyType,
//                                       ExGetPreviousMode(),
//                                       (PVOID *)&KeyObject,
//                                       NULL);
//   if (!NT_SUCCESS (Status))
//     {
//       DPRINT ("ObReferenceObjectByHandle() failed with status %x\n", Status);
//       return Status;
//     }
// 
//   PostOperationInfo.Object = (PVOID)KeyObject;
//   SetInformationKeyInfo.Object = (PVOID)KeyObject;
//   SetInformationKeyInfo.KeySetInformationClass = KeyInformationClass;
//   SetInformationKeyInfo.KeySetInformation = KeyInformation;
//   SetInformationKeyInfo.KeySetInformationLength = KeyInformationLength;
// 
//   Status = CmiCallRegisteredCallbacks(RegNtSetInformationKey, &SetInformationKeyInfo);
//   if (!NT_SUCCESS(Status))
//     {
//       ObDereferenceObject (KeyObject);
//       return Status;
//     }
// 
//   if (KeyInformationClass != KeyWriteTimeInformation)
//     {
//       Status = STATUS_INVALID_INFO_CLASS;
//     }
// 
//   else if (KeyInformationLength != sizeof (KEY_WRITE_TIME_INFORMATION))
//     {
//       Status = STATUS_INFO_LENGTH_MISMATCH;
//     }
//   else
//     {
//       // Acquire hive lock
//       KeEnterCriticalRegion();
//       ExAcquireResourceExclusiveLite(&CmiRegistryLock, TRUE);
// 
//       VERIFY_KEY_OBJECT(KeyObject);
// 
//       KeyObject->KeyCell->LastWriteTime.QuadPart =
//         ((PKEY_WRITE_TIME_INFORMATION)KeyInformation)->LastWriteTime.QuadPart;
// 
//       CmiMarkBlockDirty (KeyObject->RegistryHive,
//                          KeyObject->KeyCellOffset);
// 
//       // Release hive lock
//       ExReleaseResourceLite(&CmiRegistryLock);
//       KeLeaveCriticalRegion();
//     }
// 
//   PostOperationInfo.Status = Status;
//   CmiCallRegisteredCallbacks(RegNtPostSetInformationKey, &PostOperationInfo);
// 
//   ObDereferenceObject (KeyObject);
// 
//   if (NT_SUCCESS(Status))
//     {
//       CmiSyncHives ();
//     }
// 
//   DPRINT ("NtSaveKey() done\n");
// 
//   return STATUS_SUCCESS;
// }
// 
// 
// //
// // NOTE:
// // KeyObjectAttributes->RootDirectory specifies the handle to the parent key and
// // KeyObjectAttributes->Name specifies the name of the key to unload.
// //
// NTSTATUS STDCALL
// NtUnloadKey (IN POBJECT_ATTRIBUTES KeyObjectAttributes)
// {
//   PREGISTRY_HIVE RegistryHive;
//   NTSTATUS Status;
// 
//   PAGED_CODE();
// 
//   DPRINT ("NtUnloadKey() called\n");
// 
// #if 0
//   if (!SeSinglePrivilegeCheck (SeRestorePrivilege, ExGetPreviousMode ()))
//     return STATUS_PRIVILEGE_NOT_HELD;
// #endif
// 
//   // Acquire registry lock exclusively
//   KeEnterCriticalRegion();
//   ExAcquireResourceExclusiveLite(&CmiRegistryLock, TRUE);
// 
//   Status = CmiDisconnectHive (KeyObjectAttributes,
//                               &RegistryHive);
//   if (!NT_SUCCESS (Status))
//     {
//       DPRINT1 ("CmiDisconnectHive() failed (Status %lx)\n", Status);
//       ExReleaseResourceLite (&CmiRegistryLock);
//       KeLeaveCriticalRegion();
//       return Status;
//     }
// 
//   DPRINT ("RegistryHive %p\n", RegistryHive);
// 
// #if 0
//   // Flush hive
//   if (!IsNoFileHive (RegistryHive))
//     CmiFlushRegistryHive (RegistryHive);
// #endif
// 
//   CmiRemoveRegistryHive (RegistryHive);
// 
//   // Release registry lock
//   ExReleaseResourceLite (&CmiRegistryLock);
//   KeLeaveCriticalRegion();
// 
//   DPRINT ("NtUnloadKey() done\n");
// 
//   return STATUS_SUCCESS;
// }
// 
// 
// NTSTATUS STDCALL
// NtInitializeRegistry (IN BOOLEAN SetUpBoot)
// {
//   NTSTATUS Status;
// 
//   PAGED_CODE();
// 
//   if (CmiRegistryInitialized == TRUE)
//     return STATUS_ACCESS_DENIED;
// 
//   // Save boot log file
//   IopSaveBootLogToFile();
// 
//   Status = CmiInitHives (SetUpBoot);
// 
//   CmiRegistryInitialized = TRUE;
//
//  return Status;
//}

