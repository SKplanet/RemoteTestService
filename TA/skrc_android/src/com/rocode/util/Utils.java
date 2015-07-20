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

package com.rocode.util;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.ResolveInfo;
import android.database.Cursor;
import android.net.Uri;
import android.provider.CallLog;

import com.rocode.log.Log;

public class Utils {

	public static boolean isSystemPackage(PackageInfo pkgInfo) {
		return ((pkgInfo.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0) ? true : false;
	}

	public static boolean isSystemPackage(ResolveInfo ri) {
		return ((ri.activityInfo.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) != 0) ? true : false;

	}

	public static void smsDelete(Context context) {
		Uri deleteUri = Uri.parse("content://sms");
		int count = 0;
		Cursor c = context.getContentResolver().query(deleteUri, null, null, null, null);
		while (c.moveToNext()) {
			try {
				// Delete the SMS
				String pid = c.getString(0);
				// Get id;
				String uri = "content://sms/" + pid;
				count = context.getContentResolver().delete(Uri.parse(uri), null, null);
				Log.d("SMS", "sms count:" + count);

			} catch (Exception e) {
			}
		}
	}
	
	public static void mmsDelete(Context context) {
		Uri deleteUri = Uri.parse("content://mms");
		int count = 0;
		Cursor c = context.getContentResolver().query(deleteUri, null, null, null, null);
		while (c.moveToNext()) {
			try {
				// Delete the SMS
				String pid = c.getString(0);
				// Get id;
				String uri = "content://mms/" + pid;
				count = context.getContentResolver().delete(Uri.parse(uri), null, null);
				Log.d("MMS", "mms count:" + count);

			} catch (Exception e) {
			}
		}
	}

	// Call Log 지우기

	public static void callDelete(Context context) {
		int result = 0;
		Cursor cursor = context.getContentResolver().query(CallLog.Calls.CONTENT_URI, null, null, null, null);

		while (cursor.moveToNext()) {
			String _ID = cursor.getString(cursor.getColumnIndex(CallLog.Calls._ID));

			result = context.getContentResolver().delete(CallLog.Calls.CONTENT_URI, CallLog.Calls._ID + " =" + _ID, null);
		}
		
		result = context.getContentResolver().delete(CallLog.Calls.CONTENT_URI, "type=1", null);
		result = context.getContentResolver().delete(CallLog.Calls.CONTENT_URI, "type=2", null);
		result = context.getContentResolver().delete(CallLog.Calls.CONTENT_URI, "type=3", null);
		Log.d("CALL:", "result:" + result);
	}
}
