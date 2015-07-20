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


package com.skplanet.skrc2.rui;
import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.IOException;
import java.nio.channels.SocketChannel;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JSplitPane;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;

import com.skplanet.skrc2.ftp.FTPDlg;
import com.skplanet.skrc2.main.TCFrame;
import com.skplanet.skrc2.ruicUI.AboutDlg;
import com.skplanet.skrc2.ruicUI.AppInstDlg;
import com.skplanet.skrc2.ruicUI.DevInfoDlg;
import com.skplanet.skrc2.ruicUI.DropDownButton;
import com.skplanet.skrc2.ruicUI.RUICanvas;
import com.skplanet.skrc2.ruicUI.RUILogcat;
import com.skplanet.skrc2.ruicUI.RUIStatus;
import com.skplanet.skrc2.utils.Settings;
import com.skplanet.skrc2.utils.Utils;


public class RUIClient {
	// rui constant
	public static final int TA_VERSION_LEN	= 14;
	public static final String TC_VERSION = "2.0.0.2";
	public static final String TC_PRODUCT_NAME = "SKPlanet Test Client.v2";
	public static final String TC_COPYRIGHT = "Copyright (C) 2013";
	public static final String TC_DEVKEY = "0108995225901052700256";
	public static final int CONNECTION_TIMEOUT = 30;
	public static final int CONN_RES_OK = 0;
	public static final int CONN_RES_INVALID_RESERVATION_ID = 1;
	public static final int CONN_RES_INVALID_TIME = 2;
	public static final int CONN_RES_INVALID_VERSION = 3;
	public static final int CONN_RES_INVALID_UNKNOWN = 4;
	
	public static final int FILE_ERR_CONVERT	=		0x0001;	// error occur during convert path
	public static final int FILE_ERR_OPENFILE	=	0x0002;	// could not open file
	public static final int FILE_ERR_READ			=	0x0003;	// file read error
	public static final int FILE_ERR_STAT			=	0x0004;	// file stat failed perhaps it is absent or is not a regular file
	public static final int FILE_ERR_CREATE		=	0x0005;  // file creation error
	public static final int FILE_ERR_WRITE		=	0x0006;	// file write error
	public static final int FILE_ERR_CANCELBYUSER	=	0x0007; // user cancel transfer
	
	public TAConnectionThread tcCThread;
	
	public SocketChannel tcSock;
	public SocketAbortable abortable;
	
	public String	tcAddress;
	public int		tcPort;
	public int		tcVPort;
	public String	tcReservationID;
	
	public RUIService			tcService;
	public TCFrame			uiWindow;
	public RUICanvas		tcCanvas;
	public RUIStatus 		tcStatus;
	public RUILogcat			tcLogcat;
	public String  tcErr;
	private TAState m_lastState;
	//ProgressWheelPanel ing;
    public enum STATE { DISCONNECTED, CONNECTING, CONNECTED };
	public STATE state;
	
	public AppInstDlg dialogAppInstall;

	// ui component
	//private GridBagLayout gBag = new GridBagLayout();
	private JSplitPane splitPane;
	JMenuBar menubar;
    JMenu filemenu;
    JMenu toolmenu;
    JMenu optionmenu;
    JMenu helpmenu;
    
    Action connect;
    Action sound;
    Action appinst;
    Action devinfo;
    Action logcat;
    private boolean logstarted = false;
    Action exit;
    

    Action home;
    Action back	;
    Action menu;
    Action wakeup;

    Action volUp;
    Action volDown;

    Action capture;
    Action filetransfer;
    Action cleanup;
    
    Action showTool;
    Action showStatus;
    Action showButton;
    
    Action about;
    
    JToolBar toolbar;
    JToolBar toolbardev;
    JPanel toolbardevPanel;
    
    JButton connectButton;
    JToggleButton soundButton;
    DropDownButton rotateButton;
    JPopupMenu rotatemenu;
    Action rotate0;
    Action rotate90;
    Action rotate180;
    Action rotate270;
    Action rotateAuto;
    
    boolean bshowTool;
    boolean bshowStatus;
    boolean bshowButton;
    
    public FTPDlg	ftpdlg; 
	
    
	public RUIClient( TCFrame window, String key, String address, int port) {
		Utils.LOG("new RUIClient key(%s) address(%s) port(%d)\n", key, address, port);
		uiWindow = window;
		tcReservationID = key;
		tcAddress = address;
		tcPort = port;
		m_lastState = new TAState();
		m_lastState.m_devState = -1;
		state = STATE.DISCONNECTED;
		dialogAppInstall = null;
		
		bshowTool = Settings.getShowToolbar();
		bshowStatus = Settings.getShowStatusbar();
		bshowButton = Settings.getShowButtonbar();
		
		Container contentPane = window.getContentPane();

	    // Specify a layout manager for the content pane
	    contentPane.setLayout(new BorderLayout());
	    //contentPane.setLayout(gBag);
	    
	    tcStatus = new RUIStatus(this);
		tcService = new RUIService(this);
	    tcCanvas = new RUICanvas(tcService);
	    tcCanvas.setPreferredSize(new Dimension(500,700));
		JPanel canvasPanel = new JPanel();
		canvasPanel.setLayout(new BorderLayout());
		
		toolbar = new JToolBar();
		toolbar.setLayout(new FlowLayout(FlowLayout.CENTER, 0, 0));
		toolbardev = new JToolBar();
		toolbardev.setLayout(new FlowLayout(FlowLayout.CENTER, 0, 0));
		toolbardevPanel = new JPanel();
		toolbardevPanel.add(toolbardev);
		toolbardevPanel.setSize(new Dimension(500,1));
		canvasPanel.add("Center", tcCanvas);
		canvasPanel.add("South", toolbardevPanel);

	    tcLogcat = new RUILogcat(tcService);
	    
        splitPane = new JSplitPane(JSplitPane.VERTICAL_SPLIT,
        		canvasPanel, tcLogcat);
        splitPane.setPreferredSize(new Dimension(500,760));

	    
	    contentPane.add("Center", splitPane);
	    contentPane.add("South", tcStatus);
	    
		addMenuToolbar();
		
		showLogcat(false);
		toolbar.setVisible(bshowTool);
		tcStatus.setVisible(bshowStatus);
		toolbardevPanel.setVisible(bshowButton);
		
		this.connect();
	}

	
	// menu command
		//- Connect
		@SuppressWarnings("serial")
		class connectAction extends AbstractAction {
			public connectAction() {
				super("Connect"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				if( state == STATE.DISCONNECTED) {
					Utils.LOG("Command : Connect");
					connect();
				}
				else {
					disconnect();
				}

			}
		}
		//- sound
		@SuppressWarnings("serial")
		class soundAction extends AbstractAction {
			public soundAction() {
				super("Sound Off"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				try {
					tcService.setAudioStream(!tcService.getSoundOn());
				} catch (TransportException e) {
					e.printStackTrace();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		// - appinst
		@SuppressWarnings("serial")
		class appinstAction extends AbstractAction {
			public appinstAction() {
				super("AppInst"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				dialogAppInstall = new AppInstDlg( uiWindow.tc);
				dialogAppInstall.doModal();
			}
		}
		// - devinfo
		@SuppressWarnings("serial")
		class devinfoAction extends AbstractAction {
			public devinfoAction() {
				super("DeviceInfomation"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				DevInfoDlg dlg = new DevInfoDlg(uiWindow.tc);
				dlg.doModal();
			}
		}
		//- logcat
		@SuppressWarnings("serial")
		class logcatAction extends AbstractAction {
			public logcatAction() {
				super("logCat"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				if( logstarted) {
					showLogcat(false);
				} else {
					showLogcat(true);
				}
			}
		}		
		//- HOME
		@SuppressWarnings("serial")		
		class homeAction extends AbstractAction {
			public homeAction() {
				super("HOME"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				Utils.LOG("Command : HOME");
				try {
					tcService.keyEvent(RUIKEY.AKEYCODE_HOME, 1);
					tcService.keyEvent(RUIKEY.AKEYCODE_HOME, 0);
					//ShowLogcat(true);
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		//- BACK
		@SuppressWarnings("serial")
		class backAction extends AbstractAction {
			public backAction() {
				super("BACK"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				Utils.LOG("Command : BACK");
				try {
					tcService.keyEvent(RUIKEY.AKEYCODE_BACK, 1);
					tcService.keyEvent(RUIKEY.AKEYCODE_BACK, 0);
					//ShowLogcat(false);
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		//- MENU
		@SuppressWarnings("serial")
		class menuAction extends AbstractAction {
			public menuAction() {
				super("MENU"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				Utils.LOG("Command : MENU");
				try {
					tcService.keyEvent(RUIKEY.AKEYCODE_MENU, 1);
					tcService.keyEvent(RUIKEY.AKEYCODE_MENU, 0);
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		//- WAKEUP
		@SuppressWarnings("serial")
		class wakeupAction extends AbstractAction {
			public wakeupAction() {
				super("WAKEUP"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				Utils.LOG("Command : WAKEUP");
				try {
					tcService.wakeUP();
				} catch (IOException e) {
					e.printStackTrace();
				} catch (TransportException e) {
					e.printStackTrace();
				}
			}
		}
		//- Exit
		@SuppressWarnings("serial")
		class exitAction extends AbstractAction {
			public exitAction() {
				super("Exit"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				Utils.LOG("Command : Exit");
				uiWindow.exit(0);
			}
		}
		//- capture
		@SuppressWarnings("serial")
		class captureAction extends AbstractAction {
			public captureAction() {
				super("Capture"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				tcService.screenCapture();
			}
		}
		@SuppressWarnings("serial")
		class volUpAction extends AbstractAction {
			public volUpAction() {
				super("Volume Up"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				try {
					tcService.keyEvent(RUIKEY.AKEYCODE_VOLUME_UP, 1);
					tcService.keyEvent(RUIKEY.AKEYCODE_VOLUME_UP, 0);
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		//- volDown
		@SuppressWarnings("serial")
		class volDownAction extends AbstractAction {
			public volDownAction() {
				super("Volume Down"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				try {
					tcService.keyEvent(RUIKEY.AKEYCODE_VOLUME_DOWN, 1);
					tcService.keyEvent(RUIKEY.AKEYCODE_VOLUME_DOWN, 0);
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		//- rotate0
		@SuppressWarnings("serial")
		class rotate0Action extends AbstractAction {
			public rotate0Action() {
				super("Rotate 0"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
					tcService.setRotate(0);
			}
		}
		//- rotate90
		@SuppressWarnings("serial")
		class rotate90Action extends AbstractAction {
			public rotate90Action() {
				super("Rotate 90"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
					tcService.setRotate(1);
			}
		}
		//- rotate180
		@SuppressWarnings("serial")
		class rotate180Action extends AbstractAction {
			public rotate180Action() {
				super("Rotate 180"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
					tcService.setRotate(2);
			}
		}
		//- rotate270
		@SuppressWarnings("serial")
		class rotate270Action extends AbstractAction {
			public rotate270Action() {
				super("Rotate 270"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
					tcService.setRotate(3);
			}
		}
		//- rotateAuto
		@SuppressWarnings("serial")
		class rotateAutoAction extends AbstractAction {
			public rotateAutoAction() {
				super("Rotate Auto"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				tcService.setRotate(0);
				tcService.setRotate(4);
			}
		}
		//- filetransfer
		@SuppressWarnings("serial")
		class filetransferAction extends AbstractAction {
			public filetransferAction() {
				super("File Transfer"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				ftpdlg = new FTPDlg(uiWindow.tc);
				ftpdlg.doModal();
			}
		}
		//- cleanup
		@SuppressWarnings("serial")
		class cleanupAction extends AbstractAction {
			public cleanupAction() {
				super("CleanUp"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				int nResult = JOptionPane.showConfirmDialog(null, "Do you want to remove all user installed applications?", RUIClient.TC_PRODUCT_NAME, JOptionPane.OK_CANCEL_OPTION);
				if( nResult == JOptionPane.OK_OPTION) {
					tcService.removeUserPackage();
				}
			}
		}
		//- showTool
		@SuppressWarnings("serial")
		class showToolAction extends AbstractAction {
			public showToolAction() {
				super("Tool Bar"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				bshowTool = !bshowTool;
				toolbar.setVisible(bshowTool);
				this.putValue(AbstractAction.SELECTED_KEY, bshowTool);
				Settings.setShowToolbar(bshowTool);
			}
		}
		//- showStatus
		@SuppressWarnings("serial")
		class showStatusAction extends AbstractAction {
			public showStatusAction() {
				super("Status Bar"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				bshowStatus = !bshowStatus;
				tcStatus.setVisible(bshowStatus);
				this.putValue(AbstractAction.SELECTED_KEY, bshowStatus);
				Settings.setShowStatusbar(bshowStatus);
			}
		}
		//- showButton
		@SuppressWarnings("serial")
		class showButtonAction extends AbstractAction {
			public showButtonAction() {
				super("Button Bar"); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				bshowButton = !bshowButton;
				toolbardevPanel.setVisible(bshowButton);
				this.putValue(AbstractAction.SELECTED_KEY, bshowButton);
				Settings.setShowButtonbar(bshowButton);
			}
		}
		//- about
		@SuppressWarnings("serial")
		class aboutAction extends AbstractAction {
			public aboutAction() {
				super("About TC..."); // Specify the name of the action
			}

			@Override
			public void actionPerformed(ActionEvent arg0) {
				AboutDlg dlg = new AboutDlg(uiWindow.tc);
				dlg.doModal();
			}
		}
		public void addMenuToolbar() {

	    //--------------------------------------
		menubar = new JMenuBar(); // Create a menubar
	    uiWindow.setJMenuBar(menubar); // Display it in the JFrame
	    
	    filemenu = new JMenu("File");
	    connect = new connectAction();
	    sound		= new soundAction();
	    appinst	= new appinstAction();
	    devinfo  = new devinfoAction();
	    logcat	= new logcatAction();
	    exit       = new exitAction();
	    home	 = new homeAction();
	    back	 = new backAction();
	    menu	 = new menuAction();
	    wakeup = new wakeupAction();
	    //search = new searchAction();
	    volUp = new volUpAction();
	    volDown = new volDownAction();
	    capture = new captureAction();
	    rotate0 = new rotate0Action();
	    rotate90 = new rotate90Action();
	    rotate180 = new rotate180Action();
	    rotate270 = new rotate270Action();
	    rotateAuto = new rotateAutoAction();
	    filetransfer = new filetransferAction();
	    cleanup = new cleanupAction();
	    showTool = new showToolAction();
	    showStatus = new showStatusAction();
	    showButton = new showButtonAction();
	    about = new aboutAction();
	    
	    filemenu.add(connect);
	    filemenu.add(exit);
	    
	    rotatemenu = new JPopupMenu();
	    rotatemenu.add(new JCheckBoxMenuItem(rotate0));
	    rotatemenu.add(new JCheckBoxMenuItem(rotate90));
	    rotatemenu.add(new JCheckBoxMenuItem(rotate180));
	    rotatemenu.add(new JCheckBoxMenuItem(rotate270));
	    rotatemenu.addSeparator();
	    rotatemenu.add(new JCheckBoxMenuItem(rotateAuto));

	    toolmenu = new JMenu("Tools");
	    JMenu rmenu = new JMenu("Rotate");
	    rmenu.add(new JCheckBoxMenuItem(rotate0));
	    rmenu.add(new JCheckBoxMenuItem(rotate90));
	    rmenu.add(new JCheckBoxMenuItem(rotate180));
	    rmenu.add(new JCheckBoxMenuItem(rotate270));
	    rmenu.addSeparator();
	    rmenu.add(new JCheckBoxMenuItem(rotateAuto));
	    toolmenu.add(rmenu);
	    
	    toolmenu.add(capture);
	    toolmenu.add(appinst);
	    toolmenu.add(new JCheckBoxMenuItem(this.logcat));
	    toolmenu.add(this.devinfo);
	    toolmenu.add(filetransfer);
	    toolmenu.add(cleanup);
	    
	    optionmenu = new JMenu("Option");
	    optionmenu.add(new JCheckBoxMenuItem(showTool));
	    showTool.putValue(AbstractAction.SELECTED_KEY, bshowTool);
	    
	    optionmenu.add(new JCheckBoxMenuItem(showStatus));
	    showStatus.putValue(AbstractAction.SELECTED_KEY, bshowStatus);
	    
	    optionmenu.add(new JCheckBoxMenuItem(showButton));
	    showButton.putValue(AbstractAction.SELECTED_KEY, bshowButton);
	    
	    helpmenu = new JMenu("Help");
	    helpmenu.add(about);
	    
	    menubar.add(filemenu);
	    menubar.add(toolmenu);
	    menubar.add(optionmenu);
	    menubar.add(helpmenu);
	    
	    //--------------------------------------
	    //toolbar = new JToolBar();
	    JButton btn;
	    connectButton = toolbar.add(connect);
	    connectButton.setIcon(Utils.getButtonIcon("connect_b.png"));
	    connectButton.setRolloverIcon(Utils.getButtonIcon("connect_o.png"));
	    connectButton.setDisabledIcon(Utils.getButtonIcon("connect_l.png"));
	    connectButton.setHideActionText(true);
	    connectButton.setBorder(BorderFactory.createEtchedBorder());
	    connectButton.setBorderPainted(false);

	    btn = toolbar.add(appinst);
	    btn.setIcon(Utils.getButtonIcon("appinst_b.png"));
	    btn.setRolloverIcon(Utils.getButtonIcon("appinst_o.png"));
	    btn.setDisabledIcon(Utils.getButtonIcon("appinst_l.png"));
	    btn.setHideActionText(true);
	    btn.setBorder(BorderFactory.createEtchedBorder());
	    btn.setBorderPainted(false);
	    
	    btn = toolbar.add(cleanup);
	    btn.setIcon(Utils.getButtonIcon("cleanup_b.png"));
	    btn.setRolloverIcon(Utils.getButtonIcon("cleanup_o.png"));
	    btn.setDisabledIcon(Utils.getButtonIcon("cleanup_l.png"));
	    btn.setHideActionText(true);
	    btn.setBorder(BorderFactory.createEtchedBorder());
	    btn.setBorderPainted(false);

	    JToggleButton tbtn = new JToggleButton(logcat);
	    tbtn.setIcon(Utils.getButtonIcon("logcat_off_b.png"));
	    tbtn.setRolloverIcon(Utils.getButtonIcon("logcat_off_o.png"));
	    tbtn.setDisabledIcon(Utils.getButtonIcon("logcat_off_l.png"));
	    tbtn.setSelectedIcon(Utils.getButtonIcon("logcat_b.png"));
	    tbtn.setRolloverSelectedIcon(Utils.getButtonIcon("logcat_o.png"));
	    tbtn.setHideActionText(true);
	    tbtn.setBorder(BorderFactory.createEtchedBorder());
	    tbtn.setBorderPainted(false);
	    toolbar.add(tbtn);

	    btn = toolbar.add(capture);
	    btn.setIcon(Utils.getButtonIcon("capture_b.png"));
	    btn.setRolloverIcon(Utils.getButtonIcon("capture_o.png"));
	    btn.setDisabledIcon(Utils.getButtonIcon("capture_l.png"));
	    btn.setHideActionText(true);
	    btn.setBorder(BorderFactory.createEtchedBorder());
	    btn.setBorderPainted(false);

	    rotateButton = new DropDownButton("");
	    rotateButton.setIcon(Utils.getButtonIcon("rotate_b.png"));
	    rotateButton.setRolloverIcon(Utils.getButtonIcon("rotate_o.png"));
	    rotateButton.setDisabledIcon(Utils.getButtonIcon("rotate_l.png"));
	    rotateButton.setMenu(rotatemenu);
	    rotateButton.setBorder(BorderFactory.createEtchedBorder());
	    rotateButton.setBorderPainted(false);


	    toolbar.add(rotateButton);

	    toolbar.setFloatable(false);
	    toolbar.setBorder(null);

	    uiWindow.getContentPane().add(toolbar, BorderLayout.NORTH);
	    
	    //--------------------------------------
	   
	    btn = toolbardev.add(menu);
	    btn.setIcon(Utils.getButtonIcon("menu_b.png"));
	    btn.setRolloverIcon(Utils.getButtonIcon("menu_o.png"));
	    btn.setDisabledIcon(Utils.getButtonIcon("menu_l.png"));
	    btn.setHideActionText(true);
	    btn.setBorder(BorderFactory.createEtchedBorder());
	    btn.setBorderPainted(false);

	    btn = toolbardev.add(home);
	    btn.setIcon(Utils.getButtonIcon("home_b.png"));
	    btn.setRolloverIcon(Utils.getButtonIcon("home_o.png"));
	    btn.setDisabledIcon(Utils.getButtonIcon("home_l.png"));
	    btn.setHideActionText(true);
	    btn.setBorder(BorderFactory.createEtchedBorder());
	    btn.setBorderPainted(false);

	    btn = toolbardev.add(back);
	    btn.setIcon(Utils.getButtonIcon("back_b.png"));
	    btn.setRolloverIcon(Utils.getButtonIcon("back_o.png"));
	    btn.setDisabledIcon(Utils.getButtonIcon("back_l.png"));
	    btn.setHideActionText(true);
	    btn.setBorder(BorderFactory.createEtchedBorder());
	    btn.setBorderPainted(false);

	    soundButton = new JToggleButton(sound);
	    soundButton.setIcon(Utils.getButtonIcon("sound_off_b.png"));
	    soundButton.setRolloverIcon(Utils.getButtonIcon("sound_off_o.png"));
	    soundButton.setDisabledIcon(Utils.getButtonIcon("sound_off_l.png"));
	    soundButton.setSelectedIcon(Utils.getButtonIcon("sound_b.png"));
	    soundButton.setRolloverSelectedIcon(Utils.getButtonIcon("sound_o.png"));
	    soundButton.setHideActionText(true);
	    soundButton.setBorder(BorderFactory.createEtchedBorder());
	    soundButton.setBorderPainted(false);
	    toolbardev.add(soundButton);

	    btn = toolbardev.add(volUp);
	    btn.setIcon(Utils.getButtonIcon("volup_b.png"));
	    btn.setRolloverIcon(Utils.getButtonIcon("volup_o.png"));
	    btn.setDisabledIcon(Utils.getButtonIcon("volup_l.png"));
	    btn.setHideActionText(true);
	    btn.setBorder(BorderFactory.createEtchedBorder());
	    btn.setBorderPainted(false);

	    btn = toolbardev.add(volDown);
	    btn.setIcon(Utils.getButtonIcon("voldn_b.png"));
	    btn.setRolloverIcon(Utils.getButtonIcon("voldn_o.png"));
	    btn.setDisabledIcon(Utils.getButtonIcon("voldn_l.png"));
	    btn.setHideActionText(true);
	    btn.setBorder(BorderFactory.createEtchedBorder());
	    btn.setBorderPainted(false);


	    toolbardev.setFloatable(false);
	    toolbardev.setBorder(null);
	    //uiWindow.getContentPane().add(toolbardev, BorderLayout.SOUTH);
	    onDisconnected();
	}

	public void showLogcat(boolean on) {
		//Utils.LOG("Component Count:%d", splitPane.getComponentCount());
		if( on ) {
			if( splitPane.getComponentCount() > 2 )
				return;
			logcat.putValue(AbstractAction.SELECTED_KEY, Boolean.TRUE);
			logstarted = true;
			splitPane.add(tcLogcat);
	        splitPane.setResizeWeight(1.0 - Settings.getLogcatWindowSize());
	        splitPane.setDividerSize(5);
	        if( tcService.isAlive()) {
	        	tcService.getLogcat("", 0);
	        }
	        splitPane.addPropertyChangeListener(new PropertyChangeListener() {
	        	public void propertyChange (PropertyChangeEvent changeEvent) {
	        		JSplitPane sourceSplitPane = (JSplitPane)changeEvent.getSource();
	        		String propertyName = changeEvent.getPropertyName();
	        		if (propertyName.equals(
	        				JSplitPane.LAST_DIVIDER_LOCATION_PROPERTY)) {
	        			int current = sourceSplitPane.getDividerLocation();
	        			int height = sourceSplitPane.getHeight();
	        			Settings.setLogcatWindowSize((double)(height - current) / (double)height);
	        		}
	        	}
	        });
	        
		} else {
			if( splitPane.getComponentCount() < 3 )
				return;
			splitPane.getResizeWeight();
			logcat.putValue(AbstractAction.SELECTED_KEY, Boolean.FALSE);
			logstarted = false;
	        splitPane.remove(tcLogcat);
	        splitPane.setDividerSize(0);
	        if( tcService.isAlive()) {
		        tcService.getLogcat("", 6);
		        tcLogcat.resetContents();
	        }
		}
	}
	
	public void waitCursor(boolean on) {
		if( on ) {
			uiWindow.setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
		}
		else {
			uiWindow.setCursor(Cursor.getDefaultCursor());
		}
	}
	
	// on UI event
	public void onConnectionResult(boolean btimeout, boolean aborted, boolean connected) {
		if(  !aborted && connected == true )
		{
			Utils.LOG("connect validation passed");
		} else {
			if( btimeout ) {
				JOptionPane.showMessageDialog(null, "Connection timeout!", RUIClient.TC_PRODUCT_NAME, JOptionPane.ERROR_MESSAGE);
			}
			else if ( aborted ) {
				JOptionPane.showMessageDialog(null, "Stop Connection!", RUIClient.TC_PRODUCT_NAME, JOptionPane.WARNING_MESSAGE);
			}
			else {
				if( tcErr != null && tcErr.isEmpty() == false) {
					JOptionPane.showMessageDialog(null, tcErr, "Error", JOptionPane.ERROR_MESSAGE);
					tcErr = "";
				}
			}
			onDisconnected();
		}		
	}
	
	public void onConnResFromServer(int conn_result) {
		String message = "";
		int type = JOptionPane.ERROR_MESSAGE;
		
	 	   if( conn_result == RUIClient.CONN_RES_INVALID_RESERVATION_ID) {
	 		   message = "Connection Failed.\nCheck your reservation id.";
	 	   } else if( conn_result == RUIClient.CONN_RES_INVALID_TIME) {
	 		   message = "Connection Failed.\nCheck your reservation time.";
	 	   } else if( conn_result == RUIClient.CONN_RES_INVALID_VERSION) {
	 		   message = "Connection Failed.\nThis version is not supported.\nUpgrade Required.\n(Check home page.)";
	 	   } else if( conn_result == RUIClient.CONN_RES_INVALID_UNKNOWN) {
	 		   message =  "Connection Failed.";
		   }

	 	   if( message.length() > 0) {
	 		   tcStatus.setText(message);
	 		   JOptionPane.showMessageDialog(null, message, RUIClient.TC_PRODUCT_NAME, type);
	 	   }
	}
	public void onDisconnectResult(boolean btimeout, boolean aborted, boolean interrupted) {
		String message = "";
		int type = JOptionPane.ERROR_MESSAGE;
		if(  interrupted )
		{
			if( aborted ) {
				message = "Stop Connection!";
				type = JOptionPane.WARNING_MESSAGE;
			} else {
				message = "Disconnected!";
				type = JOptionPane.ERROR_MESSAGE;
			}
		} else {
			if( aborted ) {
				//message = "Disconnected!";
				//type = JOptionPane.PLAIN_MESSAGE;
			}
			else {
				message = "Abnormal Disconnection";
				type = JOptionPane.ERROR_MESSAGE;
			}
		}
		onDisconnected();
 	   if( message.length() > 0) {
 		   tcStatus.setText(message);
 		   JOptionPane.showMessageDialog(null, message, RUIClient.TC_PRODUCT_NAME, type);
 	   }		
	}
	
	public void onDeviceInfoUpdate() {
		uiWindow.setTitle(RUIClient.TC_PRODUCT_NAME + " - (" + tcService.devBuildModel + ")");
	}
	
	public void onConnecting() {
		tcCanvas.repaint();
		waitCursor(true);
		state = STATE.CONNECTING;
		connectButton.setIcon(Utils.getButtonIcon("disconnect_b.png"));
		connectButton.setRolloverIcon(Utils.getButtonIcon("disconnect_o.png"));
		connectButton.setDisabledIcon(Utils.getButtonIcon("disconnect_l.png"));
		connect.putValue(AbstractAction.NAME, "Disconnect");
		tcStatus.setText("Connecting...");
		uiWindow.onConnecting();
	}
	public void onConnected() {
		waitCursor(false);
		state = STATE.CONNECTED;
		connectButton.setIcon(Utils.getButtonIcon("disconnect_b.png"));
		connectButton.setRolloverIcon(Utils.getButtonIcon("disconnect_o.png"));
		connectButton.setDisabledIcon(Utils.getButtonIcon("disconnect_l.png"));
		connect.putValue(AbstractAction.NAME, "Disconnect");
		toolmenu.setEnabled(true);
	    optionmenu.setEnabled(true);

		
		sound.setEnabled(true);
		logcat.setEnabled(true);
		home.setEnabled(true);
		back.setEnabled(true);
		menu.setEnabled(true);
		wakeup.setEnabled(true);
		appinst.setEnabled(true);
		cleanup.setEnabled(true);
		capture.setEnabled(true);
		//search.setEnabled(true);
		volUp.setEnabled(true);
		volDown.setEnabled(true);
		rotateButton.setEnabled(true);
		
		tcStatus.setServerValidTime(tcService.getServerValidTime());
		tcStatus.setConnect(true);
		tcStatus.setText("Connected.");
		
		uiWindow.onConnected();
	}
	
	public void onDisconnected() {
		waitCursor(false);
		showLogcat(false);
		tcCanvas.loadImage((byte[])null);
		tcCanvas.repaint();
		state = STATE.DISCONNECTED;
		if( dialogAppInstall != null) {
			if( dialogAppInstall.isVisible()) {
				dialogAppInstall.dispose();
			}
		}
		connectButton.setIcon(Utils.getButtonIcon("connect_b.png"));
		connectButton.setRolloverIcon(Utils.getButtonIcon("connect_o.png"));
		connectButton.setDisabledIcon(Utils.getButtonIcon("connect_l.png"));
		connect.putValue(AbstractAction.NAME, "Connect");
		toolmenu.setEnabled(false);
	    optionmenu.setEnabled(false);
		sound.putValue(AbstractAction.SELECTED_KEY, false);
		sound.putValue(AbstractAction.NAME, "Sound Off");
		//soundButton.setIcon(Utils.getButtonIcon("soundoff.png"));
		sound.setEnabled(false);
		logcat.setEnabled(false);
		home.setEnabled(false);
		back.setEnabled(false);
		menu.setEnabled(false);
		wakeup.setEnabled(false);
		appinst.setEnabled(false);
		cleanup.setEnabled(false);
		capture.setEnabled(false);
		//search.setEnabled(false);
		volUp.setEnabled(false);
		volDown.setEnabled(false);
		rotateButton.setEnabled(false);

		tcStatus.setConnect(false);
		tcStatus.setText("Ready.");
		uiWindow.onDisconnected();
	}
	
	// -- action
	public void connect() {
		
		if( tcAddress.isEmpty() || tcPort == 0)
			return;
		
		tcVPort = tcPort+1;

		Utils.LOG("Connect key(%s) address(%s) port(%d)\n", tcReservationID, tcAddress, tcPort);

		onConnecting();
		abortable = new SocketAbortable(); 
		tcService = new RUIService(this);
		tcLogcat.setTCService(tcService);
		tcCanvas.setTCService(tcService);
		tcCThread = new TAConnectionThread(this);
		tcCThread.start();
		
	}
	
	public void disconnect() {
		if( tcCThread.isAlive() || tcService.isAlive()) {
			tcService.notifyDisconnectToServer();
			RUIPoc.reportClientStatusToPOC(tcReservationID, 9, 2, "User Cancelled");
		}
		abortable.done = true;
		while ( tcCThread.isAlive() || tcService.isAlive()) {
			try {
				Thread.sleep(100);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
	
	// command handler
	public boolean onScreenBuf(byte[] buffer) {
		tcCanvas.loadImage(buffer);
		tcCanvas.repaint();		
		tcStatus.onScreenBuf();
		return true;
	}
	
	public void rotateFrame(int nRotate) {
			Rectangle r = uiWindow.getBounds();
			Rectangle rpanel = tcCanvas.getBounds();
			
			boolean blandscape = false;
			if( rpanel.width > rpanel.height)
				blandscape = true;
			
			r.width += (rpanel.height - rpanel.width);
			r.height += (rpanel.width - rpanel.height);

			if( nRotate == 1 || nRotate == 3 ) { // landscape
				if( blandscape == false )
					uiWindow.setBounds(r);
			}
			else {
				if( blandscape == true )
					uiWindow.setBounds(r);
			}
			
	}
	
	public void onTAStateChanged(TAState state) {
		if( state.getScreenRotate() != m_lastState.getScreenRotate())
		{
			int r = state.getScreenRotate();
			rotateFrame(r);
			rotate0.putValue(AbstractAction.SELECTED_KEY, (r==0) ? Boolean.TRUE : Boolean.FALSE);
			rotate90.putValue(AbstractAction.SELECTED_KEY, (r==1) ? Boolean.TRUE : Boolean.FALSE);
			rotate180.putValue(AbstractAction.SELECTED_KEY, (r==2) ? Boolean.TRUE : Boolean.FALSE);
			rotate270.putValue(AbstractAction.SELECTED_KEY, (r==3) ? Boolean.TRUE : Boolean.FALSE);
			rotateAuto.putValue(AbstractAction.SELECTED_KEY, (state.isScreenRotateAuto()) ? Boolean.TRUE : Boolean.FALSE);
		}
		m_lastState.setState(state.getState());
	}
}
