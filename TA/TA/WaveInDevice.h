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


#ifndef _WAVEINDEVICE_H
#define _WAVEINDEVICE_H

class CWaveInDevice
{
public:
	CWaveInDevice();
	virtual ~CWaveInDevice();

public:
	CWaveInDevice*		pPrev;
	CWaveInDevice*		pNext;

private:
	UINT				m_nDeviceID;
	CString				m_strDeviceName;

public:
	UINT				GetDeviceID()                       { return m_nDeviceID;               }
	void				SetDeviceID(UINT nDeviceID)         { m_nDeviceID = nDeviceID;          }
	LPCTSTR				GetDeviceName()                     { return (LPCTSTR) m_strDeviceName; }
	void				SetDeviceName(LPCTSTR szDeviceName) { m_strDeviceName = szDeviceName;   }
};

#endif
