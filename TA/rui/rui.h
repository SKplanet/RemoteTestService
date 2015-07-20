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
#include "ruicmd.h"
#include "ruitdccmd.h"

#include "../skvideo/RUILib/RUIBuffer.h"
#include "../skvideo/RUILib/RUIBufferList.h"

#include "../skvideo/RUILib/RUIAudio.h"
//#include "../skvideo/RUILib/RUIScreenCaptureThread.h"
#include "../skvideo/RUILib/RUIBufferVideoEncoderThread.h"

#include "../skvideo/RUILib/RUIUDPSockServer.h"
#include "../skvideo/RUILib/RUITCPSockServer.h"
#include "../skvideo/h264/common/extension/ipp_extension.h"
#include "../skvideo/RUILib/RUIBufferAudioEncoderThread.h"

#include "WaveInDeviceList.h"

#pragma pack(push,4)
// MACRO
#define TA_VERSION_LEN	14		// "TEST AGENT 001"
#define MAXCLIENTWAIT	10000
#define MAX_TD			10
#define LOGCAT_BUFSIZE	40960
// port
#define PORT_FBC			5910
#define PORT_EVENTINJECT	5911
#define PORT_TDC			5912
//#define PORT_LISTEN			5913
#define PORT_EXT_LISTEN		6000	// from 6000 + 6100  TCP + UDP

#define POC_ENABLE				(1)
#define USE_HTTPS				(0)

#ifdef REDECON
	#define POC_ADDRESS				"1bang.co.kr"
	#define POC_DEV_STATE_PAGE		"/update.php"
#else
	//#define POC_ADDRESS				"oic.tstore.co.kr" //"oic.skplanet.com"
	#define POC_ADDRESS				"qa.skplanet.co.kr"	// season2
	#define POC_DEV_STATE_PAGE		"/testlab/front/remoteTest/device/createDeviceLog.action"
#endif

#define POC_REGIST_PAGE			"/testlab/front/remoteTest/device/createDevice.action"
#define POC_CLIENT_STATE_PAGE	"/testlab/front/remoteTest/rsrv/createDeviceUseLog.action"
#define POC_RESERVEID_PAGE		"/testlab/front/remoteTest/rsrv/getRtRsrvInfo.action"

#define	TA_STATE_INIT							0x00000000
#define	TA_STATE_INIT_PHASE1					0x00000001
#define TA_STATE_INIT_PHASE2					0x00000002
#define TA_STATE_INIT_PHASE3					0x00000004
#define TA_STATE_INIT_DONE						0x00000008

#define TA_STATE_START_SERVICE					0x00000010
#define TA_STATE_START_SERVICE_PHASE1			0x00000011
#define TA_STATE_START_SERVICE_PHASE2			0x00000012
#define TA_STATE_START_SERVICE_PHASE3			0x00000014

#define TA_STATE_READY							0x00000100

#define TA_STATE_CLIENT_INCOMING				0x00001000
#define TA_STATE_CLIENT_NEGOTIATE				0x00001001
#define TA_STATE_CLIENT_CONNECTED				0x00001002
#define TA_STATE_CLIENT_DISCONNECT				0x00001100
#define TA_STATE_CLIENT_USER_DISCONNECT			0x00001101
#define TA_STATE_CLIENT_TIMEOUT					0x00001102
#define TA_STATE_CLIENT_DISCONNECT_ERR			0x00001104

#define TA_STATE_AFTER_CLIENT_REINIT			0x00010000

#define TA_STATE_SERVICE_STOP					0x00100000
#define TA_STATE_SERVICE_STOP_PHASE1			0x00100001
#define TA_STATE_SERVICE_STOP_PHASE2			0x00100002
#define TA_STATE_SERVICE_STOP_DONE				0x00100004

#define TA_STATE_EXIT							0x01000000

#define TA_STATE_WARN_LOWBATTERY				0x10000000
#define TA_STATE_WARN_HIGHTEMPERATURE			0x10000001
#define TA_STATE_WARN_CALLBLOCKED				0x10000002
#define TA_STATE_WARN_PKGBLOCKED				0x10000004

#define TA_STATE_ERR_DEVNOTFOUND				0x10001000
#define TA_STATE_ERR_NEGOTIATE_FAILED			0x10001001
#define TA_STATE_ERR_INIT_RA					0x10001002
#define TA_STATE_ERR_START_SERVICE				0x10001002


#define FBCCMD_HEADER	8
typedef	struct tagFBCCMD{
	unsigned char  cmd;
	unsigned char  dummy1;
	unsigned short err;
	unsigned int	size;
	char* 		   data;
} FBCCMD;

#define _FBCCMD \
	unsigned char cmd; \
	unsigned char dummy1; \
	unsigned short dummy2; \
	unsigned int size;

typedef struct {
	_FBCCMD
	unsigned int	width;
	unsigned int	height;
	unsigned int	bpp;
	unsigned int	touchfd;
} FBCCMD_SCRINFO;

typedef struct {
	_FBCCMD
	unsigned char	flags;
	unsigned char	dummy3;
	unsigned short	pathsize;
	// followed by path
} FBCCMD_FILELISTREQ;

typedef struct {
	_FBCCMD
	unsigned int	mode;
	unsigned short	x;
	unsigned short	y;
} FBCCMD_TOUCH;

typedef struct {
	_FBCCMD
	unsigned int	mode;
	unsigned int	code;
} FBCCMD_KEY;

// module
#define RUIADB	"ruicmd.exe"
#define RUIINI	"ta.ini"

class RA;
class ra_adb;

typedef void (*fnUpdateComplete)(RA*, void*, int, int);
typedef void (*fnIncomingLog)(RA*, void* pData, BYTE* buf, int len);
typedef void (*fnTAStateReport)(RA*, void* pData, int);

int EnumWaveInDeviceCallback(UINT nDeviceID, WAVEINCAPS* pwic, void* pParam);

extern CRITICAL_SECTION CriticalSection;
// structure

class RUIServerPeer;

class RA {

public:
	RA(CWnd* pWnd);
	~RA();

public:
	//
	CWnd*   m_UIWnd;
	UINT	m_state;
	DWORD   m_state_tick;

	char m_taVersion[TA_VERSION_LEN+1];
	char m_ModulePath[MAX_PATH];

	char m_taSerial[256];
	int  m_bAutoStart;
	int  m_bForceMonkey;
	int  m_bAutoReInit;
	int  m_bCheckEventReturn;
	int  m_sdkVersion;

	int  m_bFirstRun;

	// device info;
	char m_devSerial[MAX_TD][256];
	char m_devIndex;
	char m_audioIndex;
	char m_audioName[128];
	DWORD m_devExternalIP;
	int   m_devExternalPort;

	char m_devModel[128];
	char m_devManufacture[128];
	char m_devBuild[128];
	char m_devSDKVer[128];
	char m_devBuildSerial[128];
	char m_devIMEI[128];
	char m_devSIMSerial[128];
	char m_devUSIM[128];
	char m_devPhoneNO[128];
	USHORT m_devBattLvl;
	USHORT m_devTemperature;

	char m_prohibitPackage[10][128];
	char m_prohibitPhoneNum[10][128];

	SOCKET m_listenSock;
	int m_listenPort;
	IN_ADDR m_listenInterface;
	SOCKET m_clientSock;
	SOCKET m_clientTempSock;

	SOCKET m_fbcsock;
	SOCKET m_monkeysock;
	SOCKET m_tdcsock;

	HANDLE m_tickHandle;
	ra_adb* m_monkeyProcess;
	HANDLE m_fbcProcess;
	//USHORT m_devICS;
	USHORT m_devOrientation;
	int m_devStateChanged;
	//BYTE m_rBuf[4];
	//BYTE m_sBuf[4];	
	BYTE m_tdcBuff[4096];

	// gdiplus
	HGLOBAL m_fbcHandle;
	int		m_fbcSize;
	int		m_fbcFullSize;
	UINT	m_fbcFrameCount;
	UINT	m_fbcLastFrameCount;
	IStream* m_pIStream;
	char m_screenshotname[MAX_PATH];

	// screen
	USHORT	m_fbcWidth;
	USHORT m_fbcHeight;
	USHORT m_fbcBPP;
	USHORT m_fbcTouch;
	BYTE m_FBCBuf[4096];

	// thread
	HANDLE m_hlistenThread;		// client listen thread
	HANDLE m_hclientThread;
	DWORD m_dwlistenThreadID;

	HANDLE m_hfbcThread;		// fbc thread
	DWORD  m_dwfbcThreadID;

	HANDLE m_hlogThread;		// logcat thread
	DWORD  m_dwlogThreadID;

	HANDLE m_htdcThread;
	DWORD  m_dwtdcThreadID;

	HANDLE m_fbUpdateEvent;
	HANDLE m_fbBufferMutex;
	HANDLE m_fbOutputMutex;
	HANDLE m_clientOutputMutex;
	HANDLE m_fbStartListen;

	// onupdate callback
	//fnUpdateComplete m_UpdateCompleteCallback;
	//void* m_UpdateCompleteData;
	fnUpdateComplete m_AdminUpdateCompleteCallback;
	void* m_AdminUpdateCompleteData;
	fnTAStateReport m_TAStateReportCallback;
	void* m_TAStateReportData;

	// onlog callback
	fnIncomingLog	m_IncomingLog;
	void*			m_IncomingLogData;
	fnIncomingLog	m_IncomingTALog;
	void*			m_IncomingTALogData;
	//log pipe
	HANDLE m_hChildStd_OUT_Rd;
	HANDLE m_hChildStd_OUT_Wr;
	HANDLE m_logProcessID;
	int	   m_logInit;

	//adb pipe
	//HANDLE m_hADBStd_OUT_Rd;
	//HANDLE m_hADBStd_OUT_Wr;
	//HANDLE m_hADBStd_IN_Rd;
	//HANDLE m_hADBStd_IN_Wr;
	//HANDLE m_adbProcessID;
	//DWORD  m_dwadbProcessID;
	char   m_chBuf[LOGCAT_BUFSIZE]; // buffer for logcat

	char m_taLog[MAX_PATH];
	char m_temppath[MAX_PATH];

	char m_bShowProgress;

	// statistics
	DWORD	m_tickRecvSum;	// tick for framebuffer get (server capability)
	DWORD	m_frameGetSum;	// count frameget
	DWORD	m_frameLastSend;
	DWORD	m_byteLastSend;
	DWORD	m_byteRecvSum;	// BPS for framebuffer get
	DWORD	m_tickSendSum;	// tick for framebuffer update request interval (maybe client fps)
	double	m_serverFps;
	double	m_serverBps;
	double  m_clientFps;
	double  m_clientBps;

	char m_sendBuf[512];
	char m_recvBuf[512];
	int  m_sendBufIdx;

	// dev status
	USHORT m_devStatus;

	// client info
	char m_clientIP[16];
	char m_clientTempIP[16];
	char m_clientReserveID[256];
	char m_clientVersion[256];
	DWORD	m_clientValidStart;
	DWORD	m_clientValidEnd;
	FILE* m_AppInstFD;
	char m_AppInst[MAX_PATH];
	int m_opt_screen;	// 0 - Image, 1 - H.264
	int m_opt_h264_quality; // 1-low, 0-normal, 2-high
	int m_opt_audio;	// 0 - off, 1- on
	int m_ftp;
	char m_bFullSizeRequest;
	char m_bInitRequired;
	DWORD m_bInitRequiredTime;

	// skvideo
	// Buffer
	BOOL							m_bUseUDP;
	IppExtensionParam				m_ieParam;
	RUIBuffer						m_bufRGB;
	//BOOL							m_bCaptureFlag;
	RUIBufferList					m_buf264;
	RUIUDPSockServer				m_UDPSockServer;
	BOOL							m_bServerStart;
	RUITCPSockServer				m_TCPSockServer;
	//RUIScreenCaptureThread			m_threadCapture;
	RUIBufferVideoEncoderThread		m_threadEncoder;
	RUIAudio						m_threadAudio;
#ifdef _USE_AAC_CODEC
	RUIBufferList					m_bufPCM;
	RUIBufferAudioEncoderThread		m_threadAAC;
#endif
	CWaveInDeviceList				m_waveInDeviceList;
	UINT							m_nBitRate;
	BOOL							m_bStreamingVideo;
	BOOL							m_bStreamingAudio;


public:
	//skvideo
	void							Video_StartServer();
	void							Video_StopServer(BOOL bLog = TRUE);
	BOOL							Video_CreateUDPSock(RUIServerPeer* pPeer, LPCTSTR szIP);
	BOOL							Video_DestroyUDPSock(RUIServerPeer* pPeer);
	void							Video_StartEncoder();
	void							Video_StopEncoder();

	// Thread
	void							Video_InitThreadValue();
	void							Video_StartCaptureThread();
	void							Video_StopCaptureThread();
	void							Video_StartEncoderThread();
	void							Video_StopEncoderThread();
	void							Video_StartStreamingThread();
	void							Video_StopStreamingThread();
	void onVideoRestartEncoder(RUICMD* pcmd);
	void onVideoStartStreaming(RUICMD* pcmd);
	void onVideoSetStreamingMode(RUICMD* pcmd);
	void CheckWaveInDevice(UINT nDeviceID, WAVEINCAPS* pwic);



public:
	int init_ra();
	void exit_ra();
	int ScanDevices();
	void QueueTimerHandler() ;
	void load_ini();
	void save_ini();
	void get_log_filename(char* logfilename);
	void check_log_filename();
	void check_log_size();

	// main api : rui_main.cpp
	void set_state(UINT st);
	int start_ta_service(bool bReinit = 0);
	int stop_ta_service();
		int init_fbc();
		int init_evt();
		int init_tdc();
		int connect_fbc();
		int connect_evt();
		int connect_tdc();
		int restart_fbc();
		int restart_evt();
		int restart_tdc();
	//int reconnect_fbc_svc();
	//int Read_from_fbc(BYTE* buff);
	void getModulePath(char* buffer, int size);
	BOOL DirectoryExists(LPCTSTR szPath);
	
	// socket api : rui_socket.cpp
	int ReadExactTimeout(SOCKET sock, char* buf, int len, int timeout);
	int ReadExact(SOCKET sock,char* buf,int len);
	int WriteExact(SOCKET sock, const char *buf, int len);
	int StringToAddr(char *str, IN_ADDR *addr);
	int ListenOnTCPPort(int port, IN_ADDR iface);
	int ConnectToTcpAddr(char *host, int port);
	int InitListener();
	void ShutdownSocket(SOCKET sock);
	void add_header_sendbuffer(UINT command, UINT idx, UINT size);
	int add_sendbuffer(BYTE* buf, int size);
	int	flush_sendbuffer(SOCKET sock);
	int	write_sendbuffer(SOCKET sock, unsigned short command, unsigned short idx, BYTE* buffer, int size);
	void reset_sendbuffer();

	// event injection : rui_event.cpp
	HANDLE adb_command(char* cmd, int show=0);
	void key_checkok();
	void mouse_event(int x, int y, int mode);
	void key_event(int key, int mode); // 0-down, 1-up, 2-downup
	void key_home(bool bDown);
	void key_back(bool bDown);
	void key_menu(bool bDown);
	void key_search(bool bDown);
	void key_volup(bool bDown);
	void key_voldown(bool bDown);
	void key_wake();
	void key_power(bool bDown);
	void key_camera(bool bDown);

	// framebuffer get : rui_frame.cpp
	int ProcessFBCMessage();
	int init_screen();
	void onFBC_screen(FBCCMD* pcmd);
	int raw_touch(int mode, int x, int y);
	int raw_key(int down, unsigned int code);
	int update_req();
	int onFBC_screendata(FBCCMD* pcmd);
	int JpegToRaw(BYTE *input, int insize, BYTE * output);
	int onFBC_filelistdata(FBCCMD* pcmd);
	int onFBC_filedownloaddata(FBCCMD* pcmd);
	int onFBC_fileerr(FBCCMD* pcmd);
	//void SetUpdateCompleteCallback(fnUpdateComplete pfn, void* pData);
	void SetAdminUpdateCompleteCallback(fnUpdateComplete pfn, void* pData);
	void SetTAStateReportCallback(fnTAStateReport pfn, void* pData);
	void update_complete(RA* pra, int size, int bFullsize);
	void TAStateReport(RA* pra, int bdev = 1);
	int  screenshot(char* fname);
	//int update_fbc(bool fat = true);
	HBITMAP CreateHBITMAP(int width, int height, void** ppvBits);

	#define Swap16(s) \
    ((unsigned short) ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff)))
	#define Swap32(l) \
    ((unsigned int) ((((l) & 0xff000000) >> 24) | \
     (((l) & 0x00ff0000) >> 8)  | \
	 (((l) & 0x0000ff00) << 8)  | \
	 (((l) & 0x000000ff) << 24)))

	/*
	inline unsigned int fromU32() { int b0 = m_rBuf[0]; int b1 = m_rBuf[1];
										int b2 = m_rBuf[2]; int b3 = m_rBuf[3];
							   return (unsigned int)b0 << 24 | b1 << 16 | b2 << 8 | b3; }
	inline int fromS32() { return (int)fromU32(); }

	inline unsigned short fromU16() { int b0 = m_rBuf[0]; int b1 = m_rBuf[1];
							   return (unsigned short)b0 << 8 | b1; }
	inline short fromS16() { return (short)fromS16(); }

	inline void toU32(unsigned int s) { m_sBuf[0] = s >> 24;m_sBuf[1] = s >> 16;
										m_sBuf[2] = s >> 8; m_sBuf[3] = (unsigned char)s; }
	inline void toS32(int s) { toU32((unsigned int)s); }

	inline void toU16(unsigned short s)  { m_sBuf[0] = s >> 8;m_sBuf[1] = (unsigned char)s; }
	inline void toS16(short s) { toU16((unsigned short)s); }

	//------------------
	inline unsigned int fromU32(unsigned char *buf) { int b0 = buf[0]; int b1 = buf[1];
										int b2 = buf[2]; int b3 = buf[3];
							   return (unsigned int)b0 << 24 | b1 << 16 | b2 << 8 | b3; }
	inline int fromS32(char *buf) { (int)fromU32((unsigned char*)buf); }

	inline unsigned short fromU16(unsigned char *buf) { int b0 = buf[0]; int b1 = buf[1];
							   return (unsigned short)(b0 << 8 | b1); }
	inline short fromS16(char *buf) { return (short)fromU16((unsigned char*)buf); }

	inline void toU32(unsigned int s, unsigned char* buf) { buf[0] = s >> 24;buf[1] = s >> 16;
										buf[2] = s >> 8; buf[3] = (unsigned char)s; }
	inline void toS32(int s, char* buf) { toU32((unsigned int)s,(unsigned char*)buf); }

	inline void toU16(unsigned short s, unsigned char* buf)  { buf[0] = s >> 8;buf[1] = (unsigned char)s; }
	inline void toS16(short s, char* buf) { toU16((unsigned short)s, (unsigned char*)buf); }
	*/
	// thread : rui_thread.cpp
	static DWORD WINAPI ruitdcThread(PVOID pvParam);
	static DWORD WINAPI ruilistenThread(PVOID pvParam);
	static DWORD WINAPI ruifbcThread(PVOID pvParam);
	static DWORD WINAPI ruiclientThread(PVOID pvParam);
	static DWORD WINAPI ruilogcatThread(PVOID pvParam);
	void RA::ShowProgress(bool bShow, const char* title = 0, int second = 0);


	// logcat : rui_log.cpp
	int start_log(char* filter);
	void stop_log();
	void SetIncomingLogCallback(fnIncomingLog pfn, void* pData);
	void incoming_log(RA* pra, BYTE* buffer, int size);
	HANDLE CreateLogcatProcess(char* filter);
	int ReadFromPipe(RA* pra) ;
	void ta_log(const char* log, ...);
	void SetIncomingTALogCallback(fnIncomingLog pfn, void* pData);
	void incoming_TAlog(RA* pra, BYTE* buffer, int size);

	// rui adb
	int init_adb();
	void stop_adb();
	int ReadFromADB(char* buffer, int size); // return 0:pending, -1:error, N:readbyte
	int WriteToADB(char* buffer, int size);
	int cmd_adb(char* command, int waitsecond=3, char* output = NULL, int outsize=1024);
	void purge_adb();
	int getdevices_adb(char* buf);
	int getSDKVersion();
	
	//rui client
	int ProcessClientMessage();
	int onNewClientNegotiate(SOCKET client_fd);
	void onNewClient();
	void onExitClient();
	void onClientDisconnectNotify(RUICMD* cmd);
	int onGetScreenInfo(RUICMD* cmd);
	int onGetScreenBuf(RUICMD* cmd);
	//static void OnUpdateComplete(RA* pra, void* pData, int size, int bFullsize);
	void OnUpdateComplete();
	int OnStateChanged();
	int onMouseEvent(RUICMD* cmd);
	int onKeyEvent(RUICMD* cmd);
	int onLogcat(RUICMD* cmd);
	static void logreceiver(RA* pra, void* pData, BYTE* buf, int len);
	int svr_disconnectnotify(UINT result);
	int res_AppInst(UINT result, char* str = 0);
	int onCliGetDevInfo(RUICMD* pcmd);
	int ReInit();

	//rui tdc
	int ProcessTDCMessage();
	int tdc_send_command(USHORT command, USHORT idx=0, USHORT rtn=0, char * buf=0, int bufsize=0);
	int tdc_setRotate(int rotate);	// 0 - 0, 1-90, 2-180, 3- 270, 4-unset
	int tdc_removemessage();
	int tdc_getinstalledpkgname();
	void tdc_on_getinstalledpkgname(TDCCMD* pcmd);
	void tdc_on_getStatus(TDCCMD* pcmd);
	void tdc_getDeviceInfo();
	int  tdc_set_prohibit_list();
	void tdc_on_getDeviceInfo(TDCCMD* pcmd);
	void tdc_setLog(int bEnable);
	void tdc_on_getBatTemperature(TDCCMD* pcmd);
	void tdc_on_getBatLevel(TDCCMD* pcmd);
	void tdc_wake_lock_aquire();
	void tdc_wake_lock_release();

	void onAppInstReq(RUICMD *pcmd);
	void onAppInstData(RUICMD *pcmd);

	// rui file
	void onReqFileList(RUICMD *pcmd);
	void onReqFileDownload(RUICMD *pcmd);
	void onCliFileUpload(RUICMD *pcmd);
	void onCliFileUploadData(RUICMD *pcmd);
	void onCliFileErrorMsg(RUICMD *pcmd);
	int file_res_filelist(char* buf, int size);
	int file_res_filedownload(char* buf, int size);
	int file_svr_err_msg(USHORT err);

	// rui poc
	int RegisterToPOC();
	void ScheduledDeviceStatusToPOC();
	int DeviceStatusToPOC(int state, int state2, const char* description);
	int ClientStatusToPOC(const char* reserve_id, int state, int state2, const char* description);
	int GetReservInfoFromPOC(const char* reserve_id);
	int Decrypt(char* enc, char*out);

};

// ra_adb class
class ra_adb
{
public:
	ra_adb(RA* pra);
	~ra_adb();

private:
	RA*    m_pra;
	HANDLE m_hADBStd_OUT_Rd;
	HANDLE m_hADBStd_OUT_Wr;
	HANDLE m_hADBStd_IN_Rd;
	HANDLE m_hADBStd_IN_Wr;
	HANDLE m_adbProcessID;
	DWORD  m_dwadbProcessID;

	int init_adb();
	void stop_adb();
	int ReadFromADB(char* buffer, int size);
	int WriteToADB(char* buffer, int size);
	void purge_adb();
public:
	int cmd_adb(char* command, int waitsecond, char* output, int outsize, int bBackground=0);


};

// utility
DWORD timedateToDword(int yr = 0, int month = 0, int day = 0, int hour = 0, int minute = 0, int sec = 0);
DWORD timedateDwordPlus(DWORD dwTime, int day, int hour, int minute, int sec);
DWORD timedateDwordMinus(DWORD dValue, DWORD mValue);
CString DwordToTimedate(DWORD dValue);
void DummyMessageLoop(HWND hwnd);

#pragma pack(pop)