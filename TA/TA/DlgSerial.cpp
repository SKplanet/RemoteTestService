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


// DlgSerial.cpp : implementation file
//

#include "stdafx.h"
#include "TA.h"
#include "DlgSerial.h"
#include "afxdialogex.h"


// DlgSerial dialog

IMPLEMENT_DYNAMIC(DlgSerial, CDialogEx)

DlgSerial::DlgSerial(CWnd* pParent /*=NULL*/)
	: CDialogEx(DlgSerial::IDD, pParent)
	, m_Serial(_T(""))
{

}

DlgSerial::~DlgSerial()
{
}

void DlgSerial::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_SERIAL, m_Serial);
	DDV_MaxChars(pDX, m_Serial, 10);
}


BEGIN_MESSAGE_MAP(DlgSerial, CDialogEx)
	ON_BN_CLICKED(IDOK, &DlgSerial::OnBnClickedOk)
END_MESSAGE_MAP()


// DlgSerial message handlers


void DlgSerial::OnBnClickedOk()
{
	UpdateData();

	if( m_Serial.GetLength() < 2 )
	{
		AfxMessageBox(IDS_SHORT_SERIAL, MB_OK | MB_ICONHAND);
		return;
	}
	CDialogEx::OnOK();
}
