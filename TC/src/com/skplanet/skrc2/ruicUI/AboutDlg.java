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
import com.skplanet.skrc2.utils.Utils;

@SuppressWarnings("serial")
public class AboutDlg extends JDialog {

	public RUIClient tc;
	
	public AboutDlg(RUIClient c) {
		super(c.uiWindow, "About TC", true);
		tc = c;
		if( c.uiWindow != null ) {
			Dimension parentSize = c.uiWindow.getSize();
			Point p = c.uiWindow.getLocation();
			
			setLocation(p.x + parentSize.width / 4, p.y + parentSize.height / 4);
		}
		
		JLabel picLabel = new JLabel( Utils.getButtonIcon("TC.png", 40, 40));
		JPanel msgPanel = new JPanel();
		
		msgPanel.setLayout(new BoxLayout(msgPanel, BoxLayout.Y_AXIS));
	    JLabel bV1 = new JLabel(RUIClient.TC_PRODUCT_NAME);
	    bV1.setAlignmentX(CENTER_ALIGNMENT);
	    msgPanel.add(bV1);

	    JLabel bV2 = new JLabel(RUIClient.TC_COPYRIGHT);
	    bV2.setAlignmentX(CENTER_ALIGNMENT);
	    msgPanel.add(bV2);
	    
	    JLabel bV3 = new JLabel(RUIClient.TC_VERSION);
	    bV3.setAlignmentX(CENTER_ALIGNMENT);
	    msgPanel.add(bV3);

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
	    picLabel.setBorder(BorderFactory.createEmptyBorder(10, 10, 0, 10));
	    currentPane.add(picLabel, BorderLayout.WEST);
	    
	    msgPanel.setBorder(BorderFactory.createEmptyBorder(10, 10, 0, 10));
	    currentPane.add(msgPanel, BorderLayout.CENTER);
	    
	    pBtn.setBorder(BorderFactory.createEmptyBorder(0, 0, 10, 10));
	    currentPane.add(pBtn, BorderLayout.SOUTH);
	    
	    pack();
	    this.setResizable(false);
	    this.setVisible(false);
	}
	
	public void doModal() {
		this.setVisible(true);
	}
}
