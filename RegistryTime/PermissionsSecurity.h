///////////////////////////////////////////////////////////////////////////////
//  File Name: PermSec.h
// Programmer: Daniel Madden
//    Contact: dmadden1961@yahoo.com
//  Copyright: Copyright (c) 2006 Daniel Madden
//    Created: 25-Aug-2006
//
// Structure and some variables used from Jeffrey Richter's example so
// I included this copyright also: Copyright (c) 2000 Jeffrey Richter
//
///////////////////////////////////////////////////////////////////////////////

#ifndef PERMS_SEC
#define PERMS_SEC

#include <ACLAPI.h>
#include <ACLUI.h>

#pragma comment(lib, "Aclui.lib")

typedef struct _ObjInf {
	HANDLE			m_hHandle;
	TCHAR			m_szName[1024];
	TCHAR			m_szObjectName[2048];
	TCHAR			m_szParentName[32];
} ObjInf;

///////////////////////////////////////////////////////////////////////////////
class CPermissionsSecurity : public ISecurityInformation, public IEffectivePermission
{
public:
	// Constructors
	CPermissionsSecurity(ObjInf* pInfo);
	CPermissionsSecurity &operator=(const CPermissionsSecurity &);
	virtual ~CPermissionsSecurity();

private:
	static GUID m_guidNULL;
	static ULONG mc_siInheritTypes;
	static SI_INHERIT_TYPE m_siInheritType[];
	static SI_ACCESS m_siAccessRegRights[];
	static OBJECT_TYPE_LIST m_DefaultOTL[];
	//	static const OBJECT_TYPE_LIST m_DefaultOTL[];

	volatile LONG m_nRef;
	ObjInf*  m_pInfo;

public:
	// IUnknown methods
	HRESULT WINAPI QueryInterface(REFIID riid, PVOID* ppvObj);
	ULONG WINAPI AddRef();
	ULONG WINAPI Release();

	// IEffectivePermission methods
	HRESULT WINAPI GetEffectivePermission(const GUID * /*pguidObjectType*/,
		PSID pUserSid,
		LPCWSTR /*pszServerName*/,
		PSECURITY_DESCRIPTOR pSD, POBJECT_TYPE_LIST* ppObjectTypeList,
		ULONG* pcObjectTypeListLength, PACCESS_MASK* ppGrantedAccessList,
		ULONG* pcGrantedAccessListLength);

	// ISecurityInformation methods
	HRESULT WINAPI GetObjectInformation(PSI_OBJECT_INFO pObjectInfo);
	HRESULT WINAPI GetSecurity(SECURITY_INFORMATION RequestedInformation,
		PSECURITY_DESCRIPTOR* ppSecurityDescriptor,
		BOOL fDefault);
	HRESULT WINAPI SetSecurity(SECURITY_INFORMATION SecurityInformation,
		PSECURITY_DESCRIPTOR pSecurityDescriptor);
	HRESULT WINAPI GetAccessRights(const GUID* pguidObjectType,
		DWORD dwFlags, // si_edit_audits, si_edit_properties
		PSI_ACCESS* ppAccess,
		ULONG* pcAccesses,
		ULONG* piDefaultAccess);
	HRESULT WINAPI MapGeneric(const GUID* pguidObjectType,
		UCHAR* pAceFlags, ACCESS_MASK* pMask);
	HRESULT WINAPI GetInheritTypes(PSI_INHERIT_TYPE* ppInheritTypes,
		ULONG* pcInheritTypes);
	HRESULT WINAPI PropertySheetPageCallback(HWND hwnd, UINT uMsg,
		SI_PAGE_TYPE uPage);

	// Our methods (from CmnHdr.h - Jeffrey Richter)
	LPWSTR _A2W(LPSTR szMultiByte, DWORD& dwSize);

};

#endif // PERMS_SEC

///////////////////////////////// End of File /////////////////////////////////