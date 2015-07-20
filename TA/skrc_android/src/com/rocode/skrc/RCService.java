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
package com.rocode.skrc;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.List;
import java.util.StringTokenizer;

import org.devtcg.tools.logcat.LogcatProcessor;

import android.app.ActivityManager;
import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Handler;
import android.os.IBinder;

import com.rocode.log.Log;
import com.rocode.monitor.CallMonitorBroadcastReceiver;
import com.rocode.rotate.IRotate;
import com.rocode.rotate.ScreenRotator;
import com.rocode.socket.DeviceInfoServer;

public class RCService extends Service {

	protected static final String TAG = "RCService";

	private DeviceInfoServer server;
	private ScreenRotator screenRotator;

	private LogcatMonitor mLogcatMonitor;

	private CallMonitorBroadcastReceiver callMonitor;

	private ArrayList<String> callAllowArray = null;
	private ArrayList<String> callBlockArray = null;

	private ArrayList<String> pkgBlockArray = null;
	private ArrayList<String> mimetypeBlockArray = null;

	private Hashtable<String, Integer> callBlockLog = new Hashtable<String, Integer>();
	private Hashtable<String, Integer> pkgBlockLog = new Hashtable<String, Integer>();

	private Intent lockActivityIntent;

	private ActivityManager am;

	private boolean pkgBlock = false;
	private Handler pkgBlockHandler = new Handler();

	private Runnable pkgBlockRunnable = new Runnable() {
		public void run() {
			ComponentName topActivity = am.getRunningTasks(1).get(0).topActivity;

			synchronized (RCService.this) {
				if (pkgBlock == false) {
					return;
				}

				if (pkgBlockArray == null) {
					pkgBlockHandler.postDelayed(pkgBlockRunnable, postDelay);
					return;
				}

				for (String pkgBlock : pkgBlockArray) {
					if (topActivity.getPackageName().contains(pkgBlock)) {
						
						if (server == null || server.isClientConnected() == false) {
							continue;
						}

						Log.d("PKGBLOCK", "top activity name is: " + topActivity.getPackageName()
								+ "\npkg block string is:" + pkgBlock);

						startActivity(lockActivityIntent);
						addPkgBlockLog(pkgBlock);
						break;
					}
				}

				pkgBlockHandler.postDelayed(pkgBlockRunnable, postDelay);
			}

		}
	};

	public static final int DEFAULT_POST_DELAY = 3000;
	private int postDelay = DEFAULT_POST_DELAY;

	@Override
	public IBinder onBind(Intent arg0) {
		return mBinder;
	}

	private ArrayList<String> getArrayList(String str, String separator) {
		if (str == null) {
			return null;
		}

		ArrayList<String> list = new ArrayList<String>();

		StringTokenizer st = new StringTokenizer(str, separator);

		int n = st.countTokens();

		for (int i = 0; i < n; ++i) {
			String token = st.nextToken();

			list.add(token);
		}

		return list;
	}

	@Override
	public void onCreate() {
		// TODO Auto-generated method stub
		super.onCreate();

		am = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {

		Log.setLogEnable(false);

		startLogcat();

		lockActivityIntent = new Intent(this, LockActivity.class);
		lockActivityIntent.setAction(Intent.ACTION_MAIN);
		lockActivityIntent.addCategory(Intent.CATEGORY_LAUNCHER);
		// lockActivityIntent.setComponent(new ComponentName(packageName,
		// className));
		lockActivityIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP
				| Intent.FLAG_ACTIVITY_SINGLE_TOP);

		screenRotator = ScreenRotator.getScreenRotator(RCService.this);

		if (server != null) {
			server.stopServer();
		}
		server = new DeviceInfoServer(this);
		server.setDaemon(true);
		server.start();

		if (mLogcatMonitor != null) {
			mLogcatMonitor.stopCatter();
		}

		if (callMonitor != null) {
			try {
				unregisterReceiver(callMonitor);
			} catch (Exception ignore) {
			}
		}

		callMonitor = new CallMonitorBroadcastReceiver(this);
		IntentFilter intentFilter = new IntentFilter(Intent.ACTION_NEW_OUTGOING_CALL);
		intentFilter.setPriority(0);
		registerReceiver(callMonitor, intentFilter);

		return START_STICKY;
	}

	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();

		stopRCService();

	}

	public void stopRCService() {
		if (callMonitor != null) {
			unregisterReceiver(callMonitor);

		}
		callMonitor = null;

		if (server != null) {
			server.stopServer();
		}
		if (mLogcatMonitor != null) {
			mLogcatMonitor.stopCatter();
		}

		if (callAllowArray != null) {
			callAllowArray.clear();
		}

		if (callBlockArray != null) {
			callBlockArray.clear();
		}

		if (pkgBlockArray != null) {
			pkgBlockArray.clear();
		}

		if (mimetypeBlockArray != null) {
			mimetypeBlockArray.clear();
		}

		android.os.Process.killProcess(android.os.Process.myPid());
		System.exit(10);
	}

	/**
	 * call block 할 수 있나?
	 * @return
	 */
	public boolean isCallBlock() {
		
		if (server == null || server.isClientConnected() == false) {
			return false;
		}

		if (this.callAllowArray == null && this.callBlockArray == null) {
			return false;
		}

		return true;
	}

	public ArrayList<String> getCallAllowArray() {
		return this.callAllowArray;
	}

	public ArrayList<String> getCallBlockArray() {
		return this.callBlockArray;
	}

	public void setCallBlockList(String callBlocks) {
		if (callBlockArray != null) {
			callBlockArray.clear();
		}

		callBlockArray = getArrayList(callBlocks, ",");

	}

	public void setCallAllowList(String callAllows) {
		if (callAllowArray != null) {
			callAllowArray.clear();
		}

		callAllowArray = getArrayList(callAllows, ",");

	}

	public void setBlockPkgNameList(String activityBlocks) {
		synchronized (RCService.this) {
			if (pkgBlockArray != null) {
				pkgBlockArray.clear();
			}

			pkgBlockArray = getArrayList(activityBlocks, ",");
			
			addMimetypePKGBlock();
		}
	}

	public void addCallBlockLog(String phoneNumber) {
		Integer blockCount = 1;
		if (this.callBlockLog.containsKey(phoneNumber)) {
			blockCount = this.callBlockLog.remove(phoneNumber) + 1;
		}

		this.callBlockLog.put(phoneNumber, blockCount);

	}

	public void addPkgBlockLog(String pkgName) {
		Integer blockCount = 1;
		if (this.pkgBlockLog.containsKey(pkgName)) {
			blockCount = this.pkgBlockLog.remove(pkgName) + 1;
		}

		this.pkgBlockLog.put(pkgName, blockCount);

	}

	public void setBlockMimetypeList(String mimetypeBlocks) {
		synchronized (RCService.this) {
			if (mimetypeBlockArray != null) {
				// pkgBlockArray 에 들어있는 mimetypeBlockArray를 제거 해야 함.
				removeMimetypePKGBlock();
				
				mimetypeBlockArray.clear();
			}

			mimetypeBlockArray = getArrayList(mimetypeBlocks, ",");

			addMimetypePKGBlock();
		}

	}

	// Intent.ACTION_VIEW와 mimetype 에 맞는 패키지 이름을 얻어 패키지 블럭에 추가하는 함수. 
	private void addMimetypePKGBlock() {
		if (mimetypeBlockArray == null) {
			return;
		}
		
		if (pkgBlockArray == null) {
			pkgBlockArray = new ArrayList<String>();
		}
		
		PackageManager manager = getPackageManager();
		for (String mimetype : mimetypeBlockArray) {
			Intent intent = new Intent(Intent.ACTION_VIEW);
			intent.setType(mimetype);

			List<ResolveInfo> resolveInfos = manager.queryIntentActivities(intent, 0);

			for (int i = 0, iend = resolveInfos.size(); i < iend; i++) {

				ResolveInfo ri = resolveInfos.get(i);

				String pkgName = ri.activityInfo.applicationInfo.packageName; 
				
				pkgBlockArray.add(pkgName);
			}

		}
	}
	
	private void removeMimetypePKGBlock() {
		if (mimetypeBlockArray == null) {
			return;
		}
		
		if (pkgBlockArray == null) {
			return;
		}
		
		PackageManager manager = getPackageManager();
		for (String mimetype : mimetypeBlockArray) {
			Intent intent = new Intent(Intent.ACTION_VIEW);
			intent.setType(mimetype);

			List<ResolveInfo> resolveInfos = manager.queryIntentActivities(intent, 0);

			for (int i = 0, iend = resolveInfos.size(); i < iend; i++) {

				ResolveInfo ri = resolveInfos.get(i);

				String pkgName = ri.activityInfo.applicationInfo.packageName; 
				
				pkgBlockArray.remove(pkgName);
			}

		}
	}


	public byte[] getLog() {
		StringBuffer buffer = new StringBuffer();

		Enumeration<String> en = this.callBlockLog.keys();

		boolean isFirstElement = true;
		while (en.hasMoreElements()) {
			String key = en.nextElement();
			if (isFirstElement) {
				buffer.append("CB:");
				isFirstElement = false;
			} else {
				buffer.append(",CB:");
			}
			buffer.append(key);
			buffer.append(" is blocked ");
			buffer.append(this.callBlockLog.get(key));
			buffer.append(" times");
		}

		this.callBlockLog.clear();

		en = this.pkgBlockLog.keys();

		while (en.hasMoreElements()) {
			String key = en.nextElement();
			if (isFirstElement) {
				buffer.append("PB:");
				isFirstElement = false;
			} else {
				buffer.append(",PB:");
			}
			buffer.append(key);
			buffer.append(" is blocked ");
			buffer.append(this.pkgBlockLog.get(key));
			buffer.append(" times");
		}

		this.pkgBlockLog.clear();

		return buffer.toString().getBytes();
	}

	public boolean isCallBlocked() {
		return this.callBlockLog.size() > 0 ? true : false;
	}

	public boolean isPkgBlocked() {
		return this.pkgBlockLog.size() > 0 ? true : false;
	}

	public DeviceInfoServer getServer() {
		return this.server;
	}

	public void startLogcat() {
		String pkgName = getApplicationContext().getPackageName();
		String myUserId = null;
		ArrayList<String> logcatArray = new ArrayList<String>();

		BufferedReader reader = null;
		try {
			Process process = Runtime.getRuntime().exec("ps -x");

			reader = new BufferedReader(new InputStreamReader(process.getInputStream()), 1024);
			String line;
			while ((line = reader.readLine()) != null) {
				if (line.indexOf(pkgName) >= 0) {
					StringTokenizer st = new StringTokenizer(line, " ");
					myUserId = st.nextToken();
				} else if (line.contains("logcat")) {
					logcatArray.add(line);
				}
			}
		} catch (IOException e) {
		} finally {
			if (reader != null) {
				try {
					reader.close();
				} catch (IOException e) {
				}
			}
		}

		for (String logcatPs : logcatArray) {
			StringTokenizer st = new StringTokenizer(logcatPs, " ");
			String userId = st.nextToken();
			if (myUserId == null || !myUserId.equals(userId)) {
				continue;
			}
			String pid = st.nextToken();

			try {
				Runtime.getRuntime().exec("kill -9 " + pid);
			} catch (IOException e) {
			}
		}

		if (mLogcatMonitor != null) {
			mLogcatMonitor.stopCatter();
		}
		mLogcatMonitor = null;

		mLogcatMonitor = new LogcatMonitor("RCService");

		mLogcatMonitor.start();
	}

	public void stopLogcat() {
		if (mLogcatMonitor != null) {
			mLogcatMonitor.stopCatter();
		}

	}

	public void startPkgBlock() {
		pkgBlock = true;
		pkgBlockHandler.post(pkgBlockRunnable);
	}

	public void stopPkbBlock() {
		pkgBlock = false;
	}

	public void setPkgBlockPostDelay(int delay) {
		postDelay = delay;
	}

	private final IRotate.Stub mBinder = new IRotate.Stub() {

		public void rotate0() {
			screenRotator.rotate0();
		}

		public void rotate90() {
			screenRotator.rotate90();
		}

		public void rotate180() {
			screenRotator.rotate180();
		}

		public void rotate270() {
			screenRotator.rotate270();
		}

		public void rotateRecovery() {
			screenRotator.rotateRecovery();
		}
	};

	class LogcatMonitor extends LogcatProcessor {

		public LogcatMonitor(String name) {
			super(name);
		}

		@Override
		public void onError(String msg, Throwable e) {
			// stop service
			stopRCService();

		}

		@Override
		public void onNewline(String line) {
			ComponentName topActivity = am.getRunningTasks(1).get(0).topActivity;

			synchronized (RCService.this) {
				if (pkgBlockArray == null) {
					return;
				}

				for (String pkgBlock : pkgBlockArray) {
					if (topActivity.getPackageName().contains(pkgBlock)) {
						
						if (server == null || server.isClientConnected() == false) {
							continue;
						}

						Log.d("PKGBLOCK", "top activity name is: " + topActivity.getPackageName()
								+ "\npkg block string is:" + pkgBlock);

						startActivity(lockActivityIntent);
						addPkgBlockLog(pkgBlock);
						return;
					}
				}
			}
		}

	}

}
