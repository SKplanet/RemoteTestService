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


// TADlg.cpp : implementation file
//

#include "stdafx.h"
#include "TA.h"
#include "TADlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include "DlgPreview.h"
#include "../rui/RUIServerPeer.h"
#include "DlgSerial.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT WM_TA_INIT			= ::RegisterWindowMessage(_T("WM_TA_INIT" ));
UINT WM_TA_STATEUPDATE	= ::RegisterWindowMessage(_T("WM_TA_STATEUPDATE" ));
UINT WM_TA_LOG			= ::RegisterWindowMessage(_T("WM_TA_LOG"));
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEditCut();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_COMMAND(ID_EDIT_CUT, &CAboutDlg::OnEditCut)
END_MESSAGE_MAP()


// CTADlg dialog


CString getIPAddress();
CString GetVersionInfo(HMODULE hLib);

CTADlg::CTADlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTADlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pra = NULL;
	m_preview = NULL;
}

CTADlg::~CTADlg()
{
		//--- set TA log callback
	m_pra->SetIncomingTALogCallback(0,0);
	m_pra->exit_ra();
	delete m_pra; m_pra = NULL;
}

void CTADlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_status);
	DDX_Control(pDX, IDC_EDIT_TALOG, m_talog);
	DDX_Control(pDX, IDC_LIST_DEVICE, m_devList);
	DDX_Control(pDX, IDC_EDIT_TANAME, m_taName);
	DDX_Control(pDX, IDC_IPADDRESS, m_taIP);
	DDX_Control(pDX, IDC_IPPORT, m_taPort);
	DDX_Control(pDX, IDC_INFO_INTERNALIP, m_taInternalIP);
	DDX_Control(pDX, IDC_CHECK_AUTOSTART, m_bAutoStart);
	DDX_Control(pDX, IDC_COMBO_AUDIO, m_audioList);
	DDX_Control(pDX, IDC_CHECK_FORCE_MONKEY, m_bForceMonkey);
}

BEGIN_MESSAGE_MAP(CTADlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CTADlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CTADlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_PREVIEW, &CTADlg::OnBnClickedButtonPreview)
	ON_MESSAGE(WM_PREVIEW_CLOSED, &CTADlg::OnPreviewWindowClosed)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_TASERIAL, &CTADlg::OnBnClickedButtonChangeTaserial)
	ON_REGISTERED_MESSAGE(WM_RUIC_RESTART_ENCODER   , OnRUICMD_RestartEncoder  )
	ON_REGISTERED_MESSAGE(WM_RUIC_START_STREAMING   , OnRUICMD_StartStreaming  )
	ON_REGISTERED_MESSAGE(WM_UPDATE_CONNECTION      , OnUpdateConnection       )
	ON_BN_CLICKED(IDC_BUTTON_RESCAN, &CTADlg::OnBnClickedButtonRescan)
	ON_BN_CLICKED(IDC_CHECK_AUTOSTART, &CTADlg::OnBnClickedCheckAutostart)
	ON_BN_CLICKED(IDC_CHECK_FORCE_MONKEY, &CTADlg::OnBnClickedCheckForceMonkey)
	ON_REGISTERED_MESSAGE(WM_TA_INIT , taInit )
	ON_REGISTERED_MESSAGE(WM_TA_STATEUPDATE , RefreshTAState )
	ON_REGISTERED_MESSAGE(WM_TA_LOG , OnTALog )
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	ON_WM_MOUSEACTIVATE()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CTADlg message handlers

BOOL CTADlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//SetTimer(0x0012, 1000, NULL);
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	CString title; title.LoadString(IDS_TA);
	title.Append(" (");
	title.Append(GetVersionInfo(0));
	title.Append(")");
	this->SetWindowText(title);
	
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	setStatus(IDS_RA_INITIALIZE);

	PostMessage(WM_TA_INIT,0,0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTADlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTADlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTADlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTADlg::lock_control(bool bSet)
{
	m_devList.EnableWindow(!bSet);
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(!bSet);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(bSet);
	GetDlgItem(IDC_BUTTON_PREVIEW)->EnableWindow(bSet);
	GetDlgItem(IDC_BUTTON_RESCAN)->EnableWindow(!bSet);
	GetDlgItem(IDC_BUTTON_CHANGE_TASERIAL)->EnableWindow(!bSet);
	GetDlgItem(IDC_IPADDRESS)->EnableWindow(!bSet);
	GetDlgItem(IDC_IPPORT)->EnableWindow(!bSet);
	GetDlgItem(IDC_COMBO_AUDIO)->EnableWindow(!bSet);
	GetDlgItem(IDC_CHECK_FORCE_MONKEY)->EnableWindow(!bSet);
}

void CTADlg::lock_control_all(bool bSet)
{
	m_devList.EnableWindow(bSet);
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(bSet);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(bSet);
	GetDlgItem(IDC_BUTTON_PREVIEW)->EnableWindow(bSet);
	GetDlgItem(IDC_BUTTON_RESCAN)->EnableWindow(bSet);
	GetDlgItem(IDC_BUTTON_CHANGE_TASERIAL)->EnableWindow(bSet);
	GetDlgItem(IDC_IPADDRESS)->EnableWindow(bSet);
	GetDlgItem(IDC_IPPORT)->EnableWindow(bSet);
	GetDlgItem(IDC_COMBO_AUDIO)->EnableWindow(bSet);
	GetDlgItem(IDC_CHECK_FORCE_MONKEY)->EnableWindow(bSet);
}

LRESULT CTADlg::RefreshTAState(WPARAM wParam, LPARAM lParam)
{
	int bDev = (int)wParam;

	if( bDev )
	{
		CString buf;

		buf.Format("%d", m_pra->m_fbcWidth);
		SetDlgItemText(IDC_INFO_WIDTH, buf);

		buf.Format("%d", m_pra->m_fbcHeight);
		SetDlgItemText(IDC_INFO_HEIGHT, buf);

		buf.Format("%d", m_pra->m_fbcBPP);
		SetDlgItemText(IDC_INFO_BPP, buf);
		if( m_pra->m_state >= TA_STATE_START_SERVICE && m_pra->m_state < TA_STATE_SERVICE_STOP)
			lock_control(true);

		SetDlgItemText(IDC_INFO_MODEL, m_pra->m_devModel);
		SetDlgItemText(IDC_INFO_MANUFACTURER, m_pra->m_devManufacture);
		SetDlgItemText(IDC_INFO_BUILD, m_pra->m_devBuild);
		SetDlgItemText(IDC_INFO_SDK, m_pra->m_devSDKVer);
		SetDlgItemText(IDC_INFO_SERIAL, m_pra->m_devBuildSerial);
		SetDlgItemText(IDC_INFO_IMEI, m_pra->m_devIMEI);
		SetDlgItemText(IDC_INFO_USIM, m_pra->m_devUSIM);
		SetDlgItemText(IDC_INFO_USIM, m_pra->m_devSIMSerial);
		SetDlgItemText(IDC_INFO_PHONENO, m_pra->m_devPhoneNO);

		buf.Format("%4.1f", m_pra->m_devTemperature / 10.0f);
		SetDlgItemText(IDC_INFO_TEMP, buf);

		buf.Format("%d %%", m_pra->m_devBattLvl);
		SetDlgItemText(IDC_INFO_BATLEVEL, buf);

		if( m_pra->m_devStatus & TDC_STATUS_SCREENON )
		{
			SetDlgItemText(IDC_INFO_SCREEN, "ON");
		}
		else
		{
			SetDlgItemText(IDC_INFO_SCREEN, "OFF");
		}

		if( m_pra->m_fbcTouch  )
		{
			SetDlgItemText(IDC_INFO_EVENTMETHOD, "RAW");
		}
		else
		{
			SetDlgItemText(IDC_INFO_EVENTMETHOD, "Monkey");
		}

				
		
		buf = "";
		if ( m_pra->m_devStatus & TDC_STATUS_ROTATEAUTO )
			buf = "Auto ";
		
		int nRotate = m_pra->m_devStatus & TDC_STATUS_ROTATE;
		switch( nRotate )
		{
		case 0 : buf += "(0)"; break;
		case 1 : buf += "(90)"; break;
		case 2 : buf += "(180)"; break;
		case 3 : buf += "(270)"; break;
		}

		SetDlgItemText(IDC_INFO_ROTATION, buf);

	// Command Socket
	}
	else
	{
		// IDC_STATIC_SYSTIME
		CString time;
		SYSTEMTIME st;
		::GetLocalTime(&st);
		time.Format("%d-%d-%d %d:%d:%d",
			st.wYear,
			st.wMonth,
			st.wDay,
			st.wHour,
			st.wMinute,
			st.wSecond);
		SetDlgItemText(IDC_STATIC_SYSTIME, time);

		// IDC_STATIC_FBC
		if( m_pra->m_hfbcThread )
			SetDlgItemText(IDC_STATIC_FBC, "Working");
		else
			SetDlgItemText(IDC_STATIC_FBC, "Stopped");

		// IDC_STATIC_TDC
		if( m_pra->m_htdcThread )
			SetDlgItemText(IDC_STATIC_TDC, "Working");
		else
			SetDlgItemText(IDC_STATIC_TDC, "Stopped");

		// IDC_STATIC_EVT
		if( m_pra->m_monkeysock != INVALID_SOCKET )
			SetDlgItemText(IDC_STATIC_EVT, "Working");
		else
			SetDlgItemText(IDC_STATIC_EVT, "Stopped");
		
		// IDC_STATIC_CLIENT 
		if( m_pra->m_state == TA_STATE_CLIENT_CONNECTED )
		{
			SetDlgItemText(IDC_STATIC_CLIENT, m_pra->m_clientIP);
			SetDlgItemText(IDC_INFO_CLIENT_ID, m_pra->m_clientReserveID);
			setStatus(IDS_RA_CONNECTED);
			CString fps;
			fps.Format("%5.2f (%5.2f)kBps", m_pra->m_serverFps, m_pra->m_serverBps);
			SetDlgItemText(IDC_INFO_FPS, fps);

			fps.Format("%5.2f (%5.2f)kBps", m_pra->m_clientFps, m_pra->m_clientBps);
			SetDlgItemText(IDC_INFO_CLIENT_FPS, fps);

			fps = DwordToTimedate(m_pra->m_clientValidStart);
			SetDlgItemText(IDC_STATIC_VALID_FROM, fps);

			fps = DwordToTimedate(m_pra->m_clientValidEnd);
			SetDlgItemText(IDC_STATIC_VALID_TO, fps);
			
		}
		else {
			SetDlgItemText(IDC_STATIC_CLIENT, "not connected");
			SetDlgItemText(IDC_INFO_CLIENT_ID, "");

			if( m_pra->m_state < TA_STATE_READY )
				setStatus(IDS_RA_INITIALIZE);
			else if( m_pra->m_state <= TA_STATE_AFTER_CLIENT_REINIT)
				setStatus(IDS_RA_WAITCLIENT);
			else if( m_pra->m_state > TA_STATE_AFTER_CLIENT_REINIT)
				setStatus(IDS_RA_STOPPED);
			SetDlgItemText(IDC_INFO_FPS, "");
			SetDlgItemText(IDC_INFO_CLIENT_FPS, "");
			SetDlgItemText(IDC_STATIC_VALID_FROM, "");
			SetDlgItemText(IDC_STATIC_VALID_TO, "");
		}

		if( m_pra->m_opt_screen ) // h.264
		{
			CString method;
			method = "H.264 ";
			if( m_pra->m_opt_h264_quality == 1 )
				method += "(Low)";
			else if( m_pra->m_opt_h264_quality == 2 )
				method += "(High)";
			else
				method += "(Normal)";

			SetDlgItemText(IDC_STATIC_SCRMETHOD, method);
		}
		else
		{
			SetDlgItemText(IDC_STATIC_SCRMETHOD, "Image");
		}

		if( m_pra->m_bStreamingAudio ) // h.264
			SetDlgItemText(IDC_STATIC_AUDIO, "ON");
		else
			SetDlgItemText(IDC_STATIC_AUDIO, "OFF");

		

	}

	return 0;
}


LRESULT CTADlg::taInit(WPARAM wParam, LPARAM lParam)
{
	lock_control_all(0);
	//--- Create RuiAgent
	m_pra = new RA(this);
	
	//--- set TA log callback
	m_pra->SetIncomingTALogCallback(OnImcomingTALog, this);

	//--- set TA state callback
	m_pra->SetTAStateReportCallback(OnTAState, this);

	//--- Init RA
	if( m_pra->init_ra() < 1 )
	{
		m_pra->DeviceStatusToPOC(8, 1, "Service Init Failed");
		m_pra->set_state(TA_STATE_ERR_INIT_RA);
		MessageBeep(-1);
	}

	//-- restore window position
	{
		char inifile[MAX_PATH];
		sprintf_s(inifile, "%s\\%s", m_pra->m_ModulePath, RUIINI);

		int x, y;
		x = GetPrivateProfileInt("TA", "x", -1, inifile);
		y  =  GetPrivateProfileInt("TA", "y", -1, inifile);
		if( x > 0 && y > 0 )
		{
			//m_pra->ta_log("\tRestore windows pos(%d,%d)", x, y);
			ShowWindow(SW_SHOW);
			SetWindowPos(&wndTopMost, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
		}
	}
	m_devList.ResetContent();
	for(int i=0; i< MAX_TD; i++)
	{
		if( m_pra->m_devSerial[i][0] )
			m_devList.AddString((LPCSTR)m_pra->m_devSerial[i]);
		else
			break;
	}

	m_audioList.ResetContent();
	CWaveInDevice* pDev = m_pra->m_waveInDeviceList.GetFirst();
	while( pDev )
	{
		m_audioList.AddString(pDev->GetDeviceName());
		pDev = m_pra->m_waveInDeviceList.GetNext(pDev);
	}

	m_bAutoStart.SetCheck(m_pra->m_bAutoStart);
	m_bForceMonkey.SetCheck(m_pra->m_bForceMonkey);

	//--- set control
	m_taName.SetWindowText(m_pra->m_taSerial);

	if( m_pra->m_devIndex >= 0 )
		m_devList.SetCurSel(m_pra->m_devIndex);
	else {
		//Message(IDS_TD_NOT_SET, MB_OK);
	}

	//m_pra->ta_log("audioIndex:%d", m_pra->m_audioIndex);
	if( m_pra->m_audioIndex >= 0 )
		m_audioList.SetCurSel(m_pra->m_audioIndex);

	m_taInternalIP.SetWindowText(getIPAddress());

	m_talog.SetLimitText(655350);

	m_taIP.SetAddress(m_pra->m_devExternalIP);
	char intbuf[12];
	sprintf_s(intbuf,"%d", m_pra->m_devExternalPort);
	m_taPort.SetWindowText( intbuf);
	lock_control(false);

	setStatus(IDS_RA_STOPPED);

	if(m_pra->m_bFirstRun)
	{
		ShowFirstRun(1);
	}
	if( m_pra->m_bAutoStart && m_pra->m_devIndex >= 0 && m_devList.GetCurSel() !=  LB_ERR)
	{
		//Sleep(1000);
		m_pra->ta_log("[Auto Start]");
		m_pra->ShowProgress(1, "Prepare Start Service", 1);
		Sleep(1000);
		m_pra->ShowProgress(0);
		OnBnClickedButtonStart();
	}

	return 0;
}

void CTADlg::ShowFirstRun(bool bShow)
{
	GetDlgItem(IDC_STATIC_1)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_STATIC_2)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_STATIC_3)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_STATIC_4)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_STATIC_5)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
}

void CTADlg::OnBnClickedButtonStart()
{
	if( (m_pra->m_devIndex = m_devList.GetCurSel()) ==  LB_ERR ) {
		m_pra->set_state(TA_STATE_ERR_DEVNOTFOUND);
		m_pra->DeviceStatusToPOC(8, 0, "Device not found error");
		Message(IDS_TD_NOT_SET, MB_OK | MB_ICONSTOP);
		return;
	}

	m_pra->ta_log("\t### Start Service Condition #####");
	m_pra->ta_log("\t\tTA Serial   \t: %s", m_pra->m_taSerial);
	m_pra->ta_log("\t\tADB Device  \t: %s", m_pra->m_devSerial[m_pra->m_devIndex]);
	m_pra->m_audioIndex = m_audioList.GetCurSel();
	if( m_pra->m_audioIndex >= 0 )
	{
		m_pra->ta_log("\t\tAudio Device\t: %s", m_pra->m_waveInDeviceList.GetAt(m_pra->m_audioIndex)->GetDeviceName());
		lstrcpy(m_pra->m_audioName, m_pra->m_waveInDeviceList.GetAt(m_pra->m_audioIndex)->GetDeviceName());
	}
	else
		m_pra->ta_log("\t\tAudio Device\t: WAVE_MAPPER(-1)");

	DWORD dwAddress;
	m_taIP.GetAddress(dwAddress);
	if( dwAddress == 0 )
	{
		m_pra->ta_log("ERR GetIP Address Failed");
		Message(IDS_IP_NOT_SET, MB_OK | MB_ICONSTOP);
		return;
	}
	CString csBuf;
	m_taIP.GetAddress(m_pra->m_devExternalIP);

	m_taPort.GetWindowText(csBuf);
	m_pra->m_devExternalPort = atoi(csBuf);

	IN_ADDR ip;
	ip.S_un.S_addr = m_pra->m_devExternalIP;
	m_pra->ta_log("\t\tListen IP   \t:%d.%d.%d.%d(%d)", ip.S_un.S_un_b.s_b4, ip.S_un.S_un_b.s_b3, ip.S_un.S_un_b.s_b2, ip.S_un.S_un_b.s_b1, m_pra->m_devExternalPort);
	m_pra->ta_log("\t#################################");


	setStatus(IDS_RA_STARTING);

	m_pra->m_bFirstRun = 0;
	ShowFirstRun(0);
	m_pra->save_ini();

	CWaitCursor waitcursor;
	lock_control(true);
	// start rui server
	if( m_pra->start_ta_service() <= 0 )
	{
		m_pra->set_state(TA_STATE_ERR_START_SERVICE);
		m_pra->DeviceStatusToPOC(8, 1, "Service Error");
		Message(IDS_TA_START_FAILED, MB_OK | MB_ICONSTOP);
		setStatus(IDS_RA_STOPPED);
		lock_control(false);
	} else {
		CString title; title.LoadString(IDS_TA);
		title.Append(" (");
		title.Append(GetVersionInfo(0));
		title.Append(")-(");
		title.Append(m_pra->m_taSerial);
		title.Append(":");
		CString buf;
		buf.Format("%d)",m_pra->m_devExternalPort);
		title.Append(buf);
		this->SetWindowText(title);
		setStatus(IDS_RA_WAITCLIENT);
		PostMessage(WM_TA_STATEUPDATE,0,0);
		//RefreshTAState();
	}
}


void CTADlg::OnBnClickedButtonStop()
{
	if( m_preview != NULL )
	{
		m_preview->DestroyWindow();
		m_preview = NULL;
	}

	CString title; title.LoadString(IDS_TA);
	title.Append(" (");
	title.Append(GetVersionInfo(0));
	title.Append(")");
	this->SetWindowText(title);


	m_pra->stop_ta_service();
	lock_control(false);
	m_pra->set_state(TA_STATE_SERVICE_STOP_DONE);
	setStatus(IDS_RA_STOPPED);
}

LRESULT CTADlg::OnTALog(WPARAM wParam, LPARAM lParam)
//void CTADlg::OnTALog(BYTE* buffer, int size)
{
	BYTE* buffer = (BYTE*)wParam;
	int size = (int)lParam;

	if( m_talog.GetSafeHwnd() == 0 )
		return 0;

	char buf[2048];
	//char *buf = (char*)malloc(size+1);
	_ASSERTE( _CrtCheckMemory( ) );
	memcpy(buf, (char*)buffer, size);
	buf[size] = 0;

	free(buffer);
	int nLength = m_talog.GetWindowTextLength();
	if( nLength + size > 32768 )
	{
		int nline = m_talog.GetLineCount();
		int nchar = m_talog.LineIndex(nline/2);
		m_talog.SetSel(0, nchar, 1);
		m_talog.ReplaceSel("\r\n");
		nLength = m_talog.GetWindowTextLength();
	}
	m_talog.SetSel(nLength+1, -1,1);
	m_talog.ReplaceSel(buf);
	m_talog.LineScroll(m_talog.GetLineCount());
	//free(buf);

	return 1;
}

void CTADlg::setStatus(UINT idMsg)
{
	CString msg;
	msg.LoadString(idMsg);
	m_status.SetWindowText(msg);
}

void CTADlg::OnBnClickedButtonPreview()
{
	if( GetKeyState(VK_CONTROL) & 0x8000 )
	{
		m_pra->ta_log("[force reinit for test]");
		m_pra->m_bInitRequiredTime = timedateToDword();
		m_pra->m_bInitRequired = 1;
		return;
	}

	//m_pra->ShutdownSocket(m_pra->m_clientSock);
	//m_pra->m_clientSock = INVALID_SOCKET;
	//return;

	if( m_preview )
	{
		m_preview->SetWindowPos(&wndTopMost, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
		return;
	}
	m_preview = new CDlgPreview(m_pra, this);
	m_preview->Create(CDlgPreview::IDD);
	m_preview->ShowWindow(SW_SHOW);
}

LRESULT CTADlg::OnPreviewWindowClosed(WPARAM wParam, LPARAM lParam)
{
	//delete m_preview;
	m_preview = NULL;

	return 0;
}

void CTADlg::OnBnClickedButtonChangeTaserial()
{
	DlgSerial dlg;
	m_taName.GetWindowText(dlg.m_Serial);

	if( dlg.DoModal() == IDOK )
	{
		m_taName.SetWindowText(dlg.m_Serial);
		lstrcpy(m_pra->m_taSerial, dlg.m_Serial);
		m_pra->save_ini();
	}
	//IDC_EDIT_TANAME
}

LRESULT CTADlg::OnRUICMD_RestartEncoder(WPARAM wParam, LPARAM lParam)
{
	//m_pra->ta_log(_T("[/RestartEncoder]"));

	m_pra->Video_StopEncoder();
	m_pra->Video_StartEncoder();
//	m_pra->m_bCaptureFlag = TRUE;

	return 0;
}


LRESULT CTADlg::OnRUICMD_StartStreaming(WPARAM wParam, LPARAM lParam)
{
	BYTE			nField0, nField1, nField2, nField3;
	RUIServerPeer*	pPeer = (RUIServerPeer*) lParam;
	if (pPeer != NULL)
	{
		pPeer->SetClientStreaming(TRUE);

		pPeer->GetPeerIP(nField0, nField1, nField2, nField3);
	}

	//m_pra->ta_log(_T("[%d.%d.%d.%d/StartStreaming]"), nField0, nField1, nField2, nField3);

	return 0;
}

// [neuromos] !이것을 RUITCPSockServer로 옮기고 싶으나 PostMessage 이외의 방법이 애매하다.
LRESULT CTADlg::OnUpdateConnection(WPARAM wParam, LPARAM lParam) // TCP 소켓에서만 사용됨
{
	UINT			uEvent = (UINT) wParam;
	RUIServerPeer*	pPeer  = reinterpret_cast <RUIServerPeer*> (lParam);

	if (pPeer != NULL)
	{
		if (uEvent == EVT_CONSUCCESS) // 접속에 성공하였다면 다른 서버를 시작한다.
		{
			BYTE	nField0;
			BYTE	nField1;
			BYTE	nField2;
			BYTE	nField3;
			pPeer->GetPeerIP(nField0, nField1, nField2, nField3);

			CString	strIP;
			strIP.Format(_T("%d.%d.%d.%d"), nField0, nField1, nField2, nField3);

			//m_pra->ta_log(_T("Client Connected. (%s)"), (LPCTSTR) strIP);

			// [neuromos] RUITCPSockPeer::RUITCPSockPeer()에서 호출한다. 메세지가 먼저 온다면 오류 발생!
//			pPeer->ResetSyncTick();


			if (! m_pra->m_bUseUDP)
				Sleep(100);
			else
				m_pra->Video_CreateUDPSock(pPeer, (LPCTSTR) strIP);

			InitIppExtensionParam_Connect(&(m_pra->m_ieParam));
			m_pra->m_ieParam.m_pEncoder = (void*) m_pra->m_threadEncoder.GetEncoder();

			//m_pra->m_TCPSockServer.StartServer();
		}

		else if (uEvent == EVT_CONFAILURE || uEvent == EVT_CONDROP)
		{
			if( m_pra->m_state < TA_STATE_SERVICE_STOP )
			{
				BYTE	nField0;
				BYTE	nField1;
				BYTE	nField2;
				BYTE	nField3;
				pPeer->GetPeerIP(nField0, nField1, nField2, nField3);

				//m_pra->ta_log(_T("Client Disconnected. (%d.%d.%d.%d)"), nField0, nField1, nField2, nField3);

				m_pra->Video_DestroyUDPSock(pPeer);
				pPeer->StopComm();
				//m_pra->m_TCPSockServer.StartServer();
			}
		}
	}

	return 1L;
}

CString getIPAddress()
{
	CString str;
	str.Empty();
 	//if ( gethostname("localhost", 256) == 0)
 	{
		PHOSTENT HostInfo;
		char* IPAdd;
		//printf("Host name: %s\n", name);
		if((HostInfo = gethostbyname("")) != NULL)
		{
			int nCount = 0;
			while(HostInfo->h_addr_list[nCount])
			{
				IPAdd = inet_ntoa(*(struct in_addr *)HostInfo->h_addr_list[nCount]);
 
				if(nCount > 0 ) str.Append(" / ");
				str.Append(IPAdd);
				++nCount;
			}
		}
 	}
	return str;
}

CString GetVersionInfo(HMODULE hLib)
{
  CString csRet;

  if (hLib == NULL)
    hLib = AfxGetResourceHandle();
  
  HRSRC hVersion = FindResource( hLib, 
    MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION );
  if (hVersion != NULL)
  {
    HGLOBAL hGlobal = LoadResource( hLib, hVersion ); 
    if ( hGlobal != NULL)  
    {  
  
      LPVOID versionInfo  = LockResource(hGlobal);  
      if (versionInfo != NULL)
      {
        DWORD vLen,langD;
        BOOL retVal;    
    
        LPVOID retbuf=NULL;
    
        static char fileEntry[256];

        sprintf_s(fileEntry,"\\VarFileInfo\\Translation");
        retVal = VerQueryValue(versionInfo,fileEntry,&retbuf,(UINT *)&vLen);
        if (retVal && vLen==4) 
        {
          memcpy(&langD,retbuf,4);            
          sprintf_s(fileEntry, "\\StringFileInfo\\%02X%02X%02X%02X\\FileVersion",
                  (langD & 0xff00)>>8,langD & 0xff,(langD & 0xff000000)>>24, 
                  (langD & 0xff0000)>>16);            
        }
        else 
          sprintf_s(fileEntry, "\\StringFileInfo\\%04X04B0\\FileVersion", 
            GetUserDefaultLangID());

        if (VerQueryValue(versionInfo,fileEntry,&retbuf,(UINT *)&vLen)) 
          csRet = (char*)retbuf;
      }
    }

    UnlockResource( hGlobal );  
    FreeResource( hGlobal );  
  }

  return csRet;
}

void CAboutDlg::OnEditCut()
{
	// TODO: Add your command handler code here
}



void CTADlg::OnBnClickedButtonRescan()
{
	CWaitCursor waitcursor;

	m_devList.ResetContent();
	m_audioList.ResetContent();

	m_pra->ScanDevices();
	for(int i=0; i< MAX_TD; i++)
	{
		if( m_pra->m_devSerial[i][0] )
			m_devList.AddString((LPCSTR)m_pra->m_devSerial[i]);
		else
			break;
	}

	CWaveInDevice* pDev = m_pra->m_waveInDeviceList.GetFirst();
	while( pDev )
	{
		m_audioList.AddString(pDev->GetDeviceName());
		pDev = m_pra->m_waveInDeviceList.GetNext(pDev);
	}

	if( m_pra->m_devIndex >= 0 )
		m_devList.SetCurSel(m_pra->m_devIndex);
	else {
		Message(IDS_TD_NOT_SET, MB_OK);
	}

	if( m_pra->m_audioIndex >= 0 )
		m_audioList.SetCurSel(m_pra->m_audioIndex);

	m_pra->save_ini();
}


void CTADlg::OnBnClickedCheckAutostart()
{
	m_pra->m_bAutoStart = m_bAutoStart.GetCheck();

	m_pra->save_ini();
}


void CTADlg::OnBnClickedCheckForceMonkey()
{
	m_pra->m_bForceMonkey = m_bForceMonkey.GetCheck();

	m_pra->save_ini();
}


void CTADlg::OnDestroy()
{
	CRect rt;

	GetWindowRect(rt);
	TRACE("%d,%d\n", rt.left, rt.top);
	if( m_pra )
	{
		char inifile[MAX_PATH];
		char intbuf[64];
		sprintf_s(inifile, "%s\\%s", m_pra->m_ModulePath, RUIINI);

		itoa(rt.left, intbuf, 10);
		WritePrivateProfileString("TA", "x", intbuf, inifile);

		itoa(rt.top, intbuf, 10);
		WritePrivateProfileString("TA", "y", intbuf, inifile);
	}

	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
}


void CTADlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if( m_pra && m_pra->m_state >= TA_STATE_ERR_DEVNOTFOUND )
	{
		m_pra->set_state(TA_STATE_SERVICE_STOP_DONE);
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}
