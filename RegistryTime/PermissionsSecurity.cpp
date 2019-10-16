///////////////////////////////////////////////////////////////////////////////
//  File Name: PermSec.cpp
// Programmer: Daniel Madden
//    Contact: dmadden1961@yahoo.com
//  Copyright: Copyright (c) 2006 Daniel Madden
//    Created: 25-Aug-2006
//
// Structure and some variables used from Jeffrey Richter's example so
// I included this copyright also: Copyright (c) 2000 Jeffrey Richter
//
///////////////////////////////////////////////////////////////////////////////


#include "StdAfx.h"
#include "PermissionsSecurity.h"

GUID CPermissionsSecurity::m_guidNULL = GUID_NULL;

///////////////////////////////////////////////////////////////////////////////
//const OBJECT_TYPE_LIST CPermissionsSecurity::m_DefaultOTL[] =
OBJECT_TYPE_LIST CPermissionsSecurity::m_DefaultOTL[] =
{
	// This empty object list is required by GetEffectivePermission(). 
	// This structure is required by ACLUI to be a static constant.
	{ 0, 0, const_cast<LPGUID>(&GUID_NULL) }
};

#define ACCESS_NULL_ENTRY NULL, 0, NULL, 0

///////////////////////////////////////////////////////////////////////////////
SI_ACCESS CPermissionsSecurity::m_siAccessRegRights[] =
{
	{ &m_guidNULL, KEY_ALL_ACCESS, L"Full Control",
	SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
	{ &m_guidNULL, KEY_QUERY_VALUE, L"Query Value",
	SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
	{ &m_guidNULL, KEY_SET_VALUE, L"Set Value",
	SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
	{ &m_guidNULL, KEY_CREATE_SUB_KEY, L"Create Subkeys",
	SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
	{ &m_guidNULL, KEY_ENUMERATE_SUB_KEYS, L"Enumerate Subkeys",
	SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
	{ &m_guidNULL, KEY_NOTIFY, L"Notify",
	SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
	{ &m_guidNULL, KEY_CREATE_LINK, L"Create Link",
	SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
	{ &m_guidNULL, READ_CONTROL, L"Read Control",
	SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
	{ &m_guidNULL, WRITE_DAC, L"Write DAC",
	SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
	{ &m_guidNULL, WRITE_OWNER, L"Write Owner",
	SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
	{ &m_guidNULL, DELETE, L"Delete",
	SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
	{ &m_guidNULL, SYNCHRONIZE, L"Synchronize",
	SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
	{ &m_guidNULL, ACCESS_SYSTEM_SECURITY, L"Access System Security",
	SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC },
	{ ACCESS_NULL_ENTRY }
};

///////////////////////////////////////////////////////////////////////////////
SI_INHERIT_TYPE CPermissionsSecurity::m_siInheritType[] = {
	{ &m_guidNULL, 0, L"This key and subkeys" },
	{ &m_guidNULL, CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE, L"This key, inherited keys/subkeys" },
	{ &m_guidNULL, CONTAINER_INHERIT_ACE, L"This key and containers" },
	{ &m_guidNULL, OBJECT_INHERIT_ACE, L"This key and inherited objects" },
	{ &m_guidNULL, CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE, L"Inherited keys/subkeys" },
	{ &m_guidNULL, OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE, L"Inherited Keys" },
	{ &m_guidNULL, INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE, L"Inherited Containers" }
};

ULONG CPermissionsSecurity::mc_siInheritTypes = sizeof(m_siInheritType) / sizeof(m_siInheritType[0]);

///////////////////////////////////////////////////////////////////////////////
// This Supports:
//		- IUnknown
//		- ISecurityInformation
//		- IEffectivePermission
///////////////////////////////////////////////////////////////////////////////
HRESULT CPermissionsSecurity::QueryInterface(REFIID riid, PVOID* ppvObj)
{
	HRESULT hr = E_NOINTERFACE;
	if (::IsEqualIID(riid, IID_IUnknown))
	{
		*ppvObj = this;
		AddRef();
		return S_OK;
	}
	if (::IsEqualIID(riid, IID_ISecurityInformation))
	{
		*ppvObj = dynamic_cast<ISecurityInformation *>(this);
		AddRef();
		return S_OK;
	}
	if (::IsEqualIID(riid, IID_IEffectivePermission))
	{
		*ppvObj = dynamic_cast<IEffectivePermission *>(this);
		AddRef();
		return S_OK;
	}
	return(hr);
}

///////////////////////////////////////////////////////////////////////////////
// IUnknown method
///////////////////////////////////////////////////////////////////////////////
ULONG CPermissionsSecurity::AddRef()
{
	return ::InterlockedIncrement(&m_nRef);
}

///////////////////////////////////////////////////////////////////////////////
// IUnknown method
///////////////////////////////////////////////////////////////////////////////
ULONG CPermissionsSecurity::Release()
{
	if (::InterlockedDecrement(&m_nRef) == 0) {
		delete this;
		return 0;
	}
	return(m_nRef);
}

///////////////////////////////////////////////////////////////////////////////
CPermissionsSecurity::CPermissionsSecurity(ObjInf* pInfo)
{
	m_pInfo = pInfo;
	m_nRef = 1;

}


///////////////////////////////////////////////////////////////////////////////
CPermissionsSecurity & CPermissionsSecurity::operator=(const CPermissionsSecurity &OldCls)
{
	if (this == &OldCls) {
		return *this;
	}

	this->m_nRef++;
	this->m_pInfo = OldCls.m_pInfo;
	this->m_guidNULL = OldCls.m_guidNULL;
	this->mc_siInheritTypes = OldCls.mc_siInheritTypes;
	memcpy(&this->m_DefaultOTL, &OldCls.m_DefaultOTL, sizeof(OldCls.m_DefaultOTL));
	memcpy(&this->m_siAccessRegRights, &OldCls.m_siAccessRegRights, sizeof(OldCls.m_siAccessRegRights));
	memcpy(&this->m_siInheritType, &OldCls.m_siInheritType, sizeof(OldCls.m_siInheritType));
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
CPermissionsSecurity::~CPermissionsSecurity()
{
#pragma chMSG(Is this really needed!)
	while (m_nRef != 0) {
		Release();
	}
}

///////////////////////////////////////////////////////////////////////////////
// dwFlags: A set of flags that influences the behaviour of the ACL editor. 
// For example, if you do not want the user pressing "Advanced" to show the 
// advanced ACL editor, you can disable it here. If you don't want the user 
// editing the security descriptor, set the SI_READONLY flag. Here are the more 
// important flags: 
//     SI_ADVANCED: Specifies the "Advanced" button will be shown so that the 
//    				user can enter the advanced UI. 
//     SI_CONTAINER: Indicates that your object is a directory (or behaves like 
//    				a directory). You should set this flag if the object is a 
//    				directory so the ACL editor can apply inheritance on the object. 
//    				You may also want to enable the inheritance stuff. 
//     SI_EDIT_AUDITS: Shows the "Auditing" tab (so the user can edit the SACL). 
//    				To view / edit the SACL, you must have the "SeSecurityPrivilege". 
//     SI_EDIT_PERMS: Shows the "Advanced" permissions tab (so the user can fully 
//    				edit the DACL). 
//     SI_EDIT_OWNER: Shows the Owner tab (so the user can change or take ownership). 
//     SI_EDIT_PROPERTIES: Shows the properties tab (so the user can edit the 
//    				properties associated with the object). 
//     SI_EDIT_EFFECTIVE (undocumented, for Windows XP and later): Shows the 
//    				"Effective Permissions" tab. 
//     SI_READONLY: Does not allow the user to change the security descriptor. 
//     SI_OWNER_READONLY: Does not allow the user to change the owner (to change 
//    				ownership, the user must be the owner, or have the 
//    				"SeTakeOwnershipPrivilege"). 
//     SI_MAY_WRITE (undocumented): "Not sure if the user may write permission". 
//     SI_NO_ADDITIONAL_PERMISSION (Undocumented): ?????? 
//     SI_NO_ACL_PROTECT: Select this if you haven't implemented inheritance. 
//    				This hides the auto-inherit checkbox. 
//     SI_OWNER_RECURSE: Shows the "Sledgehammer" button, which enables the user 
//    				to reset all children's security descriptors to default. 
//    				Do not show this unless you have implemented recursive walking 
//    				in your object. 
//     SI_RESET_DACL_TREE: Like SI_OWNER_RECURSE, but for the Advanced Permissions 
//    				tab. 
//     SI_RESET_SACL_TREE: Like SI_RESET_DACL_TREE, but for the "Auditing" tab. 
//     SI_SERVER_IS_DC: Set this if you have determined the computer is in a 
//    				domain environment. 
//     SI_EDIT_ALL: shorthand for SI_EDIT_AUDITS | SI_EDIT_PERMS | SI_EDIT_OWNER. 
//     SI_NO_TREE_APPLY: Hides the "Apply these permissions to objects and/or 
//    				containers within this container only" checkbox. Set this 
//    				flag if your object does not support inheritance. 
//     SI_PAGE_TITLE: Indicates you want a custom title to appear in the ACL editor. 
//    				Specify the alternate title in the pszPageTitle member. 
//     SI_RESET: Shows the "Default" button, which will reset the security 
//    				descriptor to defaults. If you don't know what the default 
//    				security descriptor is, do not enable this flag. 
//     SI_RESET_SACL, SI_RESET_DACL, SI_RESET_OWNER: shows the "Default" button 
//    				for certain tabs only. 
//     SI_OBJECT_GUID: (domain-specific) If your object supports multiple 
//    				inheritance, you should enable this flag. 
///////////////////////////////////////////////////////////////////////////////
HRESULT CPermissionsSecurity::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo)
{
	// We are doing both normal and advanced editing
	pObjectInfo->dwFlags = SI_ADVANCED | SI_CONTAINER | SI_EDIT_ALL |
		SI_EDIT_EFFECTIVE | SI_EDIT_PROPERTIES |
		SI_PAGE_TITLE | SI_RESET_DACL_TREE |
		SI_RESET_SACL_TREE | SI_OWNER_RECURSE;

	pObjectInfo->hInstance = GetModuleHandle(NULL);
	pObjectInfo->pszServerName = NULL;

	// This is for the "Tab" title and works with "SI_PAGE_TITLE"
	pObjectInfo->pszPageTitle = L"NtRegEdit (Security)";

	DWORD dwSize = 0;
//	pObjectInfo->pszObjectName = _A2W(m_pInfo->m_szObjectName, dwSize);
	// 11-17-2018
	pObjectInfo->pszObjectName = m_pInfo->m_szObjectName;
	return(S_OK);
}

///////////////////////////////////////////////////////////////////////////////
// Convert Whole String
///////////////////////////////////////////////////////////////////////////////
LPWSTR CPermissionsSecurity::_A2W(LPSTR szMultiByte, DWORD& dwSize)
{
	static WCHAR szInternalBuffer[8096];

	if (szMultiByte == NULL || ::strlen(szMultiByte) == 0) {
		dwSize = 0;
		return NULL;
	}

	int nLen = MultiByteToWideChar(CP_ACP, 0, szMultiByte, -1, NULL, NULL);
	if (nLen > 8096) {
		dwSize = 0;
		return NULL;
	}

	MultiByteToWideChar(CP_ACP, 0, szMultiByte, -1, szInternalBuffer, nLen);
	return szInternalBuffer;
}

///////////////////////////////////////////////////////////////////////////////
//
// This method is called by GetEffectivePermission when the user uses ACLUI's 
// frontend for GetEffectiveRightsFromAcl().
//
// We are supplied the User SID, the server for that SID, and the SD. We must 
// fill the object list and the grantedAccessList (and their sizes).
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CPermissionsSecurity::GetEffectivePermission(const GUID * /*pguidObjectType*/,
	PSID pUserSid,
	LPCWSTR /*pszServerName*/,
	PSECURITY_DESCRIPTOR pSD,
	POBJECT_TYPE_LIST* ppObjectTypeList,
	ULONG* pcObjectTypeListLength,
	PACCESS_MASK* ppGrantedAccessList,
	ULONG* pcGrantedAccessListLength)
{
	DWORD dwErr = 0;
	BOOL AclPresent = FALSE, AclDefaulted = FALSE;
	PACCESS_MASK AccessRights = NULL;
	PACL Dacl = NULL;

	*ppObjectTypeList = const_cast<OBJECT_TYPE_LIST *>(m_DefaultOTL);
	*pcObjectTypeListLength = 1;
	SetLastError(ERROR_SUCCESS);

	// Get the Acl from the SD given
	dwErr = GetSecurityDescriptorDacl(pSD, &AclPresent, &Dacl, &AclDefaulted);
	if (Dacl != NULL && dwErr == TRUE) {
		// Make a trustee from the SID given
		TRUSTEE Trustee = { 0 };
		::BuildTrusteeWithSid(&Trustee, pUserSid);

		// Copy AccessRights to a LocalAlloc ptr. ACLUI will free it.
		AccessRights = reinterpret_cast<PACCESS_MASK>(::LocalAlloc(LPTR, sizeof(PACCESS_MASK) + sizeof(ACCESS_MASK)));
		if (AccessRights == NULL) {
			return E_OUTOFMEMORY;
		}

		dwErr = GetEffectiveRightsFromAcl(Dacl, &Trustee, AccessRights);
		if (dwErr != ERROR_SUCCESS) {
			// And call the required API.
			::LocalFree(AccessRights); AccessRights = NULL;
			return HRESULT_FROM_WIN32(dwErr);
		}
		*ppGrantedAccessList = AccessRights;
		*pcGrantedAccessListLength = 1;
		return S_OK;

	}
	return HRESULT_FROM_WIN32(dwErr);
}

///////////////////////////////////////////////////////////////////////////////
//
//  IN  SECURITY_INFORMATION RequestedInformation
//					OWNER_SECURITY_INFORMATION - GROUP_SECURITY_INFORMATION -
//					DACL_SECURITY_INFORMATION - SACL_SECURITY_INFORMATION
//  OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor
//  IN  BOOL fDefault (user click default)
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CPermissionsSecurity::GetSecurity(SECURITY_INFORMATION RequestedInformation,
	PSECURITY_DESCRIPTOR* ppSecurityDescriptor,
	BOOL fDefault)
{
	HRESULT hr = 1;
	PSECURITY_DESCRIPTOR pSD;

	// Get security information
	ULONG lErr;
	lErr = GetNamedSecurityInfo(m_pInfo->m_szName,
		SE_REGISTRY_KEY, RequestedInformation,
		NULL, NULL, NULL, NULL, &pSD);

	// No matter what we still display security information
	//
	// Failure produces an empty SD
	if (lErr != ERROR_SUCCESS) {
		MessageBox(NULL, TEXT("An error occurred retrieving security ")
			TEXT("information for this object, \npossibly due to insufficient")
			TEXT(" access rights.  AccessMaster \nhas created an empty security")
			TEXT(" descriptor for editing."), TEXT("AccessMaster Notice"), MB_OK);
	}
	else {
		hr = S_OK;
		*ppSecurityDescriptor = pSD;
	}
	return(hr);
}

///////////////////////////////////////////////////////////////////////////////
//
//  IN  const GUID *pguidObjectType
//  IN  DWORD dwFlags = SI_ADVANCED - SI_EDIT_AUDITS - SI_EDIT_PROPERTIES
//  OUT PSI_ACCESS *ppAccess -- number of SI_ACCESS 
//  OUT ULONG *pcAccesses -- array of the SI_ACCESS
//  OUT ULONG *piDefaultAccess -- default index in the array
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CPermissionsSecurity::GetAccessRights(const GUID* pguidObjectType,
	DWORD dwFlags, PSI_ACCESS* ppAccess, ULONG* pcAccesses,
	ULONG* piDefaultAccess)
{
	// Locate the appropriate block of specific rights
	// See the beginning of this file
	*ppAccess = m_siAccessRegRights;
	*pcAccesses = 0;
	while ((*ppAccess)[*pcAccesses].mask != 0) {
		(*pcAccesses)++;
	}
	*piDefaultAccess = 0;

	return(S_OK);
}

///////////////////////////////////////////////////////////////////////////////
//
//  IN  const GUID *pguidObjectType
//  IN  UCHAR *pAceFlags 
//  IN  ACCESS_MASK *pMask
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CPermissionsSecurity::MapGeneric(const GUID* pguidObjectType,
	UCHAR* pAceFlags, ACCESS_MASK* pMask)
{
	if (pguidObjectType && !InlineIsEqualGUID(*pguidObjectType, GUID_NULL)) {
		return E_INVALIDARG;
	}

	GENERIC_MAPPING ObjMap;
	ObjMap.GenericAll = KEY_ALL_ACCESS;
	ObjMap.GenericWrite = KEY_WRITE;
	ObjMap.GenericRead = KEY_READ;
	ObjMap.GenericExecute = 0;
	::MapGenericMask(pMask, &ObjMap);

	return(S_OK);
}

///////////////////////////////////////////////////////////////////////////////
//
//  OUT PSI_INHERIT_TYPE *ppInheritTypes -- array of inherit flag
//  OUT ULONG *pcInheritTypes -- array size
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CPermissionsSecurity::GetInheritTypes(PSI_INHERIT_TYPE* ppInheritTypes,
	ULONG* pcInheritTypes)
{
	*ppInheritTypes = const_cast<SI_INHERIT_TYPE *>(m_siInheritType);
	*pcInheritTypes = mc_siInheritTypes;
	//#pragma chMSG(NEEDS to be FIXED (Possibly)!!)
	return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////
//
//  IN  HWND hwnd
//  IN  UINT uMsg = PSPCB_CREATE - PSPCB_RELEASE - PSPCB_SI_INITDIALOG
//  IN  SI_PAGE_TYPE uPage
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CPermissionsSecurity::PropertySheetPageCallback(HWND hWnd,
	UINT uMsg,
	SI_PAGE_TYPE uPage)
{
	UNREFERENCED_PARAMETER(hWnd);
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(uPage);
	return(S_OK);
}


///////////////////////////////////////////////////////////////////////////////
//
//  IN  SECURITY_INFORMATION SecurityInformation
//  IN  PSECURITY_DESCRIPTOR pSecurityDescriptor
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CPermissionsSecurity::SetSecurity(SECURITY_INFORMATION SecurityInformation,
	PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
	HRESULT hr = 1;

	// Get the Dacl
	PACL pDACL = NULL;
	BOOL fPresent, fDefaulted;
	GetSecurityDescriptorDacl(pSecurityDescriptor, &fPresent, &pDACL, &fDefaulted);

	// Get the SACL
	PACL pSACL = NULL;
	GetSecurityDescriptorSacl(pSecurityDescriptor, &fPresent, &pSACL, &fDefaulted);

	// Get the owner
	PSID psidOwner = NULL;
	GetSecurityDescriptorOwner(pSecurityDescriptor, &psidOwner, &fDefaulted);

	// Get the group
	PSID psidGroup = NULL;
	GetSecurityDescriptorOwner(pSecurityDescriptor, &psidGroup, &fDefaulted);

	// Find out if DACL and SACL inherit from parent objects
	SECURITY_DESCRIPTOR_CONTROL sdCtrl = NULL;
	ULONG ulRevision;
	GetSecurityDescriptorControl(pSecurityDescriptor, &sdCtrl, &ulRevision);
	if ((sdCtrl & SE_DACL_PROTECTED) != SE_DACL_PROTECTED) {
		SecurityInformation |= UNPROTECTED_DACL_SECURITY_INFORMATION;
	}
	else {
		SecurityInformation |= PROTECTED_DACL_SECURITY_INFORMATION;
	}

	if ((sdCtrl & SE_SACL_PROTECTED) != SE_SACL_PROTECTED) {
		SecurityInformation |= UNPROTECTED_SACL_SECURITY_INFORMATION;
	}
	else {
		SecurityInformation |= PROTECTED_SACL_SECURITY_INFORMATION;
	}

	// Set the security
	ULONG lErr;
	lErr = SetNamedSecurityInfo(m_pInfo->m_szName,
		SE_REGISTRY_KEY, SecurityInformation, psidOwner,
		psidGroup, pDACL, pSACL);

	// Report error
	if (lErr == ERROR_SUCCESS) {
		//		ReportErrEx(TEXT("GetNamedSecurityInfo %d"), lErr);
		//	}
		//	else {
#pragma chMSG(NEEDS Fixing: CPermissionsSecurity::SetSecurity())
		hr = S_OK;
	}
	return(hr);
}
