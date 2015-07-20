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


package com.rocode.info;

import java.io.File;
import java.io.IOException;
import java.security.GeneralSecurityException;
import java.util.ArrayList;
import java.util.List;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.BatteryManager;
import android.os.Build;
import android.os.Handler;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RecoverySystem;
import android.telephony.TelephonyManager;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.WindowManager;

import com.rocode.rotate.ScreenRotator;

public class DeviceControl {

	private Context context;
	private PowerManager pm;
	private WindowManager wm;
	private TelephonyManager tm;
	private Display display;
	private DisplayMetrics metrics;
	protected int batteryTemperature;
	private WakeLock wl;
	private boolean wakeLock;
	private ScreenRotator screenRotator;
	private Handler handler;

	protected int batteryLevel;

	private ArrayList<String> exceptPkgNameArray = null;
	private static final String[] exceptPkgNames = { "" };

	private Runnable rotate0 = new Runnable() {

		@Override
		public void run() {
			screenRotator.rotate0();
		}
	};

	private Runnable rotate90 = new Runnable() {

		@Override
		public void run() {
			screenRotator.rotate90();
		}
	};

	private Runnable rotate180 = new Runnable() {

		@Override
		public void run() {
			screenRotator.rotate180();
		}
	};

	private Runnable rotate270 = new Runnable() {

		@Override
		public void run() {
			screenRotator.rotate270();
		}
	};

	private Runnable rotateRecovery = new Runnable() {

		@Override
		public void run() {
			screenRotator.rotateRecovery();
		}
	};

	private static DeviceControl deviceControl = null;

	public static DeviceControl getDeviceControl(Context context) {
		if (deviceControl == null) {
			deviceControl = new DeviceControl(context);
		}

		return deviceControl;
	}

	public DeviceControl(Context context) {
		this.context = context;

		tm = (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);

		pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);

		wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);

		display = wm.getDefaultDisplay();

		metrics = new DisplayMetrics();
		display.getMetrics(metrics);

		context.registerReceiver(this.mBatInfoReceiver, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));

		wl = pm.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK, "SKRC");

		screenRotator = ScreenRotator.getScreenRotator(context);

		handler = new Handler();

		exceptPkgNameArray = new ArrayList<String>();
		exceptPkgNameArray.add(context.getPackageName());

		for (String pkgName : exceptPkgNames) {
			exceptPkgNameArray.add(pkgName);
		}
	}

	public void clean() {
		try {
			context.unregisterReceiver(this.mBatInfoReceiver);
		} catch (Exception ignore) {
		}

		deviceControl = null;

		if (wakeLock) {
			wl.release();
		}
	}

	// tm info

	public String getLine1Number() {
		return tm.getLine1Number() == null ? "" : tm.getLine1Number();
	}
	
	public String getSimSerialNumber() {
		return tm.getSimSerialNumber() == null ? "" : tm.getSimSerialNumber();
	}

	public String getNetworkOperator() {
		return tm.getNetworkOperator();
	}

	public String getNetworkOperatorName() {
		return tm.getNetworkOperatorName();
	}

	public boolean isSimStateAbsent() {
		if (tm.getSimState() == TelephonyManager.SIM_STATE_ABSENT) {
			// 유심이 없는 경우
			return true;
		}

		// 유심이 존재하는 경우
		return false;
	}

	public boolean isSimStateNetworkLocked() {
		if (tm.getSimState() == TelephonyManager.SIM_STATE_NETWORK_LOCKED) {
			return true;
		}

		return false;
	}

	public boolean isSimStatePinRequired() {
		if (tm.getSimState() == TelephonyManager.SIM_STATE_PIN_REQUIRED) {
			return true;
		}

		return false;
	}

	public boolean isSimStatePUKRequired() {
		if (tm.getSimState() == TelephonyManager.SIM_STATE_PUK_REQUIRED) {
			return true;
		}

		return false;
	}

	public boolean isSimStateReady() {
		if (tm.getSimState() == TelephonyManager.SIM_STATE_READY) {
			return true;
		}

		return false;
	}

	public boolean isSimStateUnknown() {
		if (tm.getSimState() == TelephonyManager.SIM_STATE_UNKNOWN) {
			return true;
		}

		return false;
	}

	public String getDeviceId() {
		String deviceId = tm.getDeviceId();

		return deviceId == null ? "" : deviceId;
	}

	public String getDeviceSoftwareVersion() {
		String deviceSoftwareVersion = tm.getDeviceSoftwareVersion();

		return deviceSoftwareVersion == null ? "" : deviceSoftwareVersion;
	}

	// pm info
	public boolean isScreenOn() {
		return pm.isScreenOn();
	}

	// public void goToSleep(long time) {
	// pm.goToSleep(time);
	// }

	public void wakeLockAcquire() {
		wl.acquire();
		wakeLock = true;
	}

	public void wakeLockRelease() {
		wl.release();
		wakeLock = false;
	}

	public void reboot(String reason) {
		pm.reboot(reason);
	}

	// display info
	public int getRotation() {
		return display.getRotation();
		// return screenRotator.getRotation();
	}

	public int getDisplayId() {
		return display.getDisplayId();
	}

	public int getWidth() {
		return display.getWidth();
	}

	public int getHeight() {
		return display.getHeight();
	}

	public void getMetrics(DisplayMetrics outMetrics) {
		display.getMetrics(outMetrics);
	}

	public int getPixelFormat() {
		return display.getPixelFormat();
	}

	public float getRefreshRate() {
		return display.getRefreshRate();
	}

	public void setRotate0() {

		handler.post(rotate0);
	}

	public void setRotate90() {
		handler.post(rotate90);
	}

	public void setRotate180() {
		handler.post(rotate180);
	}

	public void setRotate270() {
		handler.post(rotate270);
	}

	public void setRotateRecovery() {
		handler.post(rotateRecovery);
	}

	public boolean isAutoRotate() {
		int autoRotate = android.provider.Settings.System.getInt(context.getContentResolver(),
				"accelerometer_rotation", 1);
		return autoRotate == 1 ? true : false;
	}

	// DisplayMetrics

	public float getDensity() {
		return metrics.density;
	}

	public int getDensityDpi() {
		return metrics.densityDpi;
	}

	public int getHeightPixels() {
		return metrics.heightPixels;
	}

	public float getScaledDensity() {
		return metrics.scaledDensity;
	}

	public int getWidthPixels() {
		return metrics.widthPixels;
	}

	public float getXdpi() {
		return metrics.xdpi;
	}

	public float getYdpi() {
		return metrics.ydpi;
	}

	public String getRCServiceVersionName() {
		String version = "error";
		try {
			PackageInfo i = context.getPackageManager().getPackageInfo(context.getPackageName(), 0);
			version = i.versionName;
		} catch (NameNotFoundException e) {
		}

		return version;
	}

	// battery temperature

	private BroadcastReceiver mBatInfoReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context arg0, Intent intent) {
			batteryTemperature = intent.getIntExtra(BatteryManager.EXTRA_TEMPERATURE, 0);

			int batteryScale = intent.getIntExtra(BatteryManager.EXTRA_SCALE, 100);
			batteryLevel = (int) (intent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0) * 100 / batteryScale);
		}
	};

	public int getBatteryTemperature() {
		return this.batteryTemperature;
	}

	public int getBatteryLevel() {
		return this.batteryLevel;
	}

	// system info

	public String getBuildBoard() {
		return Build.BOARD;
	}

	public String getBuildBootLoader() {
		return Build.BOOTLOADER;
	}

	public String getBuildBrand() {
		return Build.BRAND;
	}

	public String getBuildCPU_ABI() {
		return Build.CPU_ABI;
	}

	public String getBuildCPU_ABI2() {
		return Build.CPU_ABI2;
	}

	public String getBuildDevice() {
		return Build.DEVICE;
	}

	public String getBuildDisplay() {
		return Build.DISPLAY;
	}

	public String getBuildFingerPrint() {
		return Build.FINGERPRINT;
	}

	public String getBuildHardware() {
		return Build.HARDWARE;
	}

	public String getBuildHost() {
		return Build.HOST;
	}

	public String getBuildID() {
		return Build.ID;
	}

	public String getBuildManuFacturer() {
		return Build.MANUFACTURER;
	}

	public String getBuildModel() {
		return Build.MODEL;
	}

	public String getBuildProduct() {
		return Build.PRODUCT;
	}

	public String getBuildRadio() {
		return Build.RADIO;
	}

	public String getBuildSerial() {
		return Build.SERIAL;
	}

	public String getBuildTags() {
		return Build.TAGS;
	}

	public long getBuildTime() {
		return Build.TIME;
	}

	public String getBuildType() {
		return Build.TYPE;
	}

	public String getBuildUser() {
		return Build.USER;
	}

	public String getBuildVersionCodeName() {
		return Build.VERSION.CODENAME;
	}

	public String getBuildVersionIncremental() {
		return Build.VERSION.INCREMENTAL;
	}

	public String getBuildVersionRelease() {
		return Build.VERSION.RELEASE;
	}

	public String getBuildVersionSDKInt() {
		return "" + Build.VERSION.SDK_INT;
	}

	// revocery system
	public void installPackage(File packageFile) throws IOException {
		RecoverySystem.installPackage(this.context, packageFile);
	}

	public void rebootWipeCache() throws IOException {
		RecoverySystem.rebootWipeUserData(this.context);
	}

	public void rebootWipeUserData() throws IOException {
		RecoverySystem.rebootWipeUserData(this.context);
	}

	public void verifyPackage(File packageFile, RecoverySystem.ProgressListener listener, File deviceCertsZipFile)
			throws IOException, GeneralSecurityException {
		RecoverySystem.verifyPackage(packageFile, listener, deviceCertsZipFile);
	}

	public ArrayList<String> getInstalledPkgNames() {
		ArrayList<String> result = new ArrayList<String>();

		List<ApplicationInfo> applications = context.getPackageManager().getInstalledApplications(0);
		for (int n = 0; n < applications.size(); n++) {
			ApplicationInfo pkgInfo = applications.get(n);
			if ((pkgInfo.flags & ApplicationInfo.FLAG_SYSTEM) == 0
					&& exceptPkgNameArray.contains(pkgInfo.packageName) == false) {
				// This app is installed by user
				result.add(pkgInfo.packageName);
			}
		}

		return result;

	}
}
