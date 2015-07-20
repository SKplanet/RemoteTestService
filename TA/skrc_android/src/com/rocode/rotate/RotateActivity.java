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

import java.io.IOException;
import java.util.List;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.ResolveInfo;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

import com.rocode.log.Log;
import com.rocode.skrc.R;
import com.rocode.skrc.RCService;

public class RotateActivity extends Activity {
	protected static final String TAG = RotateActivity.class.getCanonicalName();

	protected IRotate mServiceBinder;
	private boolean isBinded = false;

	private ServiceConnection serviceCon = new ServiceConnection() {

		public void onServiceDisconnected(ComponentName className) {
			Log.e(TAG, "onServiceDisconnected is called.");
		}

		public void onServiceConnected(ComponentName className, IBinder binder) {
			Log.e(TAG, "onServiceConnected is called.");

			if (binder != null) {
				mServiceBinder = IRotate.Stub.asInterface(binder);
				isBinded = true;
			} else {
				isBinded = false;
			}
		}

	};

	private Intent serviceIntent;

	protected ComponentName mService = null;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.rotate);

		try {
			Runtime.getRuntime().exec("logcat -c");
		} catch (IOException e) {
		}

		serviceIntent = new Intent(RotateActivity.this, RCService.class);

		bindService(serviceIntent, serviceCon, BIND_AUTO_CREATE);

		Button r0 = (Button) findViewById(R.id.button_rotate_0);
		r0.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				try {
					mServiceBinder.rotate0();
				} catch (RemoteException e) {
				}

			}
		});

		Button r90 = (Button) findViewById(R.id.button_rotate_90);
		r90.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				try {
					mServiceBinder.rotate90();
				} catch (RemoteException e) {
				}

			}
		});

		Button r180 = (Button) findViewById(R.id.button_rotate_180);
		r180.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				try {
					mServiceBinder.rotate180();
				} catch (RemoteException e) {
				}

			}
		});

		Button r270 = (Button) findViewById(R.id.button_rotate_270);
		r270.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				try {
					mServiceBinder.rotate270();
				} catch (RemoteException e) {
				}

			}
		});

		Button rRecovery = (Button) findViewById(R.id.button_rotate_recovery);
		rRecovery.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				try {
					mServiceBinder.rotateRecovery();
				} catch (RemoteException e) {
				}

			}
		});

		Button reboot = (Button) findViewById(R.id.button_start_service);
		reboot.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {

				mService = startService(serviceIntent);

			}
		});

		Button rebootWipeCache = (Button) findViewById(R.id.button_stop_service);
		rebootWipeCache.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {

				stopService(serviceIntent);

				// final PackageManager pm = getPackageManager();
				//
				// List<ApplicationInfo> packages = pm
				// .getInstalledApplications(PackageManager.GET_META_DATA);
				//
				// for (ApplicationInfo packageInfo : packages) {
				//
				// Log.d(TAG, "Installed package :" + packageInfo.packageName);
				// Log.d(TAG,
				// "Launch Activity :"
				// + pm.getLaunchIntentForPackage(packageInfo.packageName));
				//
				// }

			}
		});

		Button userApks = (Button) findViewById(R.id.button_get_user_apks);
		userApks.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View view) {
				List<ApplicationInfo> applications = getPackageManager().getInstalledApplications(0);
				for (int n = 0; n < applications.size(); n++) {
					ApplicationInfo pkgInfo = applications.get(n);
					Log.d(TAG, pkgInfo.packageName);
				}

			}
		});

		Button clearLogcat = (Button) findViewById(R.id.button_clear_logcat);
		clearLogcat.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				try {
					Runtime.getRuntime().exec("logcat -c");
				} catch (IOException e) {
				}

			}
		});

	}

	public boolean isSystemPackage(PackageInfo pkgInfo) {
		return ((pkgInfo.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0) ? true : false;
	}

	private boolean isSystemPackage(ResolveInfo ri) {
		return ((ri.activityInfo.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0) ? true : false;

	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		if (isBinded == true) {
			unbindService(serviceCon);
		}

		// if (mService == null) {
		// return;
		// }
		// Intent i = new Intent();
		// i.setComponent(mService);
		// stopService(serviceIntent);

	}

}