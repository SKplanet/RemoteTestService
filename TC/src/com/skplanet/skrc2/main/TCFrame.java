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


package com.skplanet.skrc2.main;

import java.awt.Image;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.text.SimpleDateFormat;
import java.util.List;
import java.util.Locale;

import javax.swing.JFrame;
import javax.swing.JOptionPane;

import com.skplanet.skrc2.rui.RUIClient;
import com.skplanet.skrc2.rui.RUIPoc;
import com.skplanet.skrc2.utils.Settings;
import com.skplanet.skrc2.utils.Utils;

@SuppressWarnings("serial")
public class TCFrame extends JFrame {

	public RUIClient tc;
	
	
	public TCFrame(String key, String address, int port) {
		super(RUIClient.TC_PRODUCT_NAME);
		
		// Handle window close requests
		this.addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
				if( tc != null) {
					tc.disconnect();
				}
				Settings.setWindowSize(e.getWindow().getWidth(), e.getWindow().getHeight());
				Settings.save();
				exit(0);
			}
		});
		//this.setSize(500, 800);
		this.setPreferredSize(Settings.getWindowSize());
		this.setLocationByPlatform(true);
		
		
		List<Image> icons = Utils.getApplicationIcons();
		if (icons.size() != 0) {
			this.setIconImages(icons);
		}
		
				
		tc = new RUIClient(this, key, address, port);

		this.setVisible(true);
		this.pack();
		repaint();
	}

	
	public void onConnecting() {
	}
	public void onConnected() {
	}
	public void onDisconnected() {

	}
	
	
	public static void main(String[] args) {
		System.out.println(RUIClient.TC_PRODUCT_NAME + "(" + RUIClient.TC_VERSION + ")");
		Utils.LOG("Starting ....("+args.length+")");
		if( args.length > 0) {
			for(int i =0; i < args.length; i++) {
				Utils.LOG("["+i+"]:"+args[i]);
			}
			
		}
		
		
		if( args.length == 3) {
			String validkey = args[0];
			
			if( validkey.length() > 8 && validkey.substring(8).equalsIgnoreCase(RUIClient.TC_DEVKEY)) {
				String address = args[1];
				int port = Integer.parseInt(args[2]);
				//TCFrame tc = 
				new TCFrame(validkey, address, port);
				return;
			}
		}
		else if( args.length == 1 ) {
			String reserve_id = args[0];
			String conninfo = null;
			conninfo = RUIPoc.getReservationInfoFromPOC(reserve_id);
			if( conninfo == null ) {
				RUIPoc.reportClientStatusToPOC(reserve_id, 9, 0, "Reservation Info Patch Error");
				JOptionPane.showMessageDialog(null, "Not found reservation information", RUIClient.TC_PRODUCT_NAME, JOptionPane.ERROR_MESSAGE);
				return;
			}
			
			String strSplit[] = conninfo.split("\\|", 4);
			if( strSplit.length != 4 ) {
				JOptionPane.showMessageDialog(null, "Invalid reservation information", RUIClient.TC_PRODUCT_NAME, JOptionPane.ERROR_MESSAGE);
				return;
			}
			
			//TCFrame tc = 
			new TCFrame(reserve_id, strSplit[0], Integer.valueOf(strSplit[1]));
			return;
		}else {
			Utils.LOG("not enough information");
			return;
		}
	}

	public void exit(int code) {
		Utils.LOG("Ending ....");
		System.exit(code);
	}
}
