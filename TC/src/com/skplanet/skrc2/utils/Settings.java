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

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Properties;

public class Settings
{
    private static Properties p = new Properties();
    public static final String propertyFilename = System.getProperty("user.home") +
                                                  File.separator +
                                                  ".skrc2/skrc2.properties".replace('/',
                                                                                  File.separatorChar);

    public static boolean IS_JAVA_1_6 = true;
    

    public static final String defaultWidth = "500";
    public static final String defaultHeight = "800";
    public static final int connectionTimeout = 30000;
    public static final String defaultWorkDir = System.getProperty("user.home");
    public static final String userHomeDir = System.getProperty("user.home");
    public static final String appHomeDir = userHomeDir +
                                            "/.skrc2/".replace('/',
                                                              File.separatorChar);
    public static final String defaultLogWindow = "20"; // 20% of main window height
    public static final String defaultShowToolbar = "true";
    public static final String defaultShowStatuslbar = "true";
    public static final String defaultShowButtonbar = "true";
    public static final String defaultShowLog = "false";
    

    public static Object setProperty(String key, String value)
    {
        return p.setProperty(key, value);
    }

    public static String getProperty(String key) {
    	return ""+p.getProperty(key);
    }
    
    public static Object setProperty(String key, int value)
    {
        return p.setProperty(key, Integer.toString(value));
    }

    public static Object setProperty(String key, boolean value)
    {
        String val = "false";

        if(value)
        {
            val = "true";
        }

        return p.setProperty(key, val);
    }

    public static void save()
    {
        File fsave = new File(System.getProperty("user.home") + File.separator +
                ".skrc2");
        if( !fsave.exists()) {
	        if( fsave.mkdir() == false) {
	        	System.out.println("Cannot save properties...");
	        	return;
	        }
        }
        try {
        	FileOutputStream fos = new FileOutputStream(propertyFilename);
			p.store(fos, "skrc2.properties");
			fos.close();
		} catch (FileNotFoundException e1) {
			e1.printStackTrace();
			System.out.println("Cannot save properties...");
		} catch (IOException e1) {
			e1.printStackTrace();
			System.out.println("Cannot save properties...");
		}

    }


    public static java.awt.Dimension getWindowSize()
    {
        int width = Integer.parseInt(p.getProperty("window.width",
                                                   defaultWidth));
        int height = Integer.parseInt(p.getProperty("window.height",
                                                    defaultHeight));

        if(width < 100 || height < 100) {
        	width = Integer.parseInt(defaultWidth);
        	height = Integer.parseInt(defaultHeight);
        }
        
        return new java.awt.Dimension(width, height);
    }
    
    public static void setWindowSize(int width, int height)
    {
    	p.setProperty("window.width", Integer.toString(width));
    	p.setProperty("window.height", Integer.toString(height));
    }
    
    public static String getWorkingDirectory() {
    	String dir =  p.getProperty("dir.working", defaultWorkDir);
    	File test = new File(dir);
    	
    	if(!test.exists()) {
    		return defaultWorkDir;
    	}
    	
    	return p.getProperty("dir.working", defaultWorkDir);
    }
    
    public static void setWorkingDirectory(String dir) {
    	p.setProperty("dir.working", dir);
    }

    public static double getLogcatWindowSize() {
    	int h = Integer.parseInt(p.getProperty("window.logcat", defaultLogWindow));
    	if( h < 10) h = 10;
    	if( h > 90) h = 90;
    	return (double)(h/100.0);
    }
    
    public static void setLogcatWindowSize(double f) {
    	int h =  (int)(f*100.0);
    	//Utils.LOG("weight:%f\n", f);
    	p.setProperty("window.logcat", Integer.toString(h));
    }
    static
    {
        try
        {
        	FileInputStream fis = new FileInputStream(propertyFilename);
            p.load(fis);
            fis.close();
        }
        catch(Exception e)
        {
            System.out.println("no property file loaded, using defaults... (" +
                               e + ")");
        }
    }
    
    public static boolean getShowToolbar() {
    	boolean bshow = Boolean.parseBoolean(p.getProperty("show.toolbar", defaultShowToolbar));
    	return bshow;
    }
    public static void setShowToolbar(boolean bShow) {
    	p.setProperty("show.toolbar", bShow ? "true":"false");
    }
    public static boolean getShowStatusbar() {
    	boolean bshow = Boolean.parseBoolean(p.getProperty("show.statusbar", defaultShowStatuslbar));
    	return bshow;
    }
    public static void setShowStatusbar(boolean bShow) {
    	p.setProperty("show.statusbar", bShow ? "true":"false");
    }
    public static boolean getShowButtonbar() {
    	boolean bshow = Boolean.parseBoolean(p.getProperty("show.buttonbar", defaultShowButtonbar));
    	return bshow;
    }
    public static void setShowButtonbar(boolean bShow) {
    	p.setProperty("show.buttonbar", bShow ? "true":"false");
    }
    
    public static boolean getShowLog() {
    	boolean bshow = Boolean.parseBoolean(p.getProperty("show.log", defaultShowLog));
    	return bshow;
    }
}
