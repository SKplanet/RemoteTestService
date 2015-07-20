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
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.RenderingHints;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionAdapter;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.net.MalformedURLException;

import javax.imageio.ImageIO;
import javax.swing.ImageIcon;
import javax.swing.JPanel;

import com.skplanet.skrc2.rui.RUIClient;
import com.skplanet.skrc2.rui.RUIService;
import com.skplanet.skrc2.rui.TransportException;
import com.skplanet.skrc2.rui.RUIKEY;
import com.skplanet.skrc2.utils.Utils;


@SuppressWarnings("serial")
public class RUICanvas extends JPanel {
	public static final int ICON_WIDTH = 128;
	
	public RUIService	tcService;
	public BufferedImage image = null;
	public long tickCount;
	public long downTick;
	public int downX, downY;
	//double scale;
	//double offx, offy;
	AffineTransform at;
	AffineTransform rat;
	int ix,iy,iw,ih;

	ImageIcon imageIdle;
	ImageIcon imagePower;
	
	int mouseDown;
	ProgressWheelPanel ing;
	public RUICanvas(RUIService c) {
		//scale = 1.0;
		//offx = offy = 0;
		setTCService(c);
		setBackground(Color.BLACK);
		//this.setOpaque(false);
		mouseDown = 0;


		imageIdle = Utils.getButtonIcon("skplanet.png");
		imagePower= Utils.getButtonIcon("power.png");
				
		ing = new ProgressWheelPanel();
		this.setLayout(new BorderLayout(10,5));
		this.add("Center", ing);
		ing.setSize(360,360);
		ing.setVisible(false);
				
						
      addMouseMotionListener(new MouseMotionAdapter() {

		@Override
      public void mouseDragged(MouseEvent arg0) {
          super.mouseDragged(arg0);
          if( tcService == null || tcService.isAlive() == false) return;
          
			Point pt = arg0.getPoint();
			if( calcPoint(pt) == false)
				return;
			try {
				if( mouseDown > 0) {
					int mX = tcService.fbcWidth;
					int mY = tcService.fbcHeight;
					if( (tcService.m_devState.getScreenRotate() & 0x01) != 0) {
						mX = tcService.fbcHeight;
						mY = tcService.fbcWidth;
					}
					if( pt.x < 0 || pt.y < 0 || pt.x > mX || pt.y > mY) {
				
						if( pt.x < 0) pt.x = 0;
						if( pt.y < 0) pt.y = 0;
						if( pt.x > mX) pt.x = mX-1;
						if( pt.y > mY) pt.y = mY-1;
						Utils.LOG("PT Move to UP(%d,%d)\n", pt.x, pt.y);
						tcService.mouseEvent(pt.x, pt.y, 2);
						tcService.mouseEvent(pt.x, pt.y, 0);
						mouseDown = 0;
						return;
					}
					long tickNow = System.currentTimeMillis();
					if( (tickNow - tickCount) > 10 ) {
						//Utils.LOG("PT Move (%d,%d)\n", pt.x, pt.y);
						tcService.mouseEvent(pt.x, pt.y, 2);
					} else {
						//Utils.LOG("SKIP Move");
					}
					tickCount = tickNow;
				}
			} catch (IOException e) {
				e.printStackTrace();
			}          
		}
      });
		
		addMouseListener(new MouseAdapter() {
			@Override
			public void mouseReleased(MouseEvent arg0) {
				super.mouseReleased(arg0);
	          
				if( tcService == null || tcService.isAlive() == false) return;
				
				Point pt = arg0.getPoint();
				if( calcPoint(pt) == false)
					return;
				try {
					if( mouseDown > 0) {
						long upTick = System.currentTimeMillis();
						int diffY = (downY > pt.y) ? downY - pt.y : pt.y - downY;
						int diffX = (downX > pt.x) ? downX - pt.x : pt.x - downX;
						int nTurbo = 0;
						//Utils.LOG("DiffX(%d), DiffY(%d), tickDiff(%d)\n", diffX, diffY, (upTick-downTick));
						if( (upTick - downTick < 400) && diffX > 60 && diffY < 30) {
							//tcService.mouseEvent(pt.x, pt.y, 2);
							Utils.LOG("Turbo Drag enabled!(%d)(%d)\n", downX, pt.x);
							if( downX < pt.x) { 
								int mX = tcService.fbcWidth;
								if( (tcService.m_devState.getScreenRotate() & 0x01) != 0) {
									mX = tcService.fbcHeight;
								}
								while( pt.x + nTurbo*10 < mX && nTurbo++ < 6) {
									pt.x += nTurbo*10;
									//Utils.LOG("PT Move (%d,%d)\n", pt.x, pt.y);
									tcService.mouseEvent(pt.x, pt.y, 2);
									try {
										Thread.sleep(50);
									} catch (InterruptedException e) {
										e.printStackTrace();
									}
								}
								pt.x = mX - 1;
								tcService.mouseEvent(pt.x, pt.y, 2);
							}
							else {
								while( pt.x - nTurbo*10 > 0  && nTurbo++ < 6) {
									pt.x -= nTurbo*10;
									//Utils.LOG("PT Move (%d,%d)\n", pt.x, pt.y);
									tcService.mouseEvent(pt.x, pt.y, 2);
									try {
										Thread.sleep(50);
									} catch (InterruptedException e) {
										e.printStackTrace();
									}
								}
								pt.x = 1;
								tcService.mouseEvent(pt.x, pt.y, 2);
							}
							//tcService.mouseEvent(pt.x, pt.y, 2);
						}
						
						//Utils.LOG("PT Up (%d,%d)\n", pt.x, pt.y);
						tcService.mouseEvent(pt.x, pt.y, 0);
						mouseDown = 0;
					}
				} catch (IOException e) {
					e.printStackTrace();
				}
			}

			@Override
            public void mousePressed(MouseEvent arg0) {
                super.mousePressed(arg0);

				if( tcService == null || tcService.isAlive() == false) return;
	          if( arg0.getButton() != MouseEvent.BUTTON1 ) {
					try {
						if( arg0.getButton() == MouseEvent.BUTTON2) {
						tcService.keyEvent(RUIKEY.AKEYCODE_MENU, 1);
						tcService.keyEvent(RUIKEY.AKEYCODE_MENU, 0);
						} else {
							tcService.keyEvent(RUIKEY.AKEYCODE_BACK, 1);
							tcService.keyEvent(RUIKEY.AKEYCODE_BACK, 0);
						}
					} catch (IOException e) {
					}
					return;
	          }

				Point pt = arg0.getPoint();
				if( calcPoint(pt) == false)
					return;
				try {
					if( mouseDown == 0) {
						int mX = tcService.fbcWidth;
						int mY = tcService.fbcHeight;
						if( (tcService.m_devState.getScreenRotate() & 0x01) != 0) {
							mX = tcService.fbcHeight;
							mY = tcService.fbcWidth;
						}
						if( pt.x >= 0 && pt.y >= 0 && pt.x <= mX && pt.y <= mY) {
							//Utils.LOG("PT Down (%d,%d)\n", pt.x, pt.y);
							tickCount = System.currentTimeMillis();
							downTick = tickCount;
							downX = pt.x;
							downY = pt.y;
							tcService.mouseEvent(pt.x, pt.y, 1);
							mouseDown = 1;
						}
					}
				} catch (IOException e) {
					e.printStackTrace();
				}
            }
        });

	}

	public void setTCService( RUIService svc ) {
		tcService = svc;
	}
	public boolean calcPoint(Point pt) {
		if( pt.x >= ix && pt.x <= ix+iw && pt.y >= iy && pt.y <= iy+ih)
		{
			Utils.LOG("WAKEUP");
			try {
				tcService.wakeUP();
			} catch (TransportException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
			return false;
		}
		//System.out.print(pt.x+":"+pt.y+" to ");
		rat.transform(pt, pt);
		//System.out.println(pt.x+":"+pt.y);
		return true;
	}
	/* (non-Javadoc)
	 * @see javax.swing.JComponent#paintComponent(java.awt.Graphics)
	 */
	protected void paintComponent(Graphics g) {
		super.paintComponent(g);
		
		if( image == null ) {
			ImageIcon stateImage = imageIdle;
			if( tcService != null && tcService.isAlive() && tcService.m_devState.isScreenOn() == false) {
				stateImage = imagePower;
			}
			if( tcService != null  && tcService.tc.state ==  RUIClient.STATE.CONNECTING) {
				ing.start();
			} else {
				ing.stop();
			}
			Graphics2D g2 = (Graphics2D)g;
			g2.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_BICUBIC);
			
			int w = getWidth();
			int h = getHeight();
			int imageWidth = stateImage.getIconWidth();
			int imageHeight = stateImage.getIconHeight();
			
			ix = w /2 - imageWidth /2;
			iy = h /2 - imageHeight /2;
			if( ix<0 ) ix = 0;
			if( iy< 0) iy = 0;
			ih = ICON_WIDTH;
			iw = ICON_WIDTH;
			if( w < iw ) 
			{
				if( w < ICON_WIDTH ) { 
					iw = w; ih = w;
				}
			} else {
				if( h < ICON_WIDTH ) {
					iw = h; ih = h;
				}
			}

			g.drawImage(stateImage.getImage(), ix, iy, iw, ih, null);
			//stateImage.paintIcon(this, g, ix, iy);
			return;
		}
		
		if( ing.started) {
			ing.stop();
		}
		ix = iy = ih =iw = -1;
		Graphics2D g2 = (Graphics2D)g;
		g2.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_BICUBIC);
		
		int nRotate = tcService.m_devState.getScreenRotate();
		int w = getWidth();
		int h = getHeight();
		int imageWidth, imageHeight;
		if( nRotate == 1 || nRotate == 3) {
			imageWidth = image.getHeight();
			imageHeight = image.getWidth();
		} else {
			imageWidth = image.getWidth();
			imageHeight = image.getHeight();
		}
		
		double scalex = (double)w / (double)imageWidth;
		double scaley = (double)h / (double)imageHeight;
		double scale;
		double offx, offy;
		scale = scalex < scaley ? scalex : scaley;
		
		offx = (w - scale * imageWidth)/2;
		offy = (h - scale * imageHeight)/2;
		at = AffineTransform.getTranslateInstance(offx, offy);
		at.scale(scale, scale);
		//System.out.print("offset:"+offx+":"+offy+" scale("+scale+")");


		rat = AffineTransform.getScaleInstance(
				(tcService.fbcWidth/ (double)image.getWidth()),
				(tcService.fbcHeight/(double)image.getHeight()));
		
		if( nRotate == 1) {
			//System.out.println("rotate -90 : 0, " + imageHeight);
			at.translate(0, imageHeight);
			at.rotate(Math.toRadians(-90));

			//rat.rotate(Math.toRadians(90));
			//rat.translate(0, -imageHeight);
			
		}
		else if( nRotate == 2) {
			//System.out.println("rotate -180 : " + imageWidth + ", " + imageHeight);
			at.translate(imageWidth, imageHeight);
			at.rotate(Math.toRadians(-180));

			//rat.rotate(Math.toRadians(180));
			//rat.translate(-imageWidth,  -imageHeight);
		}
		else if( nRotate == 3) {
			//System.out.println("rotate -270 : " + imageWidth + ", 0");
			at.translate(imageWidth, 0);
			at.rotate(Math.toRadians(-270));

			//rat.rotate(Math.toRadians(270));
			//rat.translate(-imageWidth,  0);
		} else {
			//System.out.println("rotate 0 : 0, 0");

			//rat = AffineTransform.getRotateInstance(Math.toRadians(0));
			//rat.translate(0,  0);
		}
		rat.scale(1/scale, 1/scale);
		rat.translate(-offx, -offy);

		g2.drawRenderedImage(image, at);
	}
	
	public Dimension getPreferredSize() {  
		int w,h;
		
		{
			w = this.getParent().getWidth();
			h = this.getParent().getHeight();
		}
        return new Dimension(w, h);  
    }  
   
    
    public void loadImage(String fileName)  
    {  

        try  
        {  
        	image = ImageIO.read( new File(fileName));
        }  
        catch(MalformedURLException mue)  
        {  
        	Utils.LOG("URL trouble: " + mue.getMessage());  
        }  
        catch(IOException ioe)  
        {  
        	Utils.LOG("read trouble: " + ioe.getMessage());  
        }  
    }
    
    public void loadImage(byte[] buffer)  
    {  
    	if( buffer == null || buffer.length == 0 )
    	{
    		image = null;
    		return;
    	}

        try  
        {  
        	image = ImageIO.read( new ByteArrayInputStream(buffer));
        }  
        catch(MalformedURLException mue)  
        {  
        	Utils.LOG("URL trouble: " + mue.getMessage());  
        }  
        catch(IOException ioe)  
        {  
        	Utils.LOG("read trouble: " + ioe.getMessage());  
        }  
    }      
}

