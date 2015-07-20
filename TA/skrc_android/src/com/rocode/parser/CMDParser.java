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

package com.rocode.parser;

import java.io.IOException;
import java.util.ArrayList;

import android.view.Surface;

import com.rocode.info.DeviceControl;
import com.rocode.log.Log;
import com.rocode.rotate.ScreenRotator;
import com.rocode.skrc.RCService;
import com.rocode.socket.DeviceInfoServer;
import com.rocode.util.Utils;

public class CMDParser {

	public static final int SEND_MULTIPLE_PACKET = -1;

	public static final int IS_SCREEN_ON = 0;
	public static final int WAKE_LOCK_ACQUIRE = 1;
	public static final int WAKE_LOCK_RELEASE = 2;

	public static final int GET_ROTATION = 100;
	public static final int GET_WIDTH = 101;
	public static final int GET_HEIGHT = 102;
	public static final int GET_PIXELFORMAT = 103;
	public static final int SET_ROTATION_0 = 104;
	public static final int SET_ROTATION_90 = 105;
	public static final int SET_ROTATION_180 = 106;
	public static final int SET_ROTATION_270 = 107;
	public static final int SET_ROTATION_RECOVERY = 108;
	public static final int GET_DENSITY_DPI = 109;
	public static final int GET_RC_VERSION = 110;

	public static final int GET_BATTERY_TEMPERATURE = 200;
	public static final int GET_BATTERY_LEVEL = 201;

	public static final int GET_BUILD_BOARD = 300;
	public static final int GET_BUILD_BOOTLOADER = 301;
	public static final int GET_BUILD_BRAND = 302;
	public static final int GET_BUILD_CPU_ABI = 303;
	public static final int GET_BUILD_CPU_ABI2 = 304;
	public static final int GET_BUILD_DEVICE = 305;
	public static final int GET_BUILD_DISPLAY = 306;
	public static final int GET_BUILD_FINGERPRINT = 307;
	public static final int GET_BUILD_HARDWARE = 308;
	public static final int GET_BUILD_HOST = 309;
	public static final int GET_BUILD_ID = 310;
	public static final int GET_BUILD_MANUFACTURER = 311;
	public static final int GET_BUILD_MODEL = 312;
	public static final int GET_BUILD_PRODUCT = 313;
	public static final int GET_BUILD_RADIO = 314;
	public static final int GET_BUILD_SERIAL = 315;
	public static final int GET_BUILD_TAGS = 316;
	public static final int GET_BUILD_TIME = 317;
	public static final int GET_BUILD_TYPE = 318;
	public static final int GET_BUILD_USER = 319;

	public static final int GET_BUILD_VERSION_CODENAME = 320;
	public static final int GET_BUILD_VERSION_INCREMENTAL = 321;
	public static final int GET_BUILD_VERSION_RELEASE = 322;
	public static final int GET_BUILD_VERSION_SDK_INT = 323;

	public static final int SET_BLOCKED_PKGNAME_LIST = 400;
	public static final int SET_CALL_BLOCK_LIST = 401;
	public static final int SET_CALL_ALLOW_LIST = 402;
	public static final int SET_BLOCKED_MIMETYPE_LIST = 403;

	public static final int SET_LOG_ENABLE = 500;
	public static final int GET_STATUS = 501;

	public static final int GET_INSTALLED_PKG_NAME = 600;

	public static final int GET_LINE1_NUMBER = 700;
	public static final int GET_NETWORK_OPERATOR = 701;
	public static final int GET_NETWORK_OPERATOR_NAME = 702;
	public static final int IS_SIM_STATE_ABSENT = 703;
	public static final int IS_SIM_STATE_NETWORK_LOCKED = 704;
	public static final int IS_SIM_STATE_PIN_REQUIRED = 705;
	public static final int IS_SIM_STATE_PUK_REQUIRED = 706;
	public static final int IS_SIM_STATE_READY = 707;
	public static final int IS_SIM_STATE_UNKNOWN = 708;
	public static final int GET_DEVICE_ID = 709;
	public static final int GET_DEVICE_SOFTWARE_VERSION = 710;
	public static final int GET_SIM_SERIAL_NUMBER = 711;

	public static final int LOGCAT_START = 800;
	public static final int LOGCAT_STOP = 801;
	public static final int PKG_BLOCK_HANDLER_START = 802;    
	public static final int PKG_BLOCK_HANDLER_STOP = 803;
	public static final int SET_PKG_BLOCK_HANDLER_DELAY = 804;

	public static final int DELETE_ALL = 900;
	public static final int DELETE_SMS = 901;
	public static final int DELETE_CALL = 902;
	public static final int DELETE_MMS = 903;

	public static final int INVALID_COMMAND = 1000;

	/**
	 * 전달받은 커맨드를 파싱하여 응답 패킷을 생성한다. 리턴값을 생성한 패킷 길이이다. 리턴값이 SEND_MULTIPLE_PACKET
	 * 이면 여러번 전송해야 하는 커맨드이므로 다른 펑션에서 처리한다.
	 * 
	 * @param service
	 * @param control
	 * @param cmdNo
	 * @param index
	 * @param body
	 * @param result
	 * @return
	 * @throws IOException
	 */
	public static int execute(RCService service, DeviceControl control, int cmdNo, int index, String body, byte[] result)
			throws IOException {
		int size;

		// calling result (on response)
		int resultValue = 1;

		int rotation = 0;

		byte[] byteValue = null;

		switch (cmdNo) {
		case IS_SCREEN_ON:
			if (control.isScreenOn() == false) {
				resultValue = 0;
			}
			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case WAKE_LOCK_ACQUIRE:
			control.wakeLockAcquire();

			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case WAKE_LOCK_RELEASE:
			control.wakeLockRelease();

			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case GET_ROTATION:

			rotation = control.getRotation();
			switch (rotation) {
			case ScreenRotator.SCREEN_ORIENTATION_PORTRAIT:
				resultValue = 0;
				break;

			case ScreenRotator.SCREEN_ORIENTATION_LANDSCAPE:
				resultValue = 1;
				break;

			case ScreenRotator.SCREEN_ORIENTATION_REVERSE_PORTRAIT:
				resultValue = 2;
				break;

			case ScreenRotator.SCREEN_ORIENTATION_REVERSE_LANDSCAPE:
				resultValue = 3;
				break;

			default:
				resultValue = 0;
				break;
			}
			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case GET_WIDTH:
			byteValue = ("" + control.getWidth()).getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case GET_HEIGHT:
			byteValue = ("" + control.getHeight()).getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case GET_PIXELFORMAT:
			byteValue = ("" + control.getPixelFormat()).getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case SET_ROTATION_0:
			control.setRotate0();

			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case SET_ROTATION_90:
			control.setRotate90();

			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case SET_ROTATION_180:
			control.setRotate180();

			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case SET_ROTATION_270:
			control.setRotate270();

			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case SET_ROTATION_RECOVERY:
			control.setRotateRecovery();

			size = DeviceInfoServer.HEADER_SIZE;
			break;

		case GET_DENSITY_DPI:
			byteValue = ("" + control.getDensityDpi()).getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
			
		case GET_RC_VERSION:
			byteValue = control.getRCServiceVersionName().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case GET_BATTERY_TEMPERATURE:
			resultValue = control.getBatteryTemperature();

			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case GET_BATTERY_LEVEL:
			resultValue = control.getBatteryLevel();

			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case GET_BUILD_BOARD:

			byteValue = control.getBuildBoard().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case GET_BUILD_BOOTLOADER:
			byteValue = control.getBuildBootLoader().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_BRAND:
			byteValue = control.getBuildBrand().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_CPU_ABI:
			byteValue = control.getBuildCPU_ABI().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_CPU_ABI2:
			byteValue = control.getBuildCPU_ABI2().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_DEVICE:
			byteValue = control.getBuildDevice().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_DISPLAY:
			byteValue = control.getBuildDisplay().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_FINGERPRINT:
			byteValue = control.getBuildFingerPrint().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_HARDWARE:
			byteValue = control.getBuildHardware().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_HOST:
			byteValue = control.getBuildHost().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_ID:
			byteValue = control.getBuildID().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_MANUFACTURER:
			byteValue = control.getBuildManuFacturer().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_MODEL:
			byteValue = control.getBuildModel().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_PRODUCT:
			byteValue = control.getBuildProduct().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_RADIO:
			byteValue = control.getBuildRadio().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_SERIAL:
			byteValue = control.getBuildSerial().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_TAGS:
			byteValue = control.getBuildTags().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_TIME:
			byteValue = ("" + control.getBuildTime()).getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_TYPE:
			byteValue = control.getBuildType().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;
		case GET_BUILD_USER:
			byteValue = control.getBuildUser().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case GET_BUILD_VERSION_CODENAME:
			byteValue = control.getBuildVersionCodeName().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case GET_BUILD_VERSION_INCREMENTAL:
			byteValue = control.getBuildVersionIncremental().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case GET_BUILD_VERSION_RELEASE:
			byteValue = control.getBuildVersionRelease().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case GET_BUILD_VERSION_SDK_INT:
			byteValue = control.getBuildVersionSDKInt().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case SET_BLOCKED_PKGNAME_LIST:
			size = DeviceInfoServer.HEADER_SIZE;

			service.setBlockPkgNameList(body);

			if (body == null) {
				resultValue = 0;
			}

			break;

		case SET_CALL_BLOCK_LIST:
			size = DeviceInfoServer.HEADER_SIZE;

			service.setCallBlockList(body);

			if (body == null) {
				resultValue = 0;
			}

			break;

		case SET_CALL_ALLOW_LIST:
			size = DeviceInfoServer.HEADER_SIZE;

			service.setCallAllowList(body);

			if (body == null) {
				resultValue = 0;
			}

			break;
			
		case SET_BLOCKED_MIMETYPE_LIST:
			size = DeviceInfoServer.HEADER_SIZE;

			service.setBlockMimetypeList(body);

			if (body == null) {
				resultValue = 0;
			}

			break;

		case SET_LOG_ENABLE:
			size = DeviceInfoServer.HEADER_SIZE;

			if (body == null) {
				Log.setLogEnable(false);
				resultValue = 0;
			} else {
				Log.setLogEnable(true);
				resultValue = 1;
			}

			break;

		case GET_STATUS:
			boolean isCallBlocked = service.isCallBlocked();
			boolean isPkgBlocked = service.isPkgBlocked();

			// log cleared
			byteValue = service.getLog();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			// verbose msg 크기가 길어 전체 패킷 사이즈가 MAX_PACKET_SIZE byte를 넘으면 제한한다.
			if (size > DeviceInfoServer.MAX_PACKET_SIZE) {
				size = DeviceInfoServer.MAX_PACKET_SIZE;
			}

			result[0] = (byte) (size >>> 8);
			result[1] = (byte) (size);
			result[2] = (byte) (cmdNo >>> 8);
			result[3] = (byte) (cmdNo);
			result[4] = (byte) (index >>> 8);
			result[5] = (byte) (index);

			result[6] = 0;
			result[7] = 0;

			rotation = control.getRotation();
			switch (rotation) {
			case Surface.ROTATION_0:
				result[7] = Surface.ROTATION_0;
				break;

			case Surface.ROTATION_90:
				result[7] = Surface.ROTATION_90;
				break;

			case Surface.ROTATION_180:
				result[7] = Surface.ROTATION_180;
				break;

			case Surface.ROTATION_270:
				result[7] = Surface.ROTATION_270;
				break;

			default:
				result[7] = 0x0;
				break;
			}

			if (control.isScreenOn()) {
				result[7] |= 0x4;
			}

			if (isCallBlocked) {
				result[7] |= 0x8;
			}

			if (isPkgBlocked) {
				result[7] |= 0x10;
			}

			if (control.isSimStateAbsent()) {
				result[7] |= 0x20;
			}

			if (control.isAutoRotate()) {
				result[7] |= 0x40;
			}

			for (int i = DeviceInfoServer.HEADER_SIZE; i < size; ++i) {
				result[i] = byteValue[i - DeviceInfoServer.HEADER_SIZE];
			}

			return size;

		case GET_INSTALLED_PKG_NAME:

			ArrayList<String> pkgNames = control.getInstalledPkgNames();

			StringBuffer buffer = new StringBuffer();
			int pkgNameCount = pkgNames.size();
			int bufferCount = 0;

			for (int i = 0; i < pkgNameCount; ++i) {

				buffer.append(pkgNames.get(i));

				bufferCount++;
				if (i > 0 && i % 10 == 0) {
					service.getServer().sendMsg(cmdNo, index, bufferCount, buffer.toString());
					bufferCount = 0;
					buffer.setLength(0);
				}
				if (bufferCount > 0) {
					buffer.append(",");
				}
			}

			if (bufferCount > 0) {
				buffer.deleteCharAt(buffer.length() - 1);
				service.getServer().sendMsg(cmdNo, index, bufferCount, buffer.toString());
			}

			return SEND_MULTIPLE_PACKET;

		case GET_LINE1_NUMBER:
			byteValue = control.getLine1Number().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case GET_SIM_SERIAL_NUMBER:
			byteValue = control.getSimSerialNumber().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case GET_NETWORK_OPERATOR:
			byteValue = control.getNetworkOperator().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case GET_NETWORK_OPERATOR_NAME:
			byteValue = control.getNetworkOperatorName().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case IS_SIM_STATE_ABSENT:
			size = DeviceInfoServer.HEADER_SIZE;

			if (control.isSimStateAbsent() == false) {
				resultValue = 0;
			}

			break;

		case IS_SIM_STATE_NETWORK_LOCKED:
			size = DeviceInfoServer.HEADER_SIZE;

			if (control.isSimStateAbsent() == false) {
				resultValue = 0;
			}

			break;

		case IS_SIM_STATE_PIN_REQUIRED:
			size = DeviceInfoServer.HEADER_SIZE;

			if (control.isSimStatePinRequired() == false) {
				resultValue = 0;
			}

			break;

		case IS_SIM_STATE_PUK_REQUIRED:
			size = DeviceInfoServer.HEADER_SIZE;

			if (control.isSimStatePUKRequired() == false) {
				resultValue = 0;
			}

			break;

		case IS_SIM_STATE_READY:
			size = DeviceInfoServer.HEADER_SIZE;

			if (control.isSimStateReady() == false) {
				resultValue = 0;
			}

			break;

		case IS_SIM_STATE_UNKNOWN:
			size = DeviceInfoServer.HEADER_SIZE;

			if (control.isSimStateUnknown() == false) {
				resultValue = 0;
			}

			break;

		case GET_DEVICE_ID:
			byteValue = control.getDeviceId().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case GET_DEVICE_SOFTWARE_VERSION:
			byteValue = control.getDeviceSoftwareVersion().getBytes();

			size = DeviceInfoServer.HEADER_SIZE + byteValue.length;

			break;

		case LOGCAT_START:
			service.startLogcat();

			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case LOGCAT_STOP:
			service.stopLogcat();

			size = DeviceInfoServer.HEADER_SIZE;

			break;
			
		case PKG_BLOCK_HANDLER_START:
			service.startPkgBlock();

			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case PKG_BLOCK_HANDLER_STOP:
			service.stopPkbBlock();

			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case SET_PKG_BLOCK_HANDLER_DELAY:
			if (body == null) {
				service.setPkgBlockPostDelay(RCService.DEFAULT_POST_DELAY);
			} else {
				int value = RCService.DEFAULT_POST_DELAY;
				try {
					value = Integer.parseInt(body);
				} catch (Exception e) {
					value = RCService.DEFAULT_POST_DELAY;
				}
				service.setPkgBlockPostDelay(value);
			}

		case DELETE_ALL:
			Utils.smsDelete(service.getBaseContext());
			Utils.callDelete(service.getBaseContext());
			Utils.mmsDelete(service.getBaseContext());

			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case DELETE_SMS:
			Utils.smsDelete(service.getBaseContext());

			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case DELETE_CALL:
			Utils.callDelete(service.getBaseContext());

			size = DeviceInfoServer.HEADER_SIZE;

			break;

		case DELETE_MMS:
			Utils.mmsDelete(service.getBaseContext());

			size = DeviceInfoServer.HEADER_SIZE;

			break;
		default:
			cmdNo = INVALID_COMMAND;

			size = DeviceInfoServer.HEADER_SIZE;

			break;
		}

		result[0] = (byte) (size >>> 8);
		result[1] = (byte) (size);
		result[2] = (byte) (cmdNo >>> 8);
		result[3] = (byte) (cmdNo);
		result[4] = (byte) (index >>> 8);
		result[5] = (byte) (index);
		result[6] = (byte) (resultValue >>> 8);
		result[7] = (byte) (resultValue);

		for (int i = DeviceInfoServer.HEADER_SIZE; i < size; ++i) {
			result[i] = byteValue[i - DeviceInfoServer.HEADER_SIZE];
		}

		return size;
	}

}
