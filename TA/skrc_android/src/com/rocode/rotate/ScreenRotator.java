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
package com.rocode.rotate;

import android.content.Context;
import android.content.pm.ActivityInfo;
import android.os.Handler;
import android.provider.Settings;
import android.view.Surface;
import android.view.View;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;

public class ScreenRotator {
	
	private static ScreenRotator screenRotator;
	private LayoutParams layoutParams;
	private View view;
	private WindowManager windowManager;
	private Context context;
	private Handler handler;
	
	private Runnable accelerometer_rotation0 = new Runnable() {

		@Override
		public void run() {
			android.provider.Settings.System.putInt(ScreenRotator.this.context.getContentResolver(), "accelerometer_rotation", 0);

		}
	};

	private Runnable accelerometer_rotation1 = new Runnable() {

		@Override
		public void run() {
			android.provider.Settings.System.putInt(ScreenRotator.this.context.getContentResolver(), "accelerometer_rotation", 1);

		}
	};

	public static final int SCREEN_ORIENTATION_PORTRAIT = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
	public static final int SCREEN_ORIENTATION_LANDSCAPE = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
	public static final int SCREEN_ORIENTATION_REVERSE_LANDSCAPE = 8;
	public static final int SCREEN_ORIENTATION_REVERSE_PORTRAIT = 9;
	public static final int SCREEN_ORIENTATION_FULL_SENSOR = 10;
	
	public static ScreenRotator getScreenRotator(Context context) {
		if (screenRotator != null) {
			return screenRotator;
		}
		
		screenRotator = new ScreenRotator(context);
		return screenRotator;
	}

	private ScreenRotator(Context context) {
		layoutParams = new LayoutParams(0, 0, 2005, 8, -3);
		layoutParams.gravity = 48;
		layoutParams.screenOrientation = SCREEN_ORIENTATION_FULL_SENSOR;

		view = new View(context);

		windowManager = (WindowManager) context.getSystemService("window");
		
		this.context = context;
		
		this.handler = new Handler();
		
	}
	
	public void rotate0() {
		layoutParams.screenOrientation = SCREEN_ORIENTATION_PORTRAIT;

		Settings.System.putInt(this.context.getContentResolver(), "user_rotation", Surface.ROTATION_0);
		
		this.handler.post(accelerometer_rotation0);
		
		if (view.getParent() != null)
			windowManager.updateViewLayout(view, layoutParams);
		else
			windowManager.addView(view, layoutParams);

	}

	public void rotate90() {
		layoutParams.screenOrientation = SCREEN_ORIENTATION_LANDSCAPE;

		Settings.System.putInt(this.context.getContentResolver(), "user_rotation", Surface.ROTATION_90);
		
		this.handler.post(accelerometer_rotation0);
		
		if (view.getParent() != null)
			windowManager.updateViewLayout(view, layoutParams);
		else
			windowManager.addView(view, layoutParams);

	}

	public void rotate180() {
		layoutParams.screenOrientation = SCREEN_ORIENTATION_REVERSE_PORTRAIT; //ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT;

		Settings.System.putInt(this.context.getContentResolver(), "user_rotation", Surface.ROTATION_180);
		
		this.handler.post(accelerometer_rotation0);

		if (view.getParent() != null)
			windowManager.updateViewLayout(view, layoutParams);
		else
			windowManager.addView(view, layoutParams);

	}

	public void rotate270() {
		layoutParams.screenOrientation = SCREEN_ORIENTATION_REVERSE_LANDSCAPE; //ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;

		Settings.System.putInt(this.context.getContentResolver(), "user_rotation", Surface.ROTATION_270);
		
		this.handler.post(accelerometer_rotation0);
		
		if (view.getParent() != null)
			windowManager.updateViewLayout(view, layoutParams);
		else
			windowManager.addView(view, layoutParams);

	}

	public void rotateRecovery() {
		layoutParams.screenOrientation = SCREEN_ORIENTATION_FULL_SENSOR; //ActivityInfo.SCREEN_ORIENTATION_FULL_SENSOR;

		this.handler.post(accelerometer_rotation1);
		if (view.getParent() != null)
			windowManager.removeView(view);

	}
	
	public static void rotate(int degree) {
		switch(degree) {
		case 0:
			screenRotator.rotate0();
			break;
		case 90:
			screenRotator.rotate90();
			break;
		case 180:
			screenRotator.rotate180();
			break;
		case 270:
			screenRotator.rotate270();
			break;
		default:
			screenRotator.rotateRecovery();
			break;
				
		}
	}

	public int getRotation() {
		return this.layoutParams.screenOrientation;
	}
}
