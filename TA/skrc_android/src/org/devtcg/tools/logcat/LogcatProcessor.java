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
package org.devtcg.tools.logcat;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import android.os.HandlerThread;

public abstract class LogcatProcessor extends HandlerThread {
	/* TODO: Support logcat filtering. */
	private static final int BUFFER_SIZE = 1024;

	private int mLines = 0;
	protected Process mLogcatProc = null;

	public LogcatProcessor(String name) {
		super(name);
	}

	public void run() {
		try {
			// 현재까지의 로그를 모두 지운다
			Runtime.getRuntime().exec("logcat -c");

			mLogcatProc = Runtime.getRuntime().exec("logcat -b system -s ActivityManager:I");
		} catch (IOException e) {
			return;
		}

		BufferedReader reader = null;

		try {
			reader = new BufferedReader(new InputStreamReader(mLogcatProc.getInputStream()), BUFFER_SIZE);
			String line;
			while ((line = reader.readLine()) != null) {

				onNewline(line);
				mLines++;

			}
		} catch (IOException e) {
			e.printStackTrace();
			onError("Error reading from logcat process", e);
		} finally {
			if (reader != null) {
				try {
					reader.close();
				} catch (IOException e) {
				}
			}
			stopCatter();
		}
	}

	public void stopCatter() {
		if (mLogcatProc == null)
			return;

		mLogcatProc.destroy();
		mLogcatProc = null;
	}

	public int getLineCount() {
		return mLines;
	}

	public abstract void onError(String msg, Throwable e);

	public abstract void onNewline(String line);
}// end of class
