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

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;

import com.skplanet.skrc2.rui.RUIClient;

@SuppressWarnings("serial")
public class DevInfoDlg extends JDialog /*implements ActionListener*/ {

	public RUIClient tc;
	
	
	public DevInfoDlg(RUIClient c) {
		super(c.uiWindow, "Device Information", true);
		tc = c;
		if( c.uiWindow != null ) {
			Dimension parentSize = c.uiWindow.getSize();
			Point p = c.uiWindow.getLocation();
			
			setLocation(p.x + parentSize.width / 4, p.y + parentSize.height / 4);
		}
		
	    JPanel pSubject = new JPanel();
	    pSubject.setLayout(new BoxLayout(pSubject, BoxLayout.Y_AXIS));

	    JLabel bS1 = new JLabel("Model:");
	    bS1.setAlignmentX(RIGHT_ALIGNMENT);
	    pSubject.add(bS1);

	    JLabel bS2 = new JLabel("Manufacturer:");
	    bS2.setAlignmentX(RIGHT_ALIGNMENT);
	    pSubject.add(bS2);

	    JLabel bS3 = new JLabel("Build:");
	    bS3.setAlignmentX(RIGHT_ALIGNMENT);
	    pSubject.add(bS3);
	    
	    JLabel bS4 = new JLabel("SDK version:");
	    bS4.setAlignmentX(RIGHT_ALIGNMENT);
	    pSubject.add(bS4);
	    
	    JLabel bS5 = new JLabel("Serial:");
	    bS5.setAlignmentX(RIGHT_ALIGNMENT);
	    pSubject.add(bS5);

	    JLabel bS6 = new JLabel("Screen Width:");
	    bS6.setAlignmentX(RIGHT_ALIGNMENT);
	    pSubject.add(bS6);

	    JLabel bS7 = new JLabel("Screen Height:");
	    bS7.setAlignmentX(RIGHT_ALIGNMENT);
	    pSubject.add(bS7);

	    
	    JPanel pVal = new JPanel();
	    pVal.setLayout(new BoxLayout(pVal, BoxLayout.Y_AXIS));
	    
	    JLabel bV1 = new JLabel(tc.tcService.devBuildModel);
	    bV1.setAlignmentX(LEFT_ALIGNMENT);
	    pVal.add(bV1);

	    JLabel bV2 = new JLabel(tc.tcService.devBuildManufacture);
	    bV2.setAlignmentX(LEFT_ALIGNMENT);
	    pVal.add(bV2);


	    JLabel bV3 = new JLabel(tc.tcService.devBuildVersionRelease);
	    bV3.setAlignmentX(LEFT_ALIGNMENT);
	    pVal.add(bV3);

	    JLabel bV4 = new JLabel(tc.tcService.devBuildSDKInt);
	    bV4.setAlignmentX(LEFT_ALIGNMENT);
	    pVal.add(bV4);

	    JLabel bV5 = new JLabel(tc.tcService.devBuildSerial);
	    bV5.setAlignmentX(LEFT_ALIGNMENT);
	    pVal.add(bV5);

	    JLabel bV6 = new JLabel(Integer.toString(tc.tcService.fbcWidth));
	    bV6.setAlignmentX(LEFT_ALIGNMENT);
	    pVal.add(bV6);

	    JLabel bV7 = new JLabel(Integer.toString(tc.tcService.fbcHeight));
	    bV7.setAlignmentX(LEFT_ALIGNMENT);
	    pVal.add(bV7);


	    JPanel pBtn = new JPanel();
	    pBtn.setLayout(new BoxLayout(pBtn, BoxLayout.Y_AXIS));
	    
	    JButton btnOK = new JButton("  OK  ");
	    btnOK.setAlignmentX(RIGHT_ALIGNMENT);
	    btnOK.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent arg0) {
				dispose();
			}
	    });
	    pBtn.add(btnOK);
	    
	    Container currentPane = getContentPane();
	    
	    currentPane.setLayout(new BorderLayout(15,1));
	    pSubject.setBorder(BorderFactory.createEmptyBorder(5, 5, 0, 5));
	    currentPane.add(pSubject, BorderLayout.WEST);
	    
	    pVal.setBorder(BorderFactory.createEmptyBorder(5, 5, 0, 5));
	    currentPane.add(pVal, BorderLayout.EAST);

	    pBtn.setBorder(BorderFactory.createEmptyBorder(0, 0, 5, 5));
	    currentPane.add(pBtn, BorderLayout.SOUTH);
	    
	    //this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	    pack();
	    this.setResizable(false);
	    this.setVisible(false);
	}
	
	public void doModal() {
		this.setVisible(true);
	}

}
