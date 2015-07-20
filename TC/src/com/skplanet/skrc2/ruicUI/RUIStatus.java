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


package com.skplanet.skrc2.ruicUI;

import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Image;
import java.util.Timer;
import java.util.TimerTask;

import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.SwingConstants;
import javax.swing.border.BevelBorder;

import com.skplanet.skrc2.rui.RUIClient;

@SuppressWarnings("serial")
public class RUIStatus extends JPanel {
	public RUIClient tc;
	private JLabel statusLabel;
	private JLabel timeLabel;
	private Icon statusIcon;
	private Image greenImage;
	private Image redImage;
	private GridBagLayout gBag;
	private boolean bGreen = true;
	private long serverValidTime = 0;
	private Timer validTimer = null;
	private boolean showDisconnWarning = true;
	
	public RUIStatus(RUIClient c) {
		super();
		tc = c;
		setPreferredSize(new Dimension(500,20));
		gBag = new GridBagLayout();
		this.setLayout(gBag);
		this.setBorder(new BevelBorder(BevelBorder.LOWERED));
		statusLabel = new JLabel("Ready");
		statusLabel.setHorizontalAlignment(SwingConstants.LEFT);
		
		timeLabel = new JLabel("00:00:00 Left");
		timeLabel.setHorizontalAlignment(SwingConstants.RIGHT);
		
		GridBagConstraints gbc = new GridBagConstraints();
		gbc.gridx = 0;
		gbc.gridy = 0;
		gbc.weightx = 0.98;
		gbc.anchor = GridBagConstraints.WEST;
		gbc.fill = GridBagConstraints.REMAINDER;
		this.gBag.setConstraints(statusLabel, gbc);
		this.add(statusLabel);
		
		statusIcon =  new Icon("red.png", 10, 10);
		redImage = statusIcon.getImage();
		
		statusIcon =  new Icon("green.png", 10, 10);
		greenImage = statusIcon.getImage();
		gbc.gridx = 1;
		gbc.gridy = 0;
		gbc.weightx = 0.001;
		//gbc.insets = new Insets(4,4,4,4);
		gbc.fill = GridBagConstraints.HORIZONTAL;
		gbc.anchor = GridBagConstraints.EAST;
		this.gBag.setConstraints(statusIcon, gbc);
		this.add(statusIcon);
		
		gbc.gridx = 2;
		gbc.gridy = 0;
		gbc.weightx = 0.019;
		gbc.fill = GridBagConstraints.BOTH;
		gbc.anchor = GridBagConstraints.EAST;
		this.gBag.setConstraints(timeLabel, gbc);
		this.add(timeLabel);
		
		//setLayout(new BoxLayout(this, BoxLayout.X_AXIS));
		validTimer = null; //new Timer();
		timeLabel.setVisible(false);
		statusIcon.setVisible(false);
	}
	
	public void setText(String text) {
		statusLabel.setText(text);
	}
	
	public void setServerValidTime(long ltime) {
		serverValidTime = ltime;
	}
	
	public void setConnect(boolean connect) {
		timeLabel.setVisible(connect);
		statusIcon.setVisible(connect);
		if( connect ) {
			validTimer = new Timer();
			validTimer.scheduleAtFixedRate( new TickTask(), 0, 1000);
			showDisconnWarning = true;
		} else {
			if( validTimer != null ) {
				validTimer.cancel();
				validTimer = null;
			}
			showDisconnWarning = false;
			serverValidTime = 0;
		}
		this.repaint();
	}
	
	public void onScreenBuf() {
		bGreen = !bGreen;
		if( bGreen ) {
			statusIcon.setImage(greenImage);
		} else {
			statusIcon.setImage(redImage);
		}
		
		statusIcon.repaint();
	}

	class TickTask extends TimerTask {
		public void run() {
			if( serverValidTime > 3) {
				serverValidTime--;
				
				int hour	= (int)(serverValidTime / 3600);
				int min	= (int)(serverValidTime - hour*3600) / 60;
				int sec	= (int)(serverValidTime - hour*3600 - min*60);
	        	
	        	String msg = String.format("%02d:%02d:%02d Left", hour, min, sec);
	        	timeLabel.setText(msg);

	        	//tc.tcLogcat.addLog("Test", msg, msg, "TC", "Test Message");
	        	
	        	if(hour ==0 && min <= 4 && showDisconnWarning == true) {
	        		showDisconnWarning = false;
	        		final String warn = String.format("%d minutes left to disconnect", min+1);
	        		
	        		Runnable msgrun = new Runnable() {

						@Override
						public void run() {
							JOptionPane.showMessageDialog(null, warn, RUIClient.TC_PRODUCT_NAME, JOptionPane.WARNING_MESSAGE);
						}
	        		};
	        		Thread msgT = new Thread(msgrun);
	        		msgT.start();
	        	}
	        	
			} else {
				timeLabel.setVisible(false);
			}
		}
	}
}
