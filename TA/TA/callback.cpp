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




#include "stdafx.h"
#include "resource.h"
#include "TADlg.h"

void CTADlg::OnImcomingTALog(RA* pra, void* pData, BYTE* buffer, int size)
{
	if( !pra || !pData)
		return;

	CTADlg* pDlg = (CTADlg*)pData;

	if( ::IsWindow(pDlg->GetSafeHwnd()) )
	{
		char * b = (char*)malloc(size);
		memcpy(b, buffer, size);
		pDlg->PostMessage(WM_TA_LOG, (WPARAM)b, (LPARAM)size);
		//pDlg->OnTALog(buffer, size);
	}
}


void CTADlg::OnTAState(RA* pra, void* pData, int state)
{
	if( !pra || !pData)
		return;

	CTADlg* pDlg = (CTADlg*)pData;

	if( ::IsWindow(pDlg->GetSafeHwnd()) )
	{
		pDlg->PostMessage(WM_TA_STATEUPDATE,(WPARAM)state,0);
		//pDlg->RefreshTAState();
	}
}