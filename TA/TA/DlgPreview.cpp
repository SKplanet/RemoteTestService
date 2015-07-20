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


// DlgPreview.cpp : implementation file
//

#include "stdafx.h"
#include "TA.h"
#include "DlgPreview.h"
#include "afxdialogex.h"


// CDlgPreview dialog

IMPLEMENT_DYNAMIC(CDlgPreview, CDialogEx)


CDlgPreview::CDlgPreview(RA* pRA, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgPreview::IDD, pParent)
{
	m_pra = pRA;
}

CDlgPreview::~CDlgPreview()
{
	m_pra->SetAdminUpdateCompleteCallback(NULL, NULL);
}

void CDlgPreview::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PREVIEW, m_preview);
	//DDX_Control(pDX, IDC_EDIT_LOG, m_log);
}


BEGIN_MESSAGE_MAP(CDlgPreview, CDialogEx)
//	ON_STN_CLICKED(IDC_STATIC_PREVIEW, &CDlgPreview::OnStnClickedStaticPreview)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_COMMAND(ID_CMD_SCREENSHOT, &CDlgPreview::OnCmdScreenshot)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_CMD_WAKE, &CDlgPreview::OnCmdWake)
	ON_COMMAND(ID_CMD_SHELL, &CDlgPreview::OnCmdShell)
	ON_COMMAND(ID_CMD_ROTATE, &CDlgPreview::OnCmdRotate)
	ON_COMMAND(ID_CMD_HOME, &CDlgPreview::OnCmdHome)
	ON_COMMAND(ID_CMD_MENU, &CDlgPreview::OnCmdMenu)
	ON_COMMAND(ID_CMD_BACK, &CDlgPreview::OnCmdBack)
	ON_COMMAND(ID_CMD_VOLUP, &CDlgPreview::OnCmdVolUp)
	ON_COMMAND(ID_CMD_VOLDN, &CDlgPreview::OnCmdVolDn)
END_MESSAGE_MAP()


// CDlgPreview message handlers


BOOL CDlgPreview::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	/*
	CRect rect;
	CWnd* pWnd = GetDlgItem(IDC_STATIC_SPLIT);
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect);
	pWnd->DestroyWindow();
	*/
#ifdef USE_TOOLBAR
	if(!m_wndToolBar.Create(this) || !m_wndToolBar.LoadToolBar(IDR_TOOLBAR_PREVIEW))
	{
		//TRACE("failed to create toolbar");
		EndDialog(IDCANCEL);
	}
#endif
	CRect	rcClientOld; // Old Client Rect
	CRect	rcClientNew; // New Client Rect with Tollbar Added
	GetClientRect(rcClientOld); // Retrive the Old Client WindowSize

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST,
	 AFX_IDW_CONTROLBAR_LAST,0,reposQuery,rcClientNew);

	CPoint ptOffset(rcClientNew.left-rcClientOld.left,
				rcClientNew.top-rcClientOld.top);
	CRect	rcChild;
	// Handle to the Dialog Controls
	CWnd*	pwndChild = GetWindow(GW_CHILD);  
	while(pwndChild) // Cycle through all child controls
	{
	 pwndChild->GetWindowRect(rcChild); // Get the child control RECT
	 ScreenToClient(rcChild); 
	// Changes the Child Rect by the values of the claculated offset
	 rcChild.OffsetRect(ptOffset); 
	  pwndChild->MoveWindow(rcChild,FALSE); // Move the Child Control
	  pwndChild = pwndChild->GetNextWindow();
	}

	CRect	rcWindow;
	GetWindowRect(rcWindow); // Get the RECT of the Dialog
	// Increase width to new Client Width
	rcWindow.right += rcClientOld.Width() - rcClientNew.Width(); 
	// Increase height to new Client Height
	rcWindow.bottom += rcClientOld.Height() - rcClientNew.Height(); 
	MoveWindow(rcWindow,FALSE); // Redraw Window
	// Now we REALLY Redraw the Toolbar
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST,
	 AFX_IDW_CONTROLBAR_LAST,0);

	//int nitemCount = m_wndToolBar.GetCount();
	//int nID;
	//for(int idx = 0; idx < nitemCount; idx++)
	//{
		//nID = m_wndToolBar.GetItemID(idx);
		//TRACE("toolbar id (%d/%d):%d\n", idx, nitemCount, nID);
	//}

	/*
	m_split.Create(
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | SS_HORIZ,
		this,
		&m_preview,
		&m_log,
		IDC_STATIC_SPLIT,
		rect,
		300,50);
	if( m_split.GetSafeHwnd() ) {
		CRect rect;
		GetClientRect(&rect);
		rect.DeflateRect(7,7);
		rect.top += TOOLBARHIGHT;
		m_split.MoveWindow(&rect);
	}
	*/

	m_preview.setRA(m_pra);
	
	SetTimer(1, 100, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
}


//void CDlgPreview::OnStnClickedStaticPreview()
//{
//	// TODO: Add your control notification handler code here
//}


void CDlgPreview::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	CRect rect;
	GetClientRect(&rect);
	/*
	if( m_split.GetSafeHwnd() ) {
		rect.DeflateRect(7,7);
		rect.top += TOOLBARHIGHT;
		m_split.MoveWindow(&rect);
	}
	*/
	if( m_preview.GetSafeHwnd() )
	{
		rect.top += TOOLBARHIGHT;
		m_preview.MoveWindow(&rect);
	}
}


void CDlgPreview::OnTimer(UINT_PTR nIDEvent)
{
	if( nIDEvent == 1 )
	{
		m_pra->update_req();
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CDlgPreview::OnPaint()
{
	CPaintDC dc(this); // device context for painting				
}


void CDlgPreview::OnDestroy()
{
	CDialogEx::OnDestroy();

	CWnd* p = GetParent();
	if( p && p->GetSafeHwnd() )
		p->PostMessage(WM_PREVIEW_CLOSED, 0, 0);
	delete this;
}


void CDlgPreview::OnClose()
{
	DestroyWindow();
}

BOOL CDlgPreview::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	static int pressid = 0;

	if( wParam == AFX_IDW_CONTROLBAR_FIRST )
	{
		NMHDR *nh;
		nh = (NMHDR*)lParam;
		//TRACE("wp:%x, code:%x, wndfrom:%x, id:%d\n",wParam, nh->code, nh->hwndFrom, nh->idFrom);
		if( nh->code == NM_LDOWN )
		{
			LPNMCLICK pNMClick = reinterpret_cast<LPNMCLICK>(nh);

			if( pNMClick->dwItemSpec  == ID_CMD_HOME)
			{
				pressid = ID_CMD_HOME;
				m_pra->key_home(true);
			}
			else if(pNMClick->dwItemSpec == ID_CMD_MENU )
			{
				pressid = ID_CMD_MENU;
				m_pra->key_menu(true);
			}
			else if(pNMClick->dwItemSpec == ID_CMD_BACK )
			{
				pressid = ID_CMD_BACK;
				m_pra->key_back(true);
			}
			else if(pNMClick->dwItemSpec == ID_CMD_VOLUP )
			{
				pressid = ID_CMD_VOLUP;
				m_pra->key_volup(true);
			}
			else if(pNMClick->dwItemSpec == ID_CMD_VOLDN )
			{
				pressid = ID_CMD_VOLDN;
				m_pra->key_voldown(true);
			}

		}
		if( nh->code == NM_RELEASEDCAPTURE)
		{
			//TRACE("RELEASED\n");
			if( pressid == ID_CMD_HOME )
			{
				m_pra->key_home(false);
			}
			else if( pressid == ID_CMD_MENU)
			{
				m_pra->key_menu(false);
			}
			else if( pressid == ID_CMD_BACK)
			{
				m_pra->key_back(false);
			}
			else if( pressid == ID_CMD_VOLUP)
			{
				m_pra->key_volup(false);
			}
			else if( pressid == ID_CMD_VOLDN)
			{
				m_pra->key_voldown(false);
			}

			pressid = 0;
		}

	}

	return CDialogEx::OnNotify(wParam, lParam, pResult);
}

void CDlgPreview::OnCmdHome()
{
	if( m_pra )
	{
		m_pra->key_home(true);
		m_pra->key_home(false);
	}
}

void CDlgPreview::OnCmdMenu()
{
	if( m_pra )
	{
		m_pra->key_menu(true);
		m_pra->key_menu(false);
	}
}

void CDlgPreview::OnCmdBack()
{
	if( m_pra )
	{
		m_pra->key_back(true);
		m_pra->key_back(false);
	}
}

void CDlgPreview::OnCmdVolUp()
{
	if( m_pra )
	{
		m_pra->key_volup(true);
		m_pra->key_volup(false);
	}
}

void CDlgPreview::OnCmdVolDn()
{
	if( m_pra )
	{
		m_pra->key_voldown(true);
		m_pra->key_voldown(false);
	}
}

void CDlgPreview::OnCmdScreenshot()
{
	char fname[MAX_PATH];
	fname[0] = 0;
	if( m_pra->screenshot(fname) > 0 )
	{
		CString msg;
		msg.Format(IDS_SCREENSHOT_SAVED, fname);
		AfxMessageBox(msg);
	}
}


void CDlgPreview::OnCmdWake()
{
#ifdef TEST
	static int rotate;
	if( m_pra ) {
		//m_pra->key_wake();
		rotate++;
		rotate %= 5;
		m_pra->tdc_setRotate(rotate);
	}
#else
	if( m_pra )
		m_pra->key_wake();
		//m_pra->key_power(1);
		//m_pra->key_power(0);
#endif
}


void CDlgPreview::OnCmdShell()
{
	if( m_pra )
		m_pra->adb_command("shell", 1);
}


void CDlgPreview::OnCmdRotate()
{
	static char r = 1;

	if( m_pra )
	{
		m_pra->tdc_setRotate(r++ % 5);
	}
}
