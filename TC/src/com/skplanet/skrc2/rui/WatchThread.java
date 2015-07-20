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



package com.skplanet.skrc2.rui;

import com.skplanet.skrc2.utils.Utils;

abstract public class WatchThread {
	public Thread wthread;
	public int timeoutmilsec;
	SocketAbortable abortable;
	
	public WatchThread(Thread th, SocketAbortable abt) {
		wthread = th;
		abortable = abt;
	}
	
	public void watch(int milsec) {

		timeoutmilsec = milsec;
		
		Runnable wt = new Runnable() {
			@Override
			public void run() {
				
				int nCount = timeoutmilsec / 100;
				if( timeoutmilsec > 0) {
					while( nCount-- > 0 && !abortable.isDone() && (wthread != null && wthread.isAlive())) {
						try {
							Thread.sleep(100);
						} catch (InterruptedException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
						Utils.LOG("%s", ".");
					}
				}
				else {
					try {
						wthread.join();
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
				onResult(( nCount <= 0 ), abortable.isDone(), wthread.isInterrupted());
			}
		};
		
		Thread wthread = new Thread(wt);
		wthread.start();
	}
	
	abstract public void onResult(boolean btimeout, boolean aborted, boolean interrupted);
}
