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




#pragma once
#include "afxwin.h"
#include "SSplitter.h"
#include "rui.h"
#include "RAToolBar.h"
#include "Preview.h"

// CDlgPreview dialog
using namespace Gdiplus;

#define WM_PREVIEW_CLOSED	(WM_USER + 0x001)

class CDlgPreview : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPreview)

public:
	CDlgPreview(RA* pRA, CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgPreview();

// Dialog Data
	enum { IDD = IDD_DIALOG_PREVIEW };

	RA* m_pra;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CRAToolBar m_wndToolBar;
	//CSSplitter  m_split;
	CPreview m_preview;
	CEdit m_log;
	virtual BOOL OnInitDialog();
//	afx_msg void OnStnClickedStaticPreview();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnCmdScreenshot();
	afx_msg void OnCmdWake();
	afx_msg void OnCmdShell();
	afx_msg void OnCmdRotate();
	afx_msg void OnCmdHome();
	afx_msg void OnCmdMenu();
	afx_msg void OnCmdBack();
	afx_msg void OnCmdVolUp();
	afx_msg void OnCmdVolDn();
};
