//////////////////////////////////////////////////////////////////////
//
// File           : TokenEx.h
// Version        : 0.2
// Function       : Header file of the CTokenEx classes.
//
// Author         : Daniel Madden Sr.
// Date           : Jan 3, 2000
//
// Notes          : None
//
// Modifications  :
//
// Rev 0.2	Jun 3, 2006	Dan Madden
//	- Added some user comments from folks here at CodeProject 
//
// Rev 0.1	Jan 3, 2000	Dan Madden
//	- Initial creation
// 
//
// Written by Daniel Madden
// Copyright © 2000-2006 Daniel Madden Sr.
//
// This grants you ("Licensee") a non-exclusive, royalty free, 
// licence to use, modify and redistribute this software in source and binary 
// code form, provided that i) this copyright notice and licence appear on all 
// copies of the software; and ii) Licensee does not utilize the software in a 
// manner which is disparaging to Daniel Madden.
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


#ifndef _TOKENEX_H__
#define _TOKENEX_H__

#pragma once

class CTokenEx
{
public:

	CTokenEx();
	~CTokenEx();

	inline void SetDeliminator(CString Deliminator) { m_Deliminator = Deliminator; }
	inline CString GetDeliminator(void) { return m_Deliminator; }

	inline CStringArray& GetCStringArray(void) { return m_csaAddIt; }

	inline void SetAddEmpty(BOOL bAddEmpty) { m_bAddEmpty = bAddEmpty; }
	inline BOOL GetAddEmpty(void) { return m_bAddEmpty; }


	// Splits a Path into 4 parts (Directory, Drive, Filename, Extention).
	// NOTE: Supports UNC path names.
	void SplitPath(BOOL UsingDirsOnly, CString Path, CString& Drive, CString& Dir, CString& FName, CString& Ext);

	// Splits a CString into an CStringArray according the Deliminator.
	void Split(CString Source, CString csDeliminator, BOOL bAddEmpty);
	void Split(CString Source, BOOL bAddEmpty);
	void Split(CString Source, CString csDeliminator);
	void Split(CString Source);

	// Joins a CStringArray to create a CString according the Deliminator.
	CString Join(void);

	// Returns the first string found (according to the token given). If the
	// string does not contain a token, then it returns the entire string. 
	//
	// NOTE: This uses a REFERENCE as the first param, so everytime it finds
	// a token, the string (& deliminator) are removed from the referenced
	// string.  SEE CODE EXAMPLE!
	CString GetString(CString &csRefered);

	CStringArray	m_csaAddIt;

protected:
	BOOL			m_bAddEmpty;
	CString			m_Deliminator;

};

#endif // ifndef _TOKENEX_H__
