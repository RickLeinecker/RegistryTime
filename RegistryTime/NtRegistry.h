//////////////////////////////////////////////////////////////////////
//
//  CNtRegistry.h: Declaration of the CNtRegistry class.
//
//////////////////////////////////////////////////////////////////////
//
// File           : NtRegistry.h
// Version        : 0.0.0.37
// Function       : Header file of the NT Native Registry API classes.
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
// Modifications  :
//
// Rev 0.0.0.37	August 10, 2006	Daniel Madden
//	- Added recursive parameter to the CopyKey function.
//	- Fixed "DeleteKeyRecursive" function.
//	- Fixed "Refrshing" function.
//
// Rev 0.0.0.36	July 16, 2006	Daniel Madden
//	- Added Key "Permission" common dialog.
//	- Added "UserPermission" "Privs & Rights" Functions.
//	- Added Key path in the Statusbar.
//
// Rev 0.0.0.35	July 2, 2006	Daniel Madden
//	- Changed the Parameters of "CopyKeys/CopyValues" functions
//		- This allows it easier to copy anything/anywhere
//	- Changed the Parameters of "FindHiddenKeys" function
//		- This is so the output goes to a CStringArray (instead of a
//		  MessageBox) which allowed me to display the output in
//		  a ListCtrl for display (thanks to a suggestion from 
//		  "Tcpip2005" from CodeProject)!!
//	- Added "InitNtRegistry()" function which does all initialization.
//	- Added "CaseSensitive" param to the "Search()" function
//	- Added "#pragma comment(linker...)" to stdafx.h to show XP Themes
//	- Added some string functions "Rtl...()"
//		- RtlInitString
//		- RtlInitAnsiString
//		- RtlInitUnicodeString
//		- RtlAnsiStringToUnicodeString
//		- RtlUnicodeStringToAnsiString
//		- RtlFreeString
//		- RtlFreeAnsiString
//		- RtlFreeUnicodeString
//
// Rev 0.0.0.34	Jun 24, 2006	Daniel Madden
//	- Added "RenameKey()" that uses the "NtRenameKey()"
//	- Added "RenameValue()" that uses a home bread functions
//	- Reformated the Header & Source so that the order of the
//    functions in the header match the order in the source.
//
// Rev 0.0.0.33	Jun 22, 2006	Daniel Madden
//	- Combined the "SetRootKey" and "SetKey" functions
//	- Added "GetCurrentUsersTextualSid()" that returns the private var "m_csSID"
//
// Rev 0.0.0.32	Jun 11, 2006	Daniel Madden
//	- Re-Activated my interest in this
//	- Did a lot of "Clean-up"
//
// Rev 0.0.0.31	Aug 1, 2004	Daniel Madden
//	- Added the "NtOpenThread"
//
// Rev 0.0.0.3	Jun 15, 2004	Daniel Madden
//	- Added some Registry Hive file functionality (SaveKey & RestoreKey)
//	- Incorporated Mark Russinovich's ntdll.h file into the class
//	  so I wouldn't have to write separate header files
// 
// Rev 0.0.0.2	Jun 8, 2004	Daniel Madden
//	- DeleteKeysRecursive fix (thanks to John P. Scrimsher)
//	- All comments added to header 
//
// Rev 0.0.0.1	Jun 3, 2004	Daniel Madden
//	- Initial creation
// 
//
// Copyright © 2004-2006 Daniel Madden, Sr.
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

#pragma once

#include <Atlconv.H>

// My own string tokenizer :-)
#include "TokenEx.h"
#include <VersionHelpers.h>

#pragma warning(disable:4100) // unreferenced formal parameter
#pragma warning(disable:4244) // conversion from 'size_t' to 'USHORT', possible loss

#ifndef __NTREGISTRY_H__
#define __NTREGISTRY_H__


typedef DWORD ULONG;
typedef WORD  USHORT, *USHORT_PTR;
typedef ULONG NTSTATUS, *PNTSTATUS;

typedef ULONG ACCESS_MASK, *PACCESS_MASK;


// this is taken from "ntddk.h" header file
//
// Define the create disposition values
//
#define FILE_SUPERSEDE					0x00000000
#define FILE_OPEN						0x00000001
#define FILE_CREATE						0x00000002
#define FILE_OPEN_IF					0x00000003
#define FILE_OVERWRITE					0x00000004
#define FILE_OVERWRITE_IF				0x00000005
#define FILE_MAXIMUM_DISPOSITION		0x00000005

//  Function failed during execution.
//
#define ERROR_FUNCTION_FAILED			1627L

//
// Define the create/open option flags
//
#define FILE_DIRECTORY_FILE				0x00000001
#define FILE_WRITE_THROUGH				0x00000002
#define FILE_SEQUENTIAL_ONLY			0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING	0x00000008

#define FILE_SYNCHRONOUS_IO_ALERT		0x00000010
#define FILE_SYNCHRONOUS_IO_NONALERT	0x00000020
#define FILE_NON_DIRECTORY_FILE			0x00000040
#define FILE_CREATE_TREE_CONNECTION		0x00000080

#define FILE_COMPLETE_IF_OPLOCKED		0x00000100
#define FILE_NO_EA_KNOWLEDGE			0x00000200
#define FILE_OPEN_FOR_RECOVERY			0x00000400
#define FILE_RANDOM_ACCESS				0x00000800

#define FILE_DELETE_ON_CLOSE			0x00001000
#define FILE_OPEN_BY_FILE_ID			0x00002000
#define FILE_OPEN_FOR_BACKUP_INTENT		0x00004000
#define FILE_NO_COMPRESSION				0x00008000

#define FILE_RESERVE_OPFILTER			0x00100000
#define FILE_OPEN_REPARSE_POINT			0x00200000
#define FILE_OPEN_NO_RECALL				0x00400000
#define FILE_OPEN_FOR_FREE_SPACE_QUERY	0x00800000

#define FILE_COPY_STRUCTURED_STORAGE	0x00000041
#define FILE_STRUCTURED_STORAGE			0x00000441

#define FILE_VALID_OPTION_FLAGS			0x00ffffff
#define FILE_VALID_PIPE_OPTION_FLAGS	0x00000032
#define FILE_VALID_MAILSLOT_OPTION_FLAGS	0x00000032
#define FILE_VALID_SET_FLAGS			0x00000036

// this is taken from "NTStatus.h" header file
#define STATUS_SUCCESS				((NTSTATUS)0x00000000L) // ntsubauth
#define STATUS_BUFFER_OVERFLOW		((NTSTATUS)0x80000005L)
#define STATUS_INVALID_PARAMETER	((NTSTATUS)0xC000000DL)
#define STATUS_ACCESS_DENIED		((NTSTATUS)0xC0000022L)
#define STATUS_NO_MORE_ENTRIES		((NTSTATUS)0x8000001AL)
#define STATUS_OBJECT_TYPE_MISMATCH ((NTSTATUS)0xC0000024L)

// 
// I hate to type ;-)
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) == STATUS_SUCCESS)
#define HKU		HKEY_USERS
#define HKLM	HKEY_LOCAL_MACHINE
#define HKCU	HKEY_CURRENT_USER
#define HKCC	HKEY_CURRENT_CONFIG
#define HKCR	HKEY_CLASSES_ROOT

typedef struct _UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;

// InitializeUnicodeStrings (WCHAR wstr, BOOL hidden, UNICODE_STRING* us);
#define InitializeUnicodeStrings( wstr, hidden, us ) {                                \
    (us)->Buffer = (PWSTR)wstr;                                                              \
	(us)->Length = (USHORT)(wcslen((const wchar_t *)wstr) * sizeof(WCHAR)) + (sizeof(WCHAR) * hidden); \
    (us)->MaximumLength = (us)->Length+4;                                             \
}

typedef struct _STRING
{
	USHORT Length;
	USHORT MaximumLength;
	//#ifdef MIDL_PASS
	//    [size_is(MaximumLength), length_is(Length) ]
	//#endif // MIDL_PASS
	PCHAR Buffer;
} STRING;
typedef STRING *PSTRING;
typedef STRING OEM_STRING;
typedef STRING *POEM_STRING;
typedef STRING ANSI_STRING;
typedef STRING *PANSI_STRING;

// =================================================================
// Key query structures
// =================================================================
typedef struct _KEY_BASIC_INFORMATION
{
	LARGE_INTEGER LastWriteTime;// The last time the key or any of its values changed.
	ULONG TitleIndex;			// Device and intermediate drivers should ignore this member.
	ULONG NameLength;			// The size in bytes of the following name, including the zero-terminating character.
	WCHAR Name[1];				// A zero-terminated Unicode string naming the key.
} KEY_BASIC_INFORMATION;
typedef KEY_BASIC_INFORMATION *PKEY_BASIC_INFORMATION;

typedef struct _KEY_FULL_INFORMATION
{
	LARGE_INTEGER LastWriteTime;// The last time the key or any of its values changed.
	ULONG TitleIndex;			// Device and intermediate drivers should ignore this member.
	ULONG ClassOffset;			// The offset from the start of this structure to the Class member.
	ULONG ClassLength;			// The number of bytes in the Class name.
	ULONG SubKeys;				// The number of subkeys for the key.
	ULONG MaxNameLen;			// The maximum length of any name for a subkey.
	ULONG MaxClassLen;			// The maximum length for a Class name.
	ULONG Values;				// The number of value entries.
	ULONG MaxValueNameLen;		// The maximum length of any value entry name.
	ULONG MaxValueDataLen;		// The maximum length of any value entry data field.
	WCHAR Class[1];				// A zero-terminated Unicode string naming the class of the key.
} KEY_FULL_INFORMATION;
typedef KEY_FULL_INFORMATION *PKEY_FULL_INFORMATION;

typedef struct _KEY_NODE_INFORMATION
{
	LARGE_INTEGER LastWriteTime;// The last time the key or any of its values changed.
	ULONG TitleIndex;			// Device and intermediate drivers should ignore this member.
	ULONG ClassOffset;			// The offset from the start of this structure to the Class member.
	ULONG ClassLength;			// The number of bytes in the Class name.
	ULONG NameLength;			// The size in bytes of the following name, including the zero-terminating character.
	WCHAR Name[1];				// A zero-terminated Unicode string naming the key.
} KEY_NODE_INFORMATION;
typedef KEY_NODE_INFORMATION *PKEY_NODE_INFORMATION;

// end_wdm
typedef struct _KEY_NAME_INFORMATION
{
	ULONG   NameLength;
	WCHAR   Name[1];            // Variable length string
} KEY_NAME_INFORMATION, *PKEY_NAME_INFORMATION;
typedef KEY_NAME_INFORMATION *PKEY_NAME_INFORMATION;

// begin_wdm
typedef enum _KEY_INFORMATION_CLASS
{
	KeyBasicInformation,
	KeyNodeInformation,
	KeyFullInformation
	// end_wdm
	,
	KeyNameInformation
	// begin_wdm
} KEY_INFORMATION_CLASS;

typedef struct _KEY_WRITE_TIME_INFORMATION
{
	LARGE_INTEGER LastWriteTime;
} KEY_WRITE_TIME_INFORMATION;
typedef KEY_WRITE_TIME_INFORMATION *PKEY_WRITE_TIME_INFORMATION;

typedef enum _KEY_SET_INFORMATION_CLASS
{
	KeyWriteTimeInformation
} KEY_SET_INFORMATION_CLASS;


// =================================================================
// DesiredAccess Flags
// =================================================================
// KEY_QUERY_VALUE			Value entries for the key can be read. 
// KEY_SET_VALUE			Value entries for the key can be written. 
// KEY_CREATE_SUB_KEY		Subkeys for the key can be created. 
// KEY_ENUMERATE_SUB_KEYS	All subkeys for the key can be read. 
// KEY_NOTIFY				This flag is irrelevant to device and intermediate drivers, 
//							and to other kernel-mode code. 
// KEY_CREATE_LINK			A symbolic link to the key can be created. This flag is 
//							irrelvant to device and intermediate drivers. 
// 
// KEY_QUERY_VALUE			(0x0001)
// KEY_SET_VALUE			(0x0002)
// KEY_CREATE_SUB_KEY		(0x0004)
// KEY_ENUMERATE_SUB_KEYS	(0x0008)
// KEY_NOTIFY				(0x0010)
// KEY_CREATE_LINK			(0x0020)
//
//
// =================================================================
// DesiredAccess to Key Values
// =================================================================
// KEY_READ				STANDARD_RIGHTS_READ, KEY_QUERY_VALUE, 
//						KEY_ENUMERATE_SUB_KEYS, and KEY_NOTIFY 
// KEY_WRITE			STANDARD_RIGHTS_WRITE, KEY_SET_VALUE, and KEY_CREATE_SUBKEY 
// KEY_EXECUTE			KEY_READ. This value is irrelevant to device and intermediate 
// 						drivers. 
// KEY_ALL_ACCESS		STANDARD_RIGHTS_ALL, KEY_QUERY_VALUE, KEY_SET_VALUE, 
// 						KEY_CREATE_SUB_KEY, KEY_ENUMERATE_SUBKEY, KEY_NOTIFY 
// 						and KEY_CREATE_LINK 
// 
// 
// =================================================================
// CreateOptions Values
// =================================================================
// REG_OPTION_NON_VOLATILE		Key is preserved when the system is rebooted. 
// REG_OPTION_VOLATILE			Key is not to be stored across boots. 
// REG_OPTION_CREATE_LINK		The created key is a symbolic link. This value is 
//								irrelevant to device and intermediate drivers. 
// REG_OPTION_BACKUP_RESTORE	Key is being opened or created with special privileges 
//								allowing backup/restore operations. This value is 
// 								irrelevant to device and intermediate drivers. 
// 
// REG_OPTION_NON_VOLATILE		(0x00000000L)
// REG_OPTION_VOLATILE			(0x00000001L)
// REG_OPTION_CREATE_LINK		(0x00000002L)
// REG_OPTION_BACKUP_RESTORE	(0x00000004L)
// 
// 
// =================================================================
// Disposition Values
// =================================================================
// REG_CREATED_NEW_KEY		A new key object was created. 
// REG_OPENED_EXISTING_KEY	An existing key object was opened. 
// 
// REG_CREATED_NEW_KEY		(0x00000001L)
// REG_OPENED_EXISTING_KEY	(0x00000002L)
//
//
// =================================================================
// Value entry query structures
// REG_XXX Type Value:
// =================================================================
// REG_BINARY			Binary data in any form 
// REG_DWORD			A 4-byte numerical value (32-bit number) 
// REG_DWORD_LITTLE_ENDIAN  A 4-byte numerical value whose least significant 
//						byte is at the lowest address 
// REG_QWORD			64-bit number. 
// REG_QWORD_LITTLE_ENDIAN	A 64-bit number in little-endian format. This is 
//						equivalent to REG_QWORD. 
// REG_DWORD_BIG_ENDIAN A 4-byte numerical value whose least significant byte 
//						is at the highest address 
// REG_EXPAND_SZ		A zero-terminated Unicode string, containing unexpanded 
//						references to environment variables, such as "%PATH%" 
// REG_LINK				A Unicode string naming a symbolic link. This type is 
//						irrelevant to device and intermediate drivers 
// REG_MULTI_SZ			An array of zero-terminated strings, terminated by another zero 
// REG_NONE				Data with no particular type 
// REG_SZ				A zero-terminated Unicode string 
// REG_RESOURCE_LIST	A device driver's list of hardware resources, used by the driver 
//						or one of the physical devices it controls, in the \ResourceMap tree 
// REG_RESOURCE_REQUIREMENTS_LIST	A device driver's list of possible hardware resources 
//						it or one of the physical devices it controls can use, from which 
//						the system writes a subset into the \ResourceMap tree 
// REG_FULL_RESOURCE_DESCRIPTOR		A list of hardware resources that a physical device 
//						is using, detected and written into the \HardwareDescription tree 
//						by the system 
//
// =================================================================
typedef struct _KEY_VALUE_BASIC_INFORMATION
{
	ULONG TitleIndex;	// Device and intermediate drivers should ignore this member.
	ULONG Type;			// The system-defined type for the registry value in the 
	// Data member (see the values above).
	ULONG NameLength;	// The size in bytes of the following value entry name, 
	// including the zero-terminating character.
	WCHAR Name[1];		// A zero-terminated Unicode string naming a value entry of 
	// the key.
} KEY_VALUE_BASIC_INFORMATION;
typedef KEY_VALUE_BASIC_INFORMATION *PKEY_VALUE_BASIC_INFORMATION;

typedef struct _KEY_VALUE_FULL_INFORMATION
{
	ULONG TitleIndex;	// Device and intermediate drivers should ignore this member.
	ULONG Type;			// The system-defined type for the registry value in the 
	// Data member (see the values above).
	ULONG DataOffset;	// The offset from the start of this structure to the data 
	// immediately following the Name string.
	ULONG DataLength;	// The number of bytes of registry information for the value 
	// entry identified by Name.
	ULONG NameLength;	// The size in bytes of the following value entry name, 
	// including the zero-terminating character.
	WCHAR Name[1];		// A zero-terminated Unicode string naming a value entry of 
	// the key.
	//	WCHAR Data[1];      // Variable size data not declared
} KEY_VALUE_FULL_INFORMATION;
typedef KEY_VALUE_FULL_INFORMATION *PKEY_VALUE_FULL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION
{
	ULONG TitleIndex;	// Device and intermediate drivers should ignore this member.
	ULONG Type;			// The system-defined type for the registry value in the 
	// Data member (see the values above).
	ULONG DataLength;	// The size in bytes of the Data member.
	UCHAR Data[1];		// A value entry of the key.
} KEY_VALUE_PARTIAL_INFORMATION;
typedef KEY_VALUE_PARTIAL_INFORMATION *PKEY_VALUE_PARTIAL_INFORMATION;

typedef struct _KEY_VALUE_ENTRY
{
	PUNICODE_STRING ValueName;
	ULONG           DataLength;
	ULONG           DataOffset;
	ULONG           Type;
} KEY_VALUE_ENTRY;
typedef KEY_VALUE_ENTRY *PKEY_VALUE_ENTRY;

typedef enum _KEY_VALUE_INFORMATION_CLASS
{
	KeyValueBasicInformation,
	KeyValueFullInformation,
	KeyValuePartialInformation,
} KEY_VALUE_INFORMATION_CLASS;

typedef struct _KEY_MULTIPLE_VALUE_INFORMATION
{
	PUNICODE_STRING	ValueName;
	ULONG			DataLength;
	ULONG			DataOffset;
	ULONG			Type;
} KEY_MULTIPLE_VALUE_INFORMATION;
typedef KEY_MULTIPLE_VALUE_INFORMATION *PKEY_MULTIPLE_VALUE_INFORMATION;

typedef struct _IO_STATUS_BLOCK
{
	union
	{
		NTSTATUS	Status;
		PVOID		Pointer;
	};
	ULONG_PTR	Information;
} IO_STATUS_BLOCK;
typedef IO_STATUS_BLOCK *PIO_STATUS_BLOCK;

typedef void (NTAPI *PIO_APC_ROUTINE)
(
IN PVOID ApcContext,
IN PIO_STATUS_BLOCK IoStatusBlock,
IN ULONG Reserved
);


//
// ClientId
//
typedef struct _CLIENT_ID
{
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID;
typedef CLIENT_ID *PCLIENT_ID;


// =================================================================
//
// Valid values for the Attributes field
//
// This handle can be inherited by child processes of the current process.
#define OBJ_INHERIT				0x00000002L

// This flag only applies to objects that are named within the Object Manager. 
// By default, such objects are deleted when all open handles to them are closed. 
// If this flag is specified, the object is not deleted when all open handles are 
// closed. Drivers can use ZwMakeTemporaryObject to delete permanent objects.
#define OBJ_PERMANENT			0x00000010L

// Only a single handle can be open for this object.
#define OBJ_EXCLUSIVE			0x00000020L

// If this flag is specified, a case-insensitive comparison is used when 
// matching the ObjectName parameter against the names of existing objects. 
// Otherwise, object names are compared using the default system settings.
#define OBJ_CASE_INSENSITIVE	0x00000040L

// If this flag is specified to a routine that creates objects, and that object 
// already exists then the routine should open that object. Otherwise, the routine 
// creating the object returns an NTSTATUS code of STATUS_OBJECT_NAME_COLLISION.
#define OBJ_OPENIF				0x00000080L

// Specifies that the handle can only be accessed in kernel mode.
#define OBJ_KERNEL_HANDLE		0x00000200L

// The routine opening the handle should enforce all access checks 
// for the object, even if the handle is being opened in kernel mode.
#define OBJ_FORCE_ACCESS_CHECK	0x00000400L

//
#define OBJ_VALID_ATTRIBUTES    0x000007F2L


typedef struct _OBJECT_ATTRIBUTES {
	ULONG Length;
	HANDLE RootDirectory;
	PUNICODE_STRING ObjectName;
	ULONG Attributes;
	PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
	PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );        \
    (p)->RootDirectory = r;                           \
    (p)->Attributes = a;                              \
    (p)->ObjectName = n;                              \
    (p)->SecurityDescriptor = s;                      \
    (p)->SecurityQualityOfService = NULL;             \
    }
//
// =================================================================


#define RtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))
#define RtlCopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
#define RtlMoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))


// =================================================================
//  NTDLL Entry Points
// =================================================================
/*
Mapping Native APIs to Win32 Registry functions

// Creates or opens a Registry key.
NtCreateKey				RegCreateKey

// Opens an existing Registry key.
NtOpenKey				RegOpenKey

// Deletes a Registry key.
NtDeleteKey				RegDeleteKey

// Deletes a value.
NtDeleteValueKey		RegDeleteValue

// Enumerates the subkeys of a key.
NtEnumerateKey			RegEnumKey, RegEnumKeyEx

// Enumerates the values within a key.
NtEnumerateValueKey		RegEnumValue

// Flushes changes back to the Registry on disk.
NtFlushKey				RegFlushKey

// Gets the Registry rolling. The single parameter to this
// specifies whether its a setup boot or a normal boot.
NtInitializeRegistry	NONE

// Allows a program to be notified of changes to a particular
// key or its subkeys.
NtNotifyChangeKey		RegNotifyChangeKeyValue

// Queries information about a key.
NtQueryKey				RegQueryKey

// Retrieves information about multiple specified values.
// This API was introduced in NT 4.0.
NtQueryMultiplValueKey	RegQueryMultipleValues

// Retrieves information about a specified value.
NtQueryValueKey			RegQueryValue, RegQueryValueEx

// Changes the backing file for a key and its subkeys.
// Used for backup/restore.
NtReplaceKey			RegReplaceKey

// Saves the contents of a key and subkey to a file.
NtSaveKey				RegSaveKey

// Loads the contents of a key from a specified file.
NtRestoreKey			RegRestoreKey

// Sets attributes of a key.
NtSetInformationKey		NONE

// Sets the data associated with a value.
NtSetValueKey			RegSetValue, RegSetValueEx

// Loads a hive file into the Registry.
NtLoadKey				RegLoadKey

// Introduced in NT 4.0. Allows for options on loading a hive.
NtLoadKey2				NONE

// Unloads a hive from the Registry.
NtUnloadKey				RegUnloadKey

// New to WinXP. Makes key storage adjacent.
NtCompactKeys			NONE

// New to WinXP. Performs in-place compaction of a hive.
NtCompressKey			NONE

// New to WinXP. Locks a registry key for modification.
NtLockRegistryKey		NONE

// New to WinXP. Renames a Registry key.
NtRenameKey				NONE
NtRenameKey(IN HANDLE KeyHandle, IN PUNICODE_STRING ReplacementName);

// New to WinXP. Saves the contents of a key and its subkeys to a file.
NtSaveKeyEx				RegSaveKeyEx

// New to WinXP. Unloads a hive from the Registry.
NtUnloadKeyEx			NONE

// New to Server 2K3. Loads a hive into the Registry.
NtLoadKeyEx				NONE

// New to Serer 2K3. Unloads a hive from the Registry.
NtUnloadKey2			NONE

// New to Server 2003. Returns the keys opened beneath a specified key.
NtQueryOpenSubKeysEx	NONE

*/


// =================================================================
//  RTL String Functions
// =================================================================

// RtlInitString
typedef NTSTATUS(STDAPICALLTYPE RTLINITSTRING)
(
IN OUT PSTRING DestinationString,
IN LPCSTR SourceString
);
//IN PCSZ
typedef RTLINITSTRING FAR * LPRTLINITSTRING;

// RtlInitAnsiString
typedef NTSTATUS(STDAPICALLTYPE RTLINITANSISTRING)
(
IN OUT PANSI_STRING DestinationString,
IN LPCSTR SourceString
);
typedef RTLINITANSISTRING FAR * LPRTLINITANSISTRING;

// RtlInitUnicodeString
typedef NTSTATUS(STDAPICALLTYPE RTLINITUNICODESTRING)
(
IN OUT PUNICODE_STRING DestinationString,
IN LPCWSTR SourceString
);
typedef RTLINITUNICODESTRING FAR * LPRTLINITUNICODESTRING;

// RtlAnsiStringToUnicodeString
typedef NTSTATUS(STDAPICALLTYPE RTLANSISTRINGTOUNICODESTRING)
(
IN OUT PUNICODE_STRING	DestinationString,
IN PANSI_STRING			SourceString,
IN BOOLEAN				AllocateDestinationString
);
typedef RTLANSISTRINGTOUNICODESTRING FAR * LPRTLANSISTRINGTOUNICODESTRING;

// RtlUnicodeStringToAnsiString
typedef NTSTATUS(STDAPICALLTYPE RTLUNICODESTRINGTOANSISTRING)
(
IN OUT PANSI_STRING		DestinationString,
IN PUNICODE_STRING		SourceString,
IN BOOLEAN				AllocateDestinationString
);
typedef RTLUNICODESTRINGTOANSISTRING FAR * LPRTLUNICODESTRINGTOANSISTRING;

// RtlFreeString
typedef NTSTATUS(STDAPICALLTYPE RTLFREESTRING)
(
IN PSTRING String
);
typedef RTLFREESTRING FAR * LPRTLFREESTRING;

// RtlFreeAnsiString
typedef NTSTATUS(STDAPICALLTYPE RTLFREEANSISTRING)
(
IN PANSI_STRING AnsiString
);
typedef RTLFREEANSISTRING FAR * LPRTLFREEANSISTRING;

// RtlFreeUnicodeString
typedef NTSTATUS(STDAPICALLTYPE RTLFREEUNICODESTRING)
(
IN PUNICODE_STRING UnicodeString
);
typedef RTLFREEUNICODESTRING FAR * LPRTLFREEUNICODESTRING;

//DWORD WINAPI RtlEqualUnicodeString(PUNICODE_STRING s1,PUNICODE_STRING s2,DWORD x);
//DWORD WINAPI RtlUpcaseUnicodeString(PUNICODE_STRING dest,PUNICODE_STRING src,BOOLEAN doalloc);
//NTSTATUS WINAPI RtlCompareUnicodeString(PUNICODE_STRING String1, PUNICODE_STRING String2, BOOLEAN CaseInSensitive);

// =================================================================
//  END - RTL String Functions
// =================================================================



// NtCreateKey
typedef NTSTATUS(STDAPICALLTYPE NTCREATEKEY)
(
IN HANDLE				KeyHandle,
IN ULONG				DesiredAccess,
IN POBJECT_ATTRIBUTES	ObjectAttributes,
IN ULONG				TitleIndex,
IN PUNICODE_STRING		Class,			/* optional*/
IN ULONG				CreateOptions,
OUT PULONG				Disposition		/* optional*/
);
typedef NTCREATEKEY FAR * LPNTCREATEKEY;


// NtOpenKey
typedef NTSTATUS(STDAPICALLTYPE NTOPENKEY)
(
IN HANDLE				KeyHandle,
IN ULONG				DesiredAccess,
IN POBJECT_ATTRIBUTES	ObjectAttributes
);
typedef NTOPENKEY FAR * LPNTOPENKEY;

// NtFlushKey
typedef NTSTATUS(STDAPICALLTYPE NTFLUSHKEY)
(
IN HANDLE KeyHandle
);
typedef NTFLUSHKEY FAR * LPNTFLUSHKEY;

// NtDeleteKey
typedef NTSTATUS(STDAPICALLTYPE NTDELETEKEY)
(
IN HANDLE KeyHandle
);
typedef NTDELETEKEY FAR * LPNTDELETEKEY;

// NtSetValueKey
typedef NTSTATUS(STDAPICALLTYPE NTSETVALUEKEY)
(
IN HANDLE			KeyHandle,
IN PUNICODE_STRING	ValueName,
IN ULONG			TitleIndex,			/* optional */
IN ULONG			Type,
IN PVOID			Data,
IN ULONG			DataSize
);
typedef NTSETVALUEKEY FAR * LPNTSETVALUEKEY;

// NtQueryValueKey
typedef NTSTATUS(STDAPICALLTYPE NTQUERYVALUEKEY)
(
// Is the handle, returned by a successful 
// call to NtCreateKey or NtOpenKey, of key 
// for which value entries are to be read.
IN HANDLE			KeyHandle,
IN PUNICODE_STRING	ValueName,
IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
OUT PVOID			KeyValueInformation,
IN ULONG			Length,
OUT PULONG			ResultLength
);
typedef NTQUERYVALUEKEY FAR * LPNTQUERYVALUEKEY;


// NtSetInformationKey
typedef NTSTATUS(STDAPICALLTYPE NTSETINFORMATIONKEY)
(
IN HANDLE	KeyHandle,
IN KEY_SET_INFORMATION_CLASS KeyInformationClass,
IN PVOID	KeyInformation,
IN ULONG	KeyInformationLength
);
typedef NTSETINFORMATIONKEY FAR * LPNTSETINFORMATIONKEY;

// NtQueryKey
typedef NTSTATUS(STDAPICALLTYPE NTQUERYKEY)
(
IN HANDLE	KeyHandle,
IN KEY_INFORMATION_CLASS KeyInformationClass,
OUT PVOID	KeyInformation,
IN ULONG	KeyInformationLength,
OUT PULONG	ResultLength
);
typedef NTQUERYKEY FAR * LPNTQUERYKEY;

// NtEnumerateKey
typedef NTSTATUS(STDAPICALLTYPE NTENUMERATEKEY)
(
IN HANDLE	KeyHandle,
IN ULONG	Index,
IN KEY_INFORMATION_CLASS KeyInformationClass,
OUT PVOID	KeyInformation,
IN ULONG	KeyInformationLength,
OUT PULONG	ResultLength
);
typedef NTENUMERATEKEY FAR * LPNTENUMERATEKEY;

// NtDeleteValueKey
typedef NTSTATUS(STDAPICALLTYPE NTDELETEVALUEKEY)
(
IN HANDLE			KeyHandle,
IN PUNICODE_STRING	ValueName
);
typedef NTDELETEVALUEKEY FAR * LPNTDELETEVALUEKEY;

// NtEnumerateValueKey
typedef NTSTATUS(STDAPICALLTYPE NTENUMERATEVALUEKEY)
(
IN HANDLE	KeyHandle,
IN ULONG	Index,
IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
OUT PVOID	KeyValueInformation,
IN ULONG	KeyValueInformationLength,
OUT PULONG	ResultLength
);
typedef NTENUMERATEVALUEKEY FAR * LPNTENUMERATEVALUEKEY;

// NtQueryMultipleValueKey
typedef NTSTATUS(STDAPICALLTYPE NTQUERYMULTIPLEVALUEKEY)
(
IN HANDLE		KeyHandle,
IN OUT PKEY_MULTIPLE_VALUE_INFORMATION ValuesList,
IN ULONG		NumberOfValues,
OUT PVOID		DataBuffer,
IN OUT ULONG	BufferLength,
OUT PULONG		RequiredLength			/* optional */
);
typedef NTQUERYMULTIPLEVALUEKEY FAR * LPNTQUERYMULTIPLEVALUEKEY;

// NtNotifyChangeKey
typedef NTSTATUS(STDAPICALLTYPE NTNOTIFYCHANGEKEY)
(
IN HANDLE				KeyHandle,
IN HANDLE				EventHandle,
IN PIO_APC_ROUTINE		ApcRoutine,
IN PVOID				ApcRoutineContext,
IN PIO_STATUS_BLOCK		IoStatusBlock,
IN ULONG				NotifyFilter,
IN BOOLEAN				WatchSubtree,
OUT PVOID				RegChangesDataBuffer,
IN ULONG				RegChangesDataBufferLength,
IN BOOLEAN				Asynchronous
);
typedef NTNOTIFYCHANGEKEY FAR * LPNTNOTIFYCHANGEKEY;

// NtRenameKey
typedef NTSTATUS(STDAPICALLTYPE NTRENAMEKEY)
(
IN HANDLE			KeyHandle,
IN PUNICODE_STRING	ReplacementName
);
typedef NTRENAMEKEY FAR * LPNTRENAMEKEY;


// =================================================================
//
// REG_FORCE_RESTORE		Windows 2000 and later: If specified, the restore 
//	(0x00000008L)			operation is executed even if open handles exist at or 
//							beneath the location in the registry hierarchy the hKey 
//							parameter points to. 
// REG_NO_LAZY_FLUSH		If specified, the key or hive specified by the hKey 
//	(0x00000004L)			parameter will not be lazy flushed, or flushed 
//							automatically and regularly after an interval of time. 
// REG_REFRESH_HIVE			If specified, the location of the hive the hKey parameter 
//	(0x00000002L)			points to will be restored to its state immediately 
//							following the last flush. The hive must not be lazy 
//							flushed (by calling RegRestoreKey with REG_NO_LAZY_FLUSH 
//							specified as the value of this parameter), the caller must 
//							have TCB privilege, and the handle the hKey parameter 
//							refers to must point to the root of the hive. 
// REG_WHOLE_HIVE_VOLATILE	If specified, a new, volatile (memory only) set of 
//	(0x00000001L)			registry information, or hive, is created. If 
//							REG_WHOLE_HIVE_VOLATILE is specified, the key identified 
//							by the hKey parameter must be either the HKEY_USERS or 
//							HKEY_LOCAL_MACHINE value.  
//
// =================================================================
//
// NtRestoreKey
typedef NTSTATUS(STDAPICALLTYPE NTRESTOREKEY)
(
IN HANDLE	KeyHandle,
IN HANDLE	FileHandle,
IN ULONG	RestoreOption
);
typedef NTRESTOREKEY FAR * LPNTRESTOREKEY;

// NtSaveKey
typedef NTSTATUS(STDAPICALLTYPE NTSAVEKEY)
(
IN HANDLE	KeyHandle,
IN HANDLE	FileHandle
);
typedef NTSAVEKEY FAR * LPNTSAVEKEY;

// NtLoadKey
typedef NTSTATUS(STDAPICALLTYPE NTLOADKEY)
(
IN POBJECT_ATTRIBUTES DestinationKeyName,	// - and HANDLE to root key.
//   Root can be \registry\machine 
//   or \registry\user. 
//   All other keys are invalid. 
IN POBJECT_ATTRIBUTES HiveFileName			// - Hive file path and name
);
typedef NTLOADKEY FAR * LPNTLOADKEY;

// NtLoadKey2
typedef NTSTATUS(STDAPICALLTYPE NTLOADKEY2)
(
IN POBJECT_ATTRIBUTES DestinationKeyName,
IN POBJECT_ATTRIBUTES HiveFileName,
IN ULONG Flags	// Flags can be 0x0000 or REG_NO_LAZY_FLUSH (0x0004)
);
typedef NTLOADKEY2 FAR * LPNTLOADKEY2;

// NtReplaceKey
typedef NTSTATUS(STDAPICALLTYPE NTREPLACEKEY)
(
IN POBJECT_ATTRIBUTES	NewHiveFileName,
IN HANDLE				KeyHandle,
IN POBJECT_ATTRIBUTES	BackupHiveFileName
);
typedef NTREPLACEKEY FAR * LPNTREPLACEKEY;

// NtUnloadKey
typedef NTSTATUS(STDAPICALLTYPE NTUNLOADKEY)
(
IN POBJECT_ATTRIBUTES	DestinationKeyName
);
typedef NTUNLOADKEY FAR * LPNTUNLOADKEY;

// =================================================================

// NtClose
typedef NTSTATUS(STDAPICALLTYPE NTCLOSE)
(
IN HANDLE KeyHandle
);
typedef NTCLOSE FAR * LPNTCLOSE;

// =================================================================

// NtCreateFile
typedef NTSTATUS(STDAPICALLTYPE NTCREATEFILE)
(
OUT PHANDLE             FileHandle,
IN ACCESS_MASK          DesiredAccess,
IN POBJECT_ATTRIBUTES   ObjectAttributes,
OUT PIO_STATUS_BLOCK    IoStatusBlock,
IN PLARGE_INTEGER       AllocationSize,		/* optional */
IN ULONG                FileAttributes,
IN ULONG                ShareAccess,
IN ULONG                CreateDisposition,
IN ULONG                CreateOptions,
IN PVOID                EaBuffer,			/* optional */
IN ULONG                EaLength
);
typedef NTCREATEFILE FAR * LPNTCREATEFILE;

// NtOpenThread
typedef NTSTATUS(STDAPICALLTYPE NTOPENTHREAD)
(
OUT PHANDLE				ThreadHandle,
IN ACCESS_MASK			DesiredAccess,
IN POBJECT_ATTRIBUTES	ObjectAttributes,
IN PCLIENT_ID			ClientId		/* optional*/
);
typedef NTOPENTHREAD FAR * LPNTOPENTHREAD;

// NtOpenProcessToken
typedef NTSTATUS(STDAPICALLTYPE NTOPENPROCESSTOKEN)
(
IN HANDLE               ProcessHandle,
IN ACCESS_MASK          DesiredAccess,
OUT PHANDLE             TokenHandle
);
typedef NTOPENPROCESSTOKEN FAR * LPNTOPENPROCESSTOKEN;

// NtAdjustPrivilegesToken
typedef NTSTATUS(STDAPICALLTYPE NTADJUSTPRIVILEGESTOKEN)
(
IN HANDLE               TokenHandle,
IN BOOLEAN              DisableAllPrivileges,
IN PTOKEN_PRIVILEGES    TokenPrivileges,
IN ULONG                PreviousPrivilegesLength,
OUT PTOKEN_PRIVILEGES   PreviousPrivileges,	/* optional */
OUT PULONG              RequiredLength		/* optional */
);
typedef NTADJUSTPRIVILEGESTOKEN FAR * LPNTADJUSTPRIVILEGESTOKEN;

// NtQueryInformationToken
typedef NTSTATUS(STDAPICALLTYPE NTQUERYINFORMATIONTOKEN)
(
IN HANDLE TokenHandle,
IN TOKEN_INFORMATION_CLASS TokenInformationClass,
OUT PVOID TokenInformation,
IN ULONG TokenInformationLength,
OUT PULONG ReturnLength
);
typedef NTQUERYINFORMATIONTOKEN FAR * LPNTQUERYINFORMATIONTOKEN;

// RtlAllocateHeap
typedef NTSTATUS(STDAPICALLTYPE RTLALLOCATEHEAP)
(
IN PVOID HeapHandle,
IN ULONG Flags,
IN ULONG Size
);
typedef RTLALLOCATEHEAP FAR * LPRTLALLOCATEHEAP;

// RtlFreeHeap
typedef NTSTATUS(STDAPICALLTYPE RTLFREEHEAP)
(
IN PVOID HeapHandle,
IN ULONG Flags,								/* optional */
IN PVOID MemoryPointer
);
typedef RTLFREEHEAP FAR * LPRTLFREEHEAP;


#define ACCOUNT_VIEW					0x00000001L
#define ACCOUNT_ADJUST_PRIVILEGES		0x00000002L
#define ACCOUNT_ADJUST_QUOTAS			0x00000004L
#define ACCOUNT_ADJUST_SYSTEM_ACCESS	0x00000008L

#define ACCOUNT_ALL_ACCESS  (ACCOUNT_VIEW|ACCOUNT_ADJUST_PRIVILEGES|ACCOUNT_ADJUST_QUOTAS|ACCOUNT_ADJUST_SYSTEM_ACCESS)

#define STATUS_OBJECT_NAME_NOT_FOUND    ((NTSTATUS)0xC0000034L)
#define STATUS_INVALID_SID              ((NTSTATUS)0xC0000078L)

// This define is so the compiler doesn't mix-up the
// NTSTATUS declarations
#define _NTDEF_

// used by the function that gets the DOMAIN\user
#include <lmcons.h>					// DNLEN, UNLEN

// iphlpapi.lib ws2_32.lib psapi.lib 
// #include <iphlpapi.h>
// #include <Psapi.h>

// Netapi32.lib Userenv.lib Kernel32.lib Advapi32.lib
#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "Userenv.lib")
#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "Advapi32.lib")

#include <Aclapi.h>
#include <Ntsecapi.h>
#include <Accctrl.h>
#include <Userenv.h>
#include <Tlhelp32.h>

#include "PermissionsSecurity.h"


#define TOKEN_ALLMOST_ACCESS	(STANDARD_RIGHTS_REQUIRED| \
								TOKEN_ASSIGN_PRIMARY| \
								TOKEN_DUPLICATE| \
								TOKEN_IMPERSONATE| \
								TOKEN_QUERY| \
								TOKEN_QUERY_SOURCE| \
								TOKEN_ADJUST_PRIVILEGES| \
								TOKEN_ADJUST_GROUPS| \
								TOKEN_ADJUST_DEFAULT)

#define MY_BUFSIZE 512  // highly unlikely to exceed 512 bytes


// LsaCreateAccount
typedef NTSTATUS(STDAPICALLTYPE LSACREATEACCOUNT)
(
IN LSA_HANDLE			PolicyHandle,
IN PSID					AccountSid,
IN ULONG				AccountAccess,
OUT PLSA_HANDLE			AccountHandle
);
typedef LSACREATEACCOUNT FAR * LPLSACREATEACCOUNT;

// LsaOpenAccount
typedef NTSTATUS(STDAPICALLTYPE LSAOPENACCOUNT)
(
IN LSA_HANDLE			PolicyHandle,
IN PSID					AccountSid,
IN ULONG				AccountAccess,
OUT PLSA_HANDLE			AccountHandle
);
typedef LSAOPENACCOUNT FAR * LPLSAOPENACCOUNT;

// LsaLookupPrivilegeValue
typedef NTSTATUS(STDAPICALLTYPE LSALOOKUPPRIVILEGEVALUE)
(
IN LSA_HANDLE			PolicyHandle,
OUT PLSA_UNICODE_STRING	PrivilegeName,
OUT PLUID				Luid
);
typedef LSALOOKUPPRIVILEGEVALUE FAR * LPLSALOOKUPPRIVILEGEVALUE;

// LsaGetSystemAccessAccount
typedef NTSTATUS(STDAPICALLTYPE LSAGETSYSTEMACCESSACCOUNT)
(
IN LSA_HANDLE			PolicyHandle,
IN PULONG				AccountAccess
);
typedef LSAGETSYSTEMACCESSACCOUNT FAR * LPLSAGETSYSTEMACCESSACCOUNT;


// LsaSetSystemAccessAccount
typedef NTSTATUS(STDAPICALLTYPE LSASETSYSTEMACCESSACCOUNT)
(
IN LSA_HANDLE			PolicyHandle,
IN ULONG				AccountAccess
);
typedef LSASETSYSTEMACCESSACCOUNT FAR * LPLSASETSYSTEMACCESSACCOUNT;


// LsaAddPrivilegesToAccount
typedef NTSTATUS(STDAPICALLTYPE LSAADDPRIVILEGESTOACCOUNT)
(
IN LSA_HANDLE			PolicyHandle,
IN PPRIVILEGE_SET		AccountPrivs
);
typedef LSAADDPRIVILEGESTOACCOUNT FAR * LPLSAADDPRIVILEGESTOACCOUNT;

// LsaRemovePrivilegesFromAccount
typedef NTSTATUS(STDAPICALLTYPE LSAREMOVEPRIVILEGESFROMACCOUNT)
(
IN LSA_HANDLE			PolicyHandle,
IN PPRIVILEGE_SET		AccountPrivs,
BOOLEAN					AllRights
);
typedef LSAREMOVEPRIVILEGESFROMACCOUNT FAR * LPLSAREMOVEPRIVILEGESFROMACCOUNT;





///////////////////////////////////////////////////////////////////
// Test Stuff...Playing, etc...
//
typedef struct _VALUE_DATA {
	char*	szName;			// Name (Char Format)
	int		nNameLen;		// Name Length (Char Format)
	WCHAR*	wszName;		// Name (Wide Char Format)
	USHORT	usNameLen;		// Name Length (Wide Char Format)
	DWORD	dwType;			// Type of Reg value
	UCHAR*	szData;			// Registry Value data
	USHORT	usDataLen;		// Registry Value data length
} VALUE_DATA;

typedef struct _KEY_DATA {
	HKEY	hKey;			// Current Registry HKEY
	CString	csRootKey;		// Root Key (i.e. \registry\user)
	CString	csSubKey;		// SubKey
	WCHAR	wszRootKey[256];// Root Key (Wide Char Format)
	USHORT	usRootLen;		// Root Key Length (Wide Char Format)
	WCHAR	wszSubKey[1024];// SubKey (Wide Char Format)
	USHORT	usSubKeyLen;	// SubKey Length (Wide Char Format)
	int		nNULLCount;		//
	VALUE_DATA*	pValueData;	// Pointer to the VALUE_DATA structure
} KEY_DATA;
//
// End - Test Stuff...
///////////////////////////////////////////////////////////////////



// NtQueryMultipleValueKey
// NtNotifyChangeKey
// =================================================================
//
//  The Class below was created to provide a means to manipulate the
//  Registry using the NT Native Registry APIs. The format of this
//  class (Look & Feel) is taken from the CRegistry class 
//  by Robert Pittenger. http://www.codeproject.com/system/registry.asp
//
//  NOTES:
//		HKEY_USERS			\Registry\User
//		HKEY_CURRENT_USER	\Registry\User\<SID_User>
//		HKEY_LOCAL_MACHINE	\Registry\Machine
//		HKEY_CLASSES_ROOT	\Registry\Machine\SOFTWARE\Classes
//		HKEY_CURRENT_CONFIG	\Registry\Machine\SYSTEM\CurrentControlSet\Hardware Profiles\Current
//
// 
// CNtRegistry never keeps a key open past the end of a function call.
// This is incase the application crashes before the next call to close
// the registry 
//
// =================================================================
class CNtRegistry
{

public:

	// Constructor/Destructor
	CNtRegistry();
	virtual ~CNtRegistry();


	// CNtRegistry Attributes	
public:

	BOOL		m_bHidden;	// Set if the current key is Hidden 8^)
	// This holds the "Current" Key broken up in pieces
	// Simply call "CStringArray& csaPath = m_tokenEx.GetCStringArray();"
	CTokenEx	m_tokenEx;

	//
	HANDLE		m_hMachineReg;
	CString		m_csMachineName;


protected:

	// UserMode = 0  -  Always this for now
	// KernelMode = 1
	DWORD		m_ntModeType;
	//
	NTSTATUS	m_NtStatus;			// return of the Nt(Registry) routines
	//
	HKEY		m_hRoot;			// Root key to the registry
	//
	DWORD		m_dwDisposition;	// Capture the Disposition of a registry key/value
	//
	CString		m_csSID;			// Users SID
	CString		m_csRootPath;		// Root path to the registry
	CString		m_csCurrentPath;	// Current path to the registry (including the RootPath)
	//
	WCHAR		m_wszPath[2048];	// Path to the registry (Wide Char Format)
	USHORT		m_usLength;			// Length of Path to the registry
	UNICODE_STRING m_usKeyName;		// U_S that contain all of the above
	//
	// Force a registry key/value to be committed to disk
	// by calling NtFlushKey.
	// NOTE:  This routine can flush the entire registry. 
	// Accordingly, it can generate a great deal of I/O. 
	// Since the system automatically flushes key changes 
	// every few seconds, it is seldom necessary.
	BOOL		m_bLazyWrite;

	// CNtRegistry Operations
public:

	////////////////////////////////////////////////////////
	// Nt Native API's
	//
	// NTDLL.dll Entry Points
	//
	LPNTCREATEKEY				NtCreateKey;
	LPNTOPENKEY					NtOpenKey;
	LPNTDELETEKEY				NtDeleteKey;
	LPNTFLUSHKEY				NtFlushKey;
	LPNTQUERYKEY				NtQueryKey;
	LPNTENUMERATEKEY			NtEnumerateKey;
	//
	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// Nt Value Functions
	////////////////////////////////////////////////////////
	//
	LPNTSETVALUEKEY				NtSetValueKey;
	LPNTSETINFORMATIONKEY		NtSetInformationKey;
	LPNTQUERYVALUEKEY			NtQueryValueKey;
	LPNTENUMERATEVALUEKEY		NtEnumerateValueKey;
	LPNTDELETEVALUEKEY			NtDeleteValueKey;
	LPNTQUERYMULTIPLEVALUEKEY	NtQueryMultipleValueKey;
	//
	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// Nt New Functions for WinXP and Serer 2K3
	////////////////////////////////////////////////////////
	//
	// (WinXP) Renames a Registry key.
	LPNTRENAMEKEY				NtRenameKey;
	// (WinXP) Makes key storage adjacent.
	//LPNTCOMPACTKEYS			NtCompactKeys
	// (WinXP) Performs in-place compaction of a hive.
	//LPNTCOMPRESSKEY			NtCompressKey
	// (WinXP) Locks a registry key for modification.
	//LPNTLOCKREGISTRYKEY		NtLockRegistryKey
	// (Server 2K3) Returns the keys opened beneath a specified key.
	//LPNTQUERYOPENSUBKEYSEX	NtQueryOpenSubKeysEx
	// (WinXP) Saves the contents of a key and its subkeys to a file.
	//LPNTSAVEKEYEX				NtSaveKeyEx;
	// (Server 2K3) Loads a hive into the Registry.
	//LPNTLOADKEYEX				NtLoadKeyEx;
	// (Server 2K3) Unloads a hive from the Registry.
	//LPNTUNLOADKEY2			NtUnloadKey2;
	// (WinXP) Unloads a hive from the Registry.
	//LPNTUNLOADKEYEX			NtUnloadKeyEx;
	//
	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// Nt Hive Functions
	////////////////////////////////////////////////////////
	//
	LPNTSAVEKEY					NtSaveKey;
	LPNTRESTOREKEY				NtRestoreKey;
	LPNTLOADKEY					NtLoadKey;
	LPNTLOADKEY2				NtLoadKey2;
	LPNTREPLACEKEY				NtReplaceKey;
	LPNTUNLOADKEY				NtUnloadKey;
	//
	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// Nt Misc Functions
	////////////////////////////////////////////////////////
	//
	LPNTCLOSE					NtClose;
	LPNTNOTIFYCHANGEKEY			NtNotifyChangeKey;
	LPNTOPENTHREAD				NtOpenThread;
	//
	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// Nt File Functions
	////////////////////////////////////////////////////////
	//
	LPNTCREATEFILE				NtCreateFile;
	//
	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// Nt Process Functions
	////////////////////////////////////////////////////////
	//
	LPNTOPENPROCESSTOKEN		NtOpenProcessToken;
	LPNTADJUSTPRIVILEGESTOKEN	NtAdjustPrivilegesToken;
	LPNTQUERYINFORMATIONTOKEN	NtQueryInformationToken;
	//
	////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////
	// END - Native API Functions
	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// Rtl String Functions
	////////////////////////////////////////////////////////
	//
	LPRTLINITSTRING					RtlInitString;
	LPRTLINITANSISTRING				RtlInitAnsiString;
	LPRTLINITUNICODESTRING			RtlInitUnicodeString;
	LPRTLANSISTRINGTOUNICODESTRING	RtlAnsiStringToUnicodeString;
	LPRTLUNICODESTRINGTOANSISTRING	RtlUnicodeStringToAnsiString;
	LPRTLFREESTRING					RtlFreeString;
	LPRTLFREEANSISTRING				RtlFreeAnsiString;
	LPRTLFREEUNICODESTRING			RtlFreeUnicodeString;
	//
	////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////
	// Nt Heap Functions
	////////////////////////////////////////////////////////
	//
	LPRTLALLOCATEHEAP			RtlAllocateHeap;
	LPRTLFREEHEAP				RtlFreeHeap;
	//
	////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////
	// Privileges/Rights/Access
	////////////////////////////////////////////////////////
	//
	//		This shows the Permissions/Access dialog box
	void	ShowPermissionsDlg(HWND hwnd);
	//		This fills the info for the Permissions/Access dialog box
	void	FillKeyInfo(ObjInf* pInfo);
	//
	volatile BOOL m_bSystemAccess;
	//
	BOOL	GetObjAcctDomainInfo(SE_OBJECT_TYPE seObjType, CString csObjName, CString& csObjAccount, CString& csObjDomain);
	//
	int		IsFile(CString csName);
	//
	void*	GetAdministrorMemberGrpSid(void);
	//
	BOOL	IsAdministrorMember(void);
	//		Get SID in Text format for the current user.
	BOOL	GetTextualSid(PSID pSid, LPWSTR szTextualSid, LPDWORD dwBufferLen);
	//		Lookup Security ID for the current user.
	BOOL	LookupSID(CString &csSID);
	//		 Enables/Disables Privileges for the current user
	NTSTATUS EnablePrivilege(CString csPrivilege, BOOL bEnable);
	//
	BOOL	IsWinXP(void);
	//
	BOOL	IsW2KorBetter(void);
	//
	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// CNtRegistry Helper Functions
	////////////////////////////////////////////////////////
	//
	//		Initializes application variables
	void	InitNtRegistry();
	//		Load the NTDLL.dll Entry Points.
	BOOL	LocateNTDLLEntryPoints(CString& csErr);
	//		Display message.(being lazy ;-)
	void	Output(CString csMsg, DWORD Buttons = MB_OK);
	//		Display NTSTATUS/System error message.
	CString DisplayError(DWORD dwError);
	//		Returns the "m_csCurrentPath" variable
	CString GetCurrentPath(void);
	//
	CString CheckRegFullPath(CString csPath);
	//		Sets the Path variables for the WCHAR Path and the Hidden variables.
	void	SetPathVars(CString csKey);
	//		Creates a new file
	BOOL	CreateNewFile(CString csFile);
	//
	////////////////////////////////////////////////////////
	// END - CNtRegistry Helper functions
	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// CNtRegistry Helper "inline" Functions
	////////////////////////////////////////////////////////
	//
	// Get Current Users SID.
	inline CString GetCurrentUsersTextualSid() { return m_csSID; }
	// Get the Disposition of the current key/value
	inline DWORD GetDisposition(void) { return m_dwDisposition; }
	// returns if the "m_csCurrentPath" is valid
	inline BOOL PathIsValid() { return (m_csCurrentPath.GetLength() > 0); }
	// Displays the last NTSTATUS Error message
	inline CString GetLastNtError() { return DisplayError(m_NtStatus); }
	// Returns the last NTSTATUS Error
	inline NTSTATUS GetLastNTErrorNum() { return m_NtStatus; }
	// Returns the "m_csRootPath" variable
	inline CString GetRootPath(void) const { return m_csRootPath; }
	//
	inline HKEY GetRootKey(void) { return m_hRoot; }
	//
	////////////////////////////////////////////////////////
	// END - CNtRegistry Helper "inline" functions
	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// CNtRegistry Key Functions
	////////////////////////////////////////////////////////
	//
	// Returns the RootPath for the HKEY given
	CString GetRootPathFor(HKEY hRoot) const;
	// Gets the Root HKEY variable in CString format
	CString GetRootKeyString(void) const;
	// Gets the Root HKEY variable in CString format
	CString GetShortRootKeyString(void) const;
	// Sets the "m_csRootPath" variable
	BOOL SetRootKey(HKEY hKey);
	// This calls "SetRootKey()" before calling "SetKet()"
	BOOL SetKey(HKEY hRoot, CString csKey, BOOL bCanCreate, BOOL bCanSaveCurrentKey);
	// This ensures the path is a full path to  the registry key... 
	// Meaning, it starts with "\registry\"
	BOOL SetKey(CString csKey, BOOL bCanCreate, BOOL bCanSaveCurrentKey);
	// Creates a registry key using NtCreateKey from the ntdll.dll
	BOOL CreateKey(CString csKey);
	// Creates a registry key using NtCreateKey from the ntdll.dll
	// just adds an extra NULL
	BOOL CreateHiddenKey(CString csKey);
	// Renames a registry key using NtRenameKey from the ntdll.dll
	// Win2000 doesn't have this capability, so if a W2K OS attempts
	// this, it uses the "Copy/Delete" functions to do this.
	BOOL RenameKey(CString csFullKey, CString csNewKeyName);
	// Deletes a registry key using NtDeleteKey from the ntdll.dll
	BOOL DeleteKey(CString csKey);
	// Deletes the key specified and "ALL" registry 
	// keys under it, for example, if you specify:
	// HKLM\SOFTWARE\MaddensApp
	// the "MaddensApp" key and all it's subkeys
	// will be deleted
	BOOL DeleteKeysRecursive(CString csKey);
	// Checks if the specified key exists in the registry
	BOOL KeyExists(CString csKey);
	// Returns the # of subkeys under the "Current" key
	int  GetSubKeyCount();
	// Adds the "Name" of subkeys under the "Current" key
	BOOL GetSubKeyList(CStringArray &csaSubkeys);
	// Checks if the key is a "Hidden" key
	BOOL IsKeyHidden(CString csKey);
	// Finds all the "Hidden" keys under the key specified.
	// If it finds one, it will show you a MessageBox
	BOOL FindHiddenKeys(CString csKey, BOOL bRecursive, CStringArray& csaResults);
	// Search for a specific string
	BOOL Search(CString csString, CString csStartKey, CStringArray& csaResults,
		int nRegSearchType = 3, BOOL bCaseSensitive = TRUE);
	//
	BOOL CopyKeys(CString csSource, CString csTarget, BOOL bRecursively);
	//
	////////////////////////////////////////////////////////
	// END - CNtRegistry Key functions
	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// CNtRegistry Value Functions
	////////////////////////////////////////////////////////
	//
	// Checks if the value name exists in the registry
	BOOL ValueExists(CString csName);
	// 
	BOOL RenameValue(CString csOldName, CString csNewName);
	//
	BOOL CopyValues(CString csSource, CString csTarget, CString csValueName, CString csNewValueName);
	// Deletes a registry value using NtDeleteValueKey from the ntdll.dll
	BOOL DeleteValue(CString csName);
	// Returns the "Data Type" and "Size" of the "Value Name"
	DWORD GetValueInfo(CString csValueName, int& nSize);
	// Returns # of value names under the "Current" key
	int GetValueCount();
	// Gets the "Value Name" of values under the "Current" key
	BOOL GetValueList(CStringArray &csaValues);
	//
	////////////////////////////////////////////////////////
	// END - CNtRegistry Value functions
	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// CNtRegistry Hive Functions (just a few)
	////////////////////////////////////////////////////////
	//
	//BOOL LoadKey(CString csHiveFilePathName, ULONG ulFlags = 0x0000);
	//BOOL UnLoadKey(CString csHiveFilePathName);
	//BOOL ReplaceKey(CString csNewHiveFile, CString csKey, CString csOldHiveFile);
	// Saves the specified key and all of its subkeys and 
	// values to the specified hive file.
	BOOL SaveKey(CString csKey, CString csHiveFile);
	// Reads the registry information in a specified hive file 
	// and copies it over the specified key. This registry 
	// information may be in the form of a key and multiple 
	// levels of subkeys.
	BOOL RestoreKey(CString csKey, CString csHiveFile);
	// This is a simple wrapper for the "SaveKey & RestoreKey" functions
	BOOL SaveRestoreKey(CString csKey, CString csHiveFile, BOOL bSaveKey);
	//
	////////////////////////////////////////////////////////
	// END - CNtRegistry Hive functions
	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// CNtRegistry data reading functions
	////////////////////////////////////////////////////////
	//
	// Returns the string (in binary form: 0a 01 de ff) associated 
	// with the specified name else, it returns the default 
	// specified
	UCHAR* ReadBinary(CString csName, UINT& uiLength) { return ReadBinary(GetCurrentPath(), csName, uiLength); }
	UCHAR* ReadBinary(CString csKey, CString csName, UINT& uiLength);
	// Returns the CString associated with the specified 
	// name else, it returns the default specified
	CString ReadString(CString csName, CString csDefault) { return ReadString(GetCurrentPath(), csName, csDefault); }
	CString ReadString(CString csKey, CString csName, CString csDefault);
	// References an CStringArray associated with the 
	// specified name else, it returns an empty array
	BOOL ReadMultiString(CString csName, CStringArray& csaReturn) { return ReadMultiString(GetCurrentPath(), csName, csaReturn); }
	BOOL ReadMultiString(CString csKey, CString csName, CStringArray& csaReturn);
	// Returns the DWORD associated with the specified 
	// name else, it returns the default specified
	DWORD ReadDword(CString csName, DWORD dwDefault) { return ReadDword(GetCurrentPath(), csName, dwDefault); }
	DWORD ReadDword(CString csKey, CString csName, DWORD dwDefault);
	// Returns the int associated with the specified 
	// name else, it returns the default specified
	int ReadInt(CString csName, int nDefault) { return ReadInt(GetCurrentPath(), csName, nDefault); }
	int ReadInt(CString csKey, CString csName, int nDefault);
	// Returns the BOOL associated with the specified 
	// name else, it returns the default specified
	BOOL ReadBool(CString csName, BOOL bDefault) { return ReadBool(GetCurrentPath(), csName, bDefault); }
	BOOL ReadBool(CString csKey, CString csName, BOOL bDefault);
	// Returns the COleDateTime associated with the specified 
	// name else, it returns the default specified
	COleDateTime ReadDateTime(CString csName, COleDateTime dtDefault) { return ReadDateTime(GetCurrentPath(), csName, dtDefault); }
	COleDateTime ReadDateTime(CString csKey, CString csName, COleDateTime dtDefault);
	// Returns the double associated with the specified 
	// name else, it returns the default specified
	double ReadFloat(CString csName, double fDefault) { return ReadFloat(GetCurrentPath(), csName, fDefault); }
	double ReadFloat(CString csKey, CString csName, double fDefault);
	// Returns the COLORREF associated with the specified 
	// name else, it returns the default specified
	COLORREF ReadColor(CString csName, COLORREF rgbDefault) { return ReadColor(GetCurrentPath(), csName, rgbDefault); }
	COLORREF ReadColor(CString csKey, CString csName, COLORREF rgbDefault);
	// Returns the CFont associated with the specified 
	// name else, it returns the default specified
	BOOL ReadFont(CString csName, CFont* pFont) { return ReadFont(GetCurrentPath(), csName, pFont); }
	BOOL ReadFont(CString csKey, CString csName, CFont* pFont);
	// Returns the CPoint associated with the specified 
	// name else, it returns the default specified
	BOOL ReadPoint(CString csName, CPoint* pPoint) { return ReadPoint(GetCurrentPath(), csName, pPoint); }
	BOOL ReadPoint(CString csKey, CString csName, CPoint* pPoint);
	// Returns the CSize associated with the specified 
	// name else, it returns the default specified
	BOOL ReadSize(CString csName, CSize* pSize) { return ReadSize(GetCurrentPath(), csName, pSize); }
	BOOL ReadSize(CString csKey, CString csName, CSize* pSize);
	// Returns the CRect associated with the specified 
	// name else, it returns the default specified
	BOOL ReadRect(CString csName, CRect* pRect) { return ReadRect(GetCurrentPath(), csName, pRect); }
	BOOL ReadRect(CString csKey, CString csName, CRect* pRect);
	//
	BOOL ReadValue(CString csName, DWORD dwRegType, KEY_VALUE_PARTIAL_INFORMATION** retInfo)
	{
		return ReadValue(GetCurrentPath(), csName, dwRegType, retInfo);
	}
	BOOL ReadValue(CString csKey, CString csName, DWORD dwRegType, KEY_VALUE_PARTIAL_INFORMATION** retInfo);
	//
	////////////////////////////////////////////////////////
	// END - CNtRegistry data reading functions
	////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////
	// CNtRegistry data writing functions
	////////////////////////////////////////////////////////
	//
	// Writes the String value specified to the Name specified
	BOOL WriteBinary(CString csName, UCHAR* pValue, UINT uiLength) { return WriteBinary(GetCurrentPath(), csName, pValue, uiLength); }
	BOOL WriteBinary(CString csKey, CString csName, UCHAR* pValue, UINT uiLength);
	// Writes the String value specified to the Name specified
	BOOL WriteString(CString csName, CString csValue) { return WriteString(GetCurrentPath(), csName, csValue); }
	BOOL WriteString(CString csKey, CString csName, CString csValue);
	// Writes the String value specified to the Name specified
	BOOL WriteExpandString(CString csName, CString csValue) { return WriteExpandString(GetCurrentPath(), csName, csValue); }
	BOOL WriteExpandString(CString csKey, CString csName, CString csValue);
	// Writes the String value specified to the Name specified
	BOOL WriteMultiString(CString csName, CStringArray& csaValue) { return WriteMultiString(GetCurrentPath(), csName, csaValue); }
	BOOL WriteMultiString(CString csKey, CString csName, CStringArray& csaValue);
	// Writes the BOOL value specified to the Name specified
	BOOL WriteBool(CString csName, BOOL bValue) { return WriteBool(GetCurrentPath(), csName, bValue); }
	BOOL WriteBool(CString csKey, CString csName, BOOL bValue);
	// Writes the int value specified to the Name specified
	BOOL WriteInt(CString csName, int nValue) { return WriteInt(GetCurrentPath(), csName, nValue); }
	BOOL WriteInt(CString csKey, CString csName, int nValue);
	// Writes the DWORD value specified to the Name specified
	BOOL WriteDword(CString csName, DWORD dwValue) { return WriteDword(GetCurrentPath(), csName, dwValue); }
	BOOL WriteDword(CString csKey, CString csName, DWORD dwValue);
	// Writes the COleDateTime value specified to the Name specified
	BOOL WriteDateTime(CString csName, COleDateTime dtValue) { return WriteDateTime(GetCurrentPath(), csName, dtValue); }
	BOOL WriteDateTime(CString csKey, CString csName, COleDateTime dtValue);
	// Writes the double value specified to the Name specified
	BOOL WriteFloat(CString csName, double fValue) { return WriteFloat(GetCurrentPath(), csName, fValue); }
	BOOL WriteFloat(CString csKey, CString csName, double fValue);
	// Writes the COLORREF value specified to the Name specified
	BOOL WriteColor(CString csName, COLORREF rgbValue) { return WriteColor(GetCurrentPath(), csName, rgbValue); }
	BOOL WriteColor(CString csKey, CString csName, COLORREF rgbValue);
	// Writes the CFont* value specified to the Name specified
	BOOL WriteFont(CString csName, CFont* pFont) { return WriteFont(GetCurrentPath(), csName, pFont); }
	BOOL WriteFont(CString csKey, CString csName, CFont* pFont);
	// Writes the CPoint* value specified to the Name specified
	BOOL WritePoint(CString csName, CPoint* pPoint) { return WritePoint(GetCurrentPath(), csName, pPoint); }
	BOOL WritePoint(CString csKey, CString csName, CPoint* pPoint);
	// Writes the CSize* value specified to the Name specified
	BOOL WriteSize(CString csName, CSize* pSize) { return WriteSize(GetCurrentPath(), csName, pSize); }
	BOOL WriteSize(CString csKey, CString csName, CSize* pSize);
	// Writes the CRect* value specified to the Name specified
	BOOL WriteRect(CString csName, CRect* pRect) { return WriteRect(GetCurrentPath(), csName, pRect); }
	BOOL WriteRect(CString csKey, CString csName, CRect* pRect);
	// 
	BOOL WriteValueString(CString csKey, CString csName, LPCTSTR lpszValue, int nLength, DWORD dwRegType);
	//
	BOOL WriteValue(CString csName, PVOID pValue, ULONG ulValueLength, DWORD dwRegType)
	{
		return WriteValue(GetCurrentPath(), csName, pValue, ulValueLength, dwRegType);
	}
	BOOL WriteValue(CString csKey, CString csName, PVOID pValue, ULONG ulValueLength, DWORD dwRegType);
	//
	////////////////////////////////////////////////////////
	// END - CNtRegistry data writing functions
	////////////////////////////////////////////////////////

};  // end of CNtRegistry class definition

#endif // #ifndef __NTREGISTRY_H__
