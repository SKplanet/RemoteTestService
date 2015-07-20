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

public class TAState {
	public short m_devState;
	
	public static final short TDC_STATUS_ROTATE		= 0x0003;
	public static final short TDC_STATUS_SCREENON = 0x0004;
	public static final short TDC_STATUS_CALLBLOCKED = 0x0008;
	public static final short TDC_STATUS_PKGBLOCKED	= 0x0010;
	public static final short TDC_STATUS_SIMABSENT = 0x0020;
	public static final short TDC_STATUS_ROTATEAUTO = 0x0040;
	
	public TAState() {
		m_devState = 0;
	}
	
	public void setState(short state) {
		m_devState = state;
	}
	
	public short getState() {
		return m_devState;
	}
	
	public boolean isScreenOn() {
		return (m_devState & TDC_STATUS_SCREENON) != 0;
	}

	public int getScreenRotate() {
		return (int)(m_devState & TDC_STATUS_ROTATE);
	}
	
	public boolean isScreenRotateAuto() {
		return (m_devState & TDC_STATUS_ROTATEAUTO) != 0;
	}
	
	public boolean isCallBlocked() {
		return (m_devState & TDC_STATUS_CALLBLOCKED) != 0;
	}
	
	public boolean isPackageBlocked() {
		return (m_devState & TDC_STATUS_PKGBLOCKED) != 0;
	}
	
	public boolean isSIMAbsent() {
		return (m_devState & TDC_STATUS_SIMABSENT) != 0;
	}
}
