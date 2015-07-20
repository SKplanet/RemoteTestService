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

import java.util.ArrayList;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.widget.Toast;

import com.rocode.skrc.RCService;

public class CallMonitorBroadcastReceiver extends BroadcastReceiver {

	protected static final String TAG = "RCService";

	private RCService service;

	public CallMonitorBroadcastReceiver(RCService service) {

		this.service = service;

	}

	@Override
	public void onReceive(Context context, Intent intent) {
		Bundle bundle = intent.getExtras();

		if (bundle == null) {
			return;
		}

		String phoneNumber = getResultData();

		if (phoneNumber == null) {
			// We could not find any previous data. Use the original phone
			// number in this case.
			phoneNumber = intent.getStringExtra(Intent.EXTRA_PHONE_NUMBER);
		}

		if (shouldCancel(phoneNumber)) {
			// Cancel our call.
			setResultData(null);
			Toast.makeText(context, "Call Canceled: " + phoneNumber, Toast.LENGTH_SHORT).show();

			service.addCallBlockLog(phoneNumber);
		}

	}

	/**
	 * callBlockArray로 시작하는 번호는 차단. 전화번호가 0 으로 시작하지 않으면 차단. 비상전화도 차단됨.
	 * 
	 * @param phoneNumber
	 * @return
	 */
	private boolean shouldCancel(String phoneNumber) {

		if (service.isCallBlock() == false) {
			return false;
		}

		boolean result = true;

		ArrayList<String> callAllowArray = service.getCallAllowArray();
		ArrayList<String> callBlockArray = service.getCallBlockArray();

		if (callBlockArray == null) {
			// block all, allow some number
			result = true;
			for (String blockNumber : callAllowArray) {
				if (phoneNumber.startsWith(blockNumber)) {
					result = false;
					break;
				}
			}
		} else if (callAllowArray == null) {
			// allow all, block some number
			result = false;
			for (String blockNumber : callBlockArray) {
				if (phoneNumber.startsWith(blockNumber)) {
					result = true;
					break;
				}
			}
		} else {
			// check allow first
			if (callAllowArray != null) {
				for (String allowNumber : callAllowArray) {
					if (phoneNumber.startsWith(allowNumber)) {
						result = false;
						break;
					}
				}
			}

			if (callBlockArray != null) {
				for (String blockNumber : callBlockArray) {
					if (phoneNumber.startsWith(blockNumber)) {
						result = true;
						break;
					}
				}
			}

		}

		return result;
	}

}
