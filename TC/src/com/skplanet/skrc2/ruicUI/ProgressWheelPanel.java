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

import java.awt.AlphaComposite;
import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.geom.AffineTransform;

import javax.swing.JPanel;
import javax.swing.Timer;

@SuppressWarnings("serial")
public class ProgressWheelPanel extends JPanel {
	private double angleInDegrees = 1;
	private Timer rotatingTimer;
	public boolean started = false;

	public ProgressWheelPanel() {
	    this.setOpaque(false);
	    rotatingTimer = new Timer(100, new ActionListener() {
	        //            @Override
	        public void actionPerformed(ActionEvent e) {
	            angleInDegrees = angleInDegrees +15;
	            if (angleInDegrees == 360) {
	                angleInDegrees = 0;
	            }
	            if( started ) {
	            	repaint();
	            }
	        }
	    });
	    rotatingTimer.setRepeats(false);

	}
	
	@Override
	protected void paintComponent(Graphics g) {
	    super.paintComponent(g);
	    Graphics2D g2d = (Graphics2D) g.create();
	
	    rotatingTimer.stop();
	
	    g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
	    g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, 0.1f));
	    g2d.setColor(Color.WHITE);
	    g2d.fillRect(0,0,getWidth(), getHeight());
	    g2d.setStroke(new BasicStroke(10f, BasicStroke.CAP_SQUARE, BasicStroke.JOIN_MITER));
	    float alpha = 0.9f;
	    g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, alpha));
	    g2d.rotate(angleInDegrees * Math.PI / 180.0, getWidth() / 2.0, getHeight() / 2.0);
	    for( int i= 0; i < 360; i+= 15) {
	    	alpha -= 0.05;
	    	if( alpha < 0.3f) alpha = 0.3f;
		    g2d.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, alpha));
		    g2d.rotate(-15 * Math.PI / 180.0, getWidth() / 2.0, getHeight() / 2.0);
		    g2d.drawLine(getWidth() / 2 + 80, getHeight() / 2, getWidth() / 2 + 100, getHeight() / 2);
	    }
	    //**************************************************************************************
	    AffineTransform transformer = new AffineTransform();
	    transformer.translate(5,5);
	    transformer.scale(2,2);
	    g2d.getTransform().concatenate(transformer);
	    //***************************************************************************************
	    g2d.dispose();
	    if( started ) {
	    	rotatingTimer.start();
	    }
	}
	public void start(){
		started = true;
		this.setVisible(true);
	    rotatingTimer.start();
	}
	public void stop(){
		started = false;
		this.setVisible(false);
	  rotatingTimer.stop();
	}
}