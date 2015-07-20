/*
	* The MIT License (MIT)
	* Copyright (c) 2015 SK PLANET. All Rights Reserved.
	*
	* Permission is hereby granted, free of charge, to any person obtaining a copy
	* of this software and associated documentation files (the "Software"), to deal
	* in the Software without restriction, including without limitation the rights
	* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	* copies of the Software, and to permit persons to whom the Software is
	* furnished to do so, subject to the following conditions:
	*
	* The above copyright notice and this permission notice shall be included in
	* all copies or substantial portions of the Software.
	*
	* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	* THE SOFTWARE.
*/



// RAToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "TA.h"
#include "RAToolBar.h"


// CRAToolBar

IMPLEMENT_DYNAMIC(CRAToolBar, CToolBar)

CRAToolBar::CRAToolBar()
{

}

CRAToolBar::~CRAToolBar()
{
}


BEGIN_MESSAGE_MAP(CRAToolBar, CToolBar)
	ON_NOTIFY_REFLECT(NM_LDOWN, &CRAToolBar::OnNMLDown)
	ON_NOTIFY_REFLECT(NM_RELEASEDCAPTURE, &CRAToolBar::OnNMReleasedcapture)
	ON_NOTIFY_REFLECT(NM_CLICK, &CRAToolBar::OnNMClick)
END_MESSAGE_MAP()



// CRAToolBar message handlers




void CRAToolBar::OnNMLDown(NMHDR *pNMHDR, LRESULT *pResult)
{
	// This feature requires Internet Explorer 5 or greater.
	// The symbol _WIN32_IE must be >= 0x0500.
	LPNMCLICK pNMClick = reinterpret_cast<LPNMCLICK>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	//TRACE("OnNMLDown\n");
}


void CRAToolBar::OnNMReleasedcapture(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
	//TRACE("OnNMReleasedcapture\n");
}


void CRAToolBar::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCLICK pNMClick = reinterpret_cast<LPNMCLICK>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	//TRACE("OnNMClick\n");
}
