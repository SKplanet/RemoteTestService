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


// TADlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "rui.h"
#include "afxcmn.h"

class CDlgPreview;

extern UINT WM_TA_INIT;
extern UINT WM_TA_STATEUPDATE;
extern UINT WM_TA_LOG;

// CTADlg dialog
class CTADlg : public CDialogEx
{
// Construction
public:
	CTADlg(CWnd* pParent = NULL);	// standard constructor
	~CTADlg();
// Dialog Data
	enum { IDD = IDD_TA_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	RA* m_pra;
	CDlgPreview* m_preview;
// Implementation
public:
	static void OnImcomingTALog(RA* pra, void* pData, BYTE* buffer, int size);
	static void OnTAState(RA* pra, void* pData, int state);
	//void OnTALog(BYTE* buffer, int size);
	LRESULT OnTALog(WPARAM wParam, LPARAM lParam);
	void setStatus(UINT idMsg);

protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	LRESULT OnRUICMD_RestartEncoder(WPARAM wParam, LPARAM lParam);
	LRESULT OnRUICMD_StartStreaming  (WPARAM wParam, LPARAM lParam);
	LRESULT OnUpdateConnection(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	void ShowFirstRun(bool bShow);
	afx_msg void OnBnClickedButtonStart();
	CStatic m_status;
	CEdit m_talog;
	afx_msg void OnBnClickedButtonStop();
	CListBox m_devList;
	void lock_control(bool bSet);
	void lock_control_all(bool bSet);
	CEdit m_taName;
	afx_msg void OnBnClickedButtonPreview();
	afx_msg LRESULT OnPreviewWindowClosed(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonChangeTaserial();
	CIPAddressCtrl m_taIP;
	CEdit m_taPort;
	CStatic m_taInternalIP;
	LRESULT RefreshTAState(WPARAM wParam, LPARAM lParam);
	LRESULT taInit(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonRescan();
	afx_msg void OnBnClickedCheckAutostart();
	CButton m_bAutoStart;
	CComboBox m_audioList;
	CButton m_bForceMonkey;
	afx_msg void OnBnClickedCheckForceMonkey();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};

//--- util.cpp
int Message(UINT idMsg, UINT type=MB_OK);

