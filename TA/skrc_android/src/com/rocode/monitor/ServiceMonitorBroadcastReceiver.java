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
package com.rocode.monitor;

import java.util.Iterator;
import java.util.List;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningServiceInfo;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.rocode.log.Log;
import com.rocode.skrc.RCService;

public class ServiceMonitorBroadcastReceiver extends BroadcastReceiver {

	private static final String TAG = "RCService";

	public ServiceMonitorBroadcastReceiver() {
	}

	public void onReceive(Context context, Intent intent) {
		if (intent.getAction() != null) {
			if (intent.getAction().equals(Intent.ACTION_TIME_TICK) && getServiceTaskName(context) == false) {
				context.startService(new Intent(intent.getAction(), null, context, RCService.class));
			}
		}
	}

	private boolean getServiceTaskName(Context context) {

		boolean checked = false;

		ActivityManager am = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);

		List<ActivityManager.RunningServiceInfo> info = am.getRunningServices(300);

		for (Iterator<ActivityManager.RunningServiceInfo> iterator = info.iterator(); iterator.hasNext();) {

			RunningServiceInfo runningTaskInfo = (RunningServiceInfo) iterator.next();

			Log.i(TAG, "Check Service name :" + runningTaskInfo.service.getClassName());

			if (runningTaskInfo.service.getClassName().equals(RCService.class.getName())) {

				checked = true;

				Log.i(TAG, "Service is.... : " + checked);

				break;
			}

		}

		return checked;

	}
}
