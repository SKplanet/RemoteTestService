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



package com.skplanet.skrc2.utils;

import java.awt.Dialog;
import java.awt.GraphicsEnvironment;
import java.awt.Image;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.Window;
import java.net.URL;
import java.util.LinkedList;
import java.util.List;

import javax.swing.ImageIcon;
import javax.swing.JDialog;

public class Utils {
	private static List<Image> icons = null;
	//private static boolean LOG_MSG_ON	= true;
	private static boolean LOG_MSG_ON	= false;

	public static void LOG(String x) {
		LOG_MSG_ON = Settings.getShowLog();
		if( LOG_MSG_ON)
			System.out.println(x);
	}

	public static void LOG(String format, Object ... args) {
		if( LOG_MSG_ON)
			System.out.printf(format, args);
	}
	
	public static synchronized List<Image> getApplicationIcons() {
		if (icons != null) {
			return icons;
		}
		icons = new LinkedList<Image>();
		URL resource = Utils.class.getResource("/com/skplanet/skrc2/images/TC.png");
		Image image = resource != null ?
				Toolkit.getDefaultToolkit().getImage(resource) :
				null;
		if (image != null) {
			icons.add(image);
		}
		return icons;
	}

	public static ImageIcon getButtonIcon(String name) {
		URL resource = Utils.class.getResource("/com/skplanet/skrc2/images/"+name);
		//URL resource = Utils.class.getResource("/com/skplanet/skrc2/images/close.png");
		return resource != null ? new ImageIcon(resource) : null;
	}
	public static ImageIcon getButtonIcon(String name, int width, int height) {
		URL resource = Utils.class.getResource("/com/skplanet/skrc2/images/"+name);
		//URL resource = Utils.class.getResource("/com/skplanet/skrc2/images/close.png");

		if( resource != null ) {
			ImageIcon icon = new ImageIcon(resource);
			Image iconimage = icon.getImage();
		    Image sicon = iconimage.getScaledInstance(width, height, Image.SCALE_SMOOTH);
		    return (new ImageIcon(sicon));
		}
		
		return null;
	}

	

	
}
