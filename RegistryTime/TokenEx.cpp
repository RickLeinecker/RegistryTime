//////////////////////////////////////////////////////////////////////
//
// File           : TokenEx.cpp
// Version        : 0.2
// Function       : CPP file of the CTokenEx classes.
//
// Author         : Daniel Madden Sr.
// Date           : Jan 3, 2000
//
// Notes          : None
//
// Modifications  : See header file...
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

#include "stdafx.h"
#include "TokenEx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTokenEx::CTokenEx()
{
	m_Deliminator = _T(",");
	m_bAddEmpty = FALSE;
	m_csaAddIt.RemoveAll();
}

CTokenEx::~CTokenEx()
{
	m_Deliminator = _T("");
	m_csaAddIt.RemoveAll();
}



void CTokenEx::Split(CString Source, CString csDeliminator, BOOL bAddEmpty)
{
	m_Deliminator = csDeliminator;
	m_bAddEmpty = bAddEmpty;
	Split(Source);
}


void CTokenEx::Split(CString Source, CString csDeliminator)
{
	m_Deliminator = csDeliminator;
	Split(Source);
}


void CTokenEx::Split(CString Source, BOOL bAddEmpty)
{
	m_bAddEmpty = bAddEmpty;
	Split(Source);
}


void CTokenEx::Split(CString Source)
{
	// initialize the variables
	CString		 newCString = Source;
	CString		 tmpCString = L"";
	CString		 AddCString = L"";

	int pos1 = 0;
	int pos = 0;

	m_csaAddIt.RemoveAll();

	if (m_Deliminator.IsEmpty()) {
		// Add default [comma] if empty!
		// acknowledgement: Prasad [gprasad@rti.ie]
		m_Deliminator = ",";
	}

	//
	CString csStr = newCString;
	int nDelCount = m_Deliminator.GetLength();

	///////////////////////////////////////////////////
	//
	// The below block was created as a suggestion 
	// from "l_d_allan" at CodeProject.com
	//
	CString csMultDel = _T("");
	for (int n = 0; n<nDelCount; n++) {
		csMultDel = _T("");
		csMultDel += m_Deliminator[n];
		csStr.Replace(csMultDel, L"~");
	}
	m_Deliminator = _T("~");
	newCString = csStr;
	//
	///////////////////////////////////////////////////


	// do this loop as long as you have a deliminator
	do {
		// set to zero
		pos1 = 0;
		// position of deliminator starting at pos1 (0)
		pos = newCString.Find(m_Deliminator, pos1);
		// if the deliminator is found...
		if (pos != -1) {

			// load a new var with the info left
			// of the position
			CString AddCString = newCString.Left(pos);

			if (!AddCString.IsEmpty()) {
				// if there is a string to add, then
				// add it to the Array
				m_csaAddIt.Add(AddCString);
			}
			else if (m_bAddEmpty) {
				// if empty strings are ok, then add them
				m_csaAddIt.Add(AddCString);
			}

			// make a copy of the of this var. with the info
			// right of the deliminator
			tmpCString = newCString.Mid(pos + m_Deliminator.GetLength());

			// reset this var with new info
			newCString = tmpCString;
		}
	} while (pos != -1);

	if ((!newCString.IsEmpty()) || m_bAddEmpty) {
		// as long as the variable is not emty, add it
		m_csaAddIt.Add(newCString);
	}
}


CString CTokenEx::GetString(CString &tmp)
{
	int		nFound = -1;

	CString csRet = L"";
	CString csTmp1 = tmp;

	if (m_Deliminator.IsEmpty()) {
		// Add default [comma] if empty!
		// acknowledgement: Guru Prasad [gprasad@rti.ie]
		m_Deliminator = ",";
	}

	if (!m_bAddEmpty) {
		// This 'do loop' will strip the empty
		// deliminators from the string
		if (tmp.Left(m_Deliminator.GetLength()) == m_Deliminator) {
			do {
				// get the 
				tmp = csTmp1.Mid(m_Deliminator.GetLength());
				csTmp1 = tmp;
			} while (tmp.Left(m_Deliminator.GetLength()) == m_Deliminator);
			tmp = csTmp1.Mid(m_Deliminator.GetLength() - 1);
		}
	}

	// copy of the 'ref' tmp variable 
	CString csTmp = tmp;

	// get the first position of the first
	// deliminator in the string
	nFound = tmp.Find(m_Deliminator, 0);
	if (nFound >= 0) {
		// fill the var. with info left
		// of the deliminator
		csRet = csTmp.Left(nFound);

		// load the ref'd var. with the info right of
		// the csTmp var.
		tmp = csTmp.Mid(nFound + (m_Deliminator.GetLength()));
	}
	else {
		// no deliminator found, so return the whole string
		csRet = csTmp;

		// load the ref'd var. with the empty string
		tmp = "";
	}

	// return the string found (if any)
	return csRet;
}


CString CTokenEx::Join(void)
{
	CString csReturn;
	CString csTmp;

	if (m_csaAddIt.GetSize() < 1) {
		//
		return _T("ERROR: Array size is 0 ??");
	}

	if (m_Deliminator.IsEmpty()) {
		// Add default [comma] if empty!
		// acknowledgement: Guru Prasad [gprasad@rti.ie]
		m_Deliminator = ",";
	}

	// Loop through the Array and Append the Deliminator
	// to the string
	for (int iNum = 0; iNum < m_csaAddIt.GetSize(); iNum++) {
		csTmp += m_csaAddIt.GetAt(iNum);
		csTmp += m_Deliminator;
	}
	// finish the returned string with the info
	// found (and remove the last appended deliminator)
	csReturn = csTmp.Left(csTmp.GetLength() - 1);

	return csReturn;
}


void CTokenEx::SplitPath(BOOL UsingDirsOnly, CString Path, CString& Drive, CString& Dir, CString& FName, CString& Ext)
{
	//
	int nSecond;

	// Look for a UNC Name!
	// If the first two CHARs are
	// Back-Slashes "\\", assume its UNC
	if (Path.Left(2) == L"\\\\") {

		// find the first '\' after the first two '\\'
		int nFirst = Path.Find(L"\\", 3);

		// find the second '\' after the first one found
		nSecond = Path.Find(L"\\", nFirst + 1);

		// if the second does NOT exist
		if (nSecond == -1) {
			// add the 'Path' var to the 'Drive' var
			// because the passed var is just a UNC share
			// i.e.: '\\Server\Share'
			Drive = Path;

			// set ALL others to empty
			Dir = "";
			FName = "";
			Ext = "";
		}
		else if (nSecond > nFirst) {
			// set the 'Drive' var with the correct info
			// i.e.: '\\Server\Share'
			Drive = Path.Left(nSecond);
		}
	}
	else {
		// Look for normal Drive Structure (C:\Dir\File.name)
		nSecond = 2;
		Drive = Path.Left(2);
	}

	// check if we are looking for directory or file
	if (UsingDirsOnly) {
		// load the correct information in the 'Dir' var
		Dir = Path.Right((Path.GetLength() - nSecond) - 1);

		// set ALL others to empty
		FName = "";
		Ext = "";
	}
	else { // this means we have a filename structure
		// first find the the '\' (in reverse)
		int nDirEnd = Path.ReverseFind('\\');
		if (nDirEnd == Path.GetLength()) {
			// if we are here, this means that
			// 'C:\' was passed, so
			// set ALL to empty
			Dir = "";
			FName = "";
			Ext = "";
		}
		else {

			// set the 'Dir' var with directory info
			Dir = Path.Mid(nSecond + 1, (nDirEnd - nSecond) - 1);

			// breakdown the filename
			//
			// look for a period '.'
			int nFileEnd = Path.ReverseFind('.');

			// if it doesn't exist 
			if (nFileEnd != -1) {

				if (nDirEnd > nFileEnd) {
					// this would also mean no file extension found
					FName = Path.Right(Path.GetLength() - nDirEnd);
					Ext = "";
				}
				else {
					// fill vars with info
					FName = Path.Mid(nDirEnd + 1, (nFileEnd - nDirEnd) - 1);
					Ext = Path.Right((Path.GetLength() - nFileEnd) - 1);
				}
			}
			else {
				// this means filename w/o extention
				FName = Path.Right((Path.GetLength() - nDirEnd) - 1);
				Ext = "";
			}
		}
	}
}

