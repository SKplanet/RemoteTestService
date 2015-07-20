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



#include "stdafx.h"
#include "rui.h"
#include "keycodes.h"

void RA::key_checkok()
{
	if( m_monkeysock == INVALID_SOCKET )
		return;

	if( m_bCheckEventReturn == 0 )
		return;


	char buf[4];

	int loop = 0;
	int n;
	while(++loop < 6)
	{
		memset(buf,0,4);
		n = ReadExactTimeout(m_monkeysock, buf, 3, 30 * loop);
		//TRACE("EVT :%d:%d:(%s)\n", loop, n, buf);
		if( buf[0] == 'o' || buf[0] == 'O' )
		{
			break;
		}
	}

	if( loop > 5 )
	{
		restart_evt();
	}

}

void RA::mouse_event(int x, int y, int mode) // 0 - up , 1 - down, 2 - move
{
	//ta_log("TDC mouse_event(%d, %d, %d)", x, y, mode);
	static DWORD tl = 0;
	static int lastmode = 0;
	static int lx = 0;
	static int ly = 0;

	char buf[1024];
	char smode[10];
	
	if( m_monkeysock == INVALID_SOCKET )
		return;

	if( this->m_fbcTouch ) // raw level touch injection
	{
		int rotate = m_devStatus & TDC_STATUS_ROTATE;
	
		//TRACE("(%d) %d-%d : ", rotate, x, y);
		int temp;
		if( rotate == 1 ) // 90
		{
			temp = x;
			x = m_fbcWidth - y;
			y = temp;
		}
		else if ( rotate == 2 ) // 180
		{
			x = m_fbcWidth - x;
			y = m_fbcHeight - y;
		}
		else if (rotate == 3 ) // 270
		{
			temp = x;
			x = y;
			y = m_fbcHeight - temp;
		}
		//TRACE("%d-%d\n", x, y);
	}

	if( mode == 1 )
		lstrcpy(smode,"down");
	else if ( mode == 2 )
		lstrcpy(smode, "move");
	else
		lstrcpy(smode,"up");

	DWORD t = clock();
	int wait = 100;
	if( m_fbcTouch )
		wait = 50;


	if( (t - tl) < (DWORD)wait )
	{
		//if( lastmode == 1 && mode == 0 )	// down and up --> tap
		//{
		//	sprintf_s(buf, "tap %d %d\n", lx, ly);
		//	WriteExact(m_monkeysock, buf, strlen(buf));
		//	TRACE(buf);
		//	lastmode = mode;
		//	tl = t;
		//	lx = x; ly = y;
		//	return;
		//}
		//if( lastmode == 1 && mode == 2 ) { // down and move --> first down then move
		//	sprintf_s(buf, "touch down %d %d\n", lx, ly);
		//	WriteExact(m_monkeysock, buf, strlen(buf));
		//	TRACE("1and 2 so first touch down\n");
		//}
		//else 
		if ( lastmode == 2 && mode == 2 ) // fast move and move.. --> skip
		{
			lx = x; ly = y;
			//TRACE("skip moving\n");
			return;
		}
		//Sleep(100);
	}
	//else {
	//	if ( lastmode == 1 && mode == 0) // down and long... up --> down and up
	//	{
	//		sprintf_s(buf, "touch down %d %d\n", lx, ly);
	//		WriteExact(m_monkeysock, buf, strlen(buf));
	//		TRACE(buf);
	//	}
	//	else if ( lastmode == 0 && mode == 1) // first click --> keep it
	//	{
	//		lastmode = mode;
	//		tl = t;
	//		lx = x; ly = y;
	//		TRACE("skip first click\n");
	//		return;
	//	}
	//}

	if( m_fbcTouch ) // use low level event
	{
		raw_touch(mode,x,y);
	}
	else // use monkey
	{
		sprintf_s(buf, "touch %s %d %d\n", smode, x, y);
		WriteExact(m_monkeysock, buf, strlen(buf));
	}
	//TRACE(buf);
	lastmode = mode;
	tl = t;
	lx = x; ly = y;

}

void RA::key_event(int key, int mode)
{
	char buf[1024];
	
	if( m_monkeysock == INVALID_SOCKET )
		return;

	if( mode == 0 )
		sprintf_s(buf, "key up %d\n", key);
	else if ( mode == 1 )
		sprintf_s(buf, "key down %d\n", key);
	else
		sprintf_s(buf, "key down %d\nkey up %d\n", key, key);

	WriteExact(m_monkeysock, buf, strlen(buf));
	key_checkok();
}

void RA::key_home(bool bDown)
{
#ifdef USE_UINPUT
	raw_key(bDown, 102);
	return;
#endif
	//ta_log("TDC KEYCODE_HOME(%d)", bDown);
	if( m_monkeysock == INVALID_SOCKET )
		return;
	char buf[1024];

	//sprintf_s(buf, "press %d\n", AKEYCODE_HOME);
	if( bDown )
		sprintf_s(buf, "key down %d\n", AKEYCODE_HOME);
	else
		sprintf_s(buf, "key up %d\n", AKEYCODE_HOME);
	WriteExact(m_monkeysock, buf, strlen(buf));
	key_checkok();
}

void RA::key_back(bool bDown)
{
#ifdef USE_UINPUT
	raw_key(bDown, 158);
	return;
#endif
	//ta_log("TDC KEYCODE_BACK");
	if( m_monkeysock == INVALID_SOCKET )
		return;
	char buf[1024];

	//sprintf_s(buf, "press %d\n", AKEYCODE_BACK);
	if( bDown )
		sprintf_s(buf, "key down %d\n", AKEYCODE_BACK);
	else
		sprintf_s(buf, "key up %d\n", AKEYCODE_BACK);
	WriteExact(m_monkeysock, buf, strlen(buf));
	key_checkok();
}

void RA::key_menu(bool bDown)
{
#ifdef USE_UINPUT
	raw_key(bDown, 139);
	return;
#endif
	//tdc_getinstalledpkgname();

	//ta_log("TDC KEYCODE_MENU");
	if( m_monkeysock == INVALID_SOCKET )
		return;
	char buf[1024];

	//sprintf_s(buf, "press %d\n", AKEYCODE_MENU);
	if( bDown )
		sprintf_s(buf, "key down %d\n", AKEYCODE_MENU);
	else
		sprintf_s(buf, "key up %d\n", AKEYCODE_MENU);
	WriteExact(m_monkeysock, buf, strlen(buf));
	key_checkok();
}

void RA::key_search(bool bDown)
{
#ifdef USE_UINPUT
	raw_key(bDown, 217);
	return;
#endif
	//ta_log("TDC KEYCODE_SEARCH");
	if( m_monkeysock == INVALID_SOCKET )
		return;
	char buf[1024];

	//sprintf_s(buf, "press %d\n", AKEYCODE_SEARCH);
	if( bDown )
		sprintf_s(buf, "key down %d\n", AKEYCODE_SEARCH);
	else
		sprintf_s(buf, "key up %d\n", AKEYCODE_SEARCH);
	WriteExact(m_monkeysock, buf, strlen(buf));
	key_checkok();
}

void RA::key_volup(bool bDown)
{
#ifdef USE_UINPUT
	raw_key(bDown, 115);
	return;
#endif

	//ta_log("TDC VOLUME_UP");
	if( m_monkeysock == INVALID_SOCKET )
		return;
	char buf[1024];

	//sprintf_s(buf, "press %d\n", AKEYCODE_VOLUME_UP);
	if( bDown )
		sprintf_s(buf, "key down %d\n", AKEYCODE_VOLUME_UP);
	else
		sprintf_s(buf, "key up %d\n", AKEYCODE_VOLUME_UP);
	WriteExact(m_monkeysock, buf, strlen(buf));
	key_checkok();
}

void RA::key_voldown(bool bDown)
{
#ifdef USE_UINPUT
	raw_key(bDown, 114);
	return;
#endif

	//ta_log("TDC VOLUME_DOWN");
	if( m_monkeysock == INVALID_SOCKET )
		return;
	char buf[1024];

	//sprintf_s(buf, "press %d\n", AKEYCODE_VOLUME_DOWN);
	if( bDown )
		sprintf_s(buf, "key down %d\n", AKEYCODE_VOLUME_DOWN);
	else
		sprintf_s(buf, "key up %d\n", AKEYCODE_VOLUME_DOWN);
	WriteExact(m_monkeysock, buf, strlen(buf));
	key_checkok();
}

void RA::key_wake()
{
#ifdef USE_UINPUT
	raw_key(1, 116); // power
	raw_key(0, 116);
	raw_key(1, 102); // home
	raw_key(0, 102);
	raw_key(1, 158); // back
	raw_key(0, 158);
	return;
#endif

	//ta_log("TDC wake device");
	if( m_monkeysock == INVALID_SOCKET )
		return;
	char buf[1024];

	sprintf_s(buf, "wake\n");
	//sprintf_s(buf, "press 187\n");
	WriteExact(m_monkeysock, buf, strlen(buf));
	key_checkok();
}

void RA::key_power(bool bDown)
{
#ifdef USE_UINPUT
	raw_key(bDown, 116);
	return;
#endif
	//ta_log("TDC sleep device");
	if( m_monkeysock == INVALID_SOCKET )
		return;
	char buf[1024];
	
	//sprintf_s(buf, "press %d\n", AKEYCODE_POWER);
	if( bDown )
		sprintf_s(buf, "key down %d\n", AKEYCODE_POWER);
	else
		sprintf_s(buf, "key up %d\n", AKEYCODE_POWER);
	WriteExact(m_monkeysock, buf, strlen(buf));
	key_checkok();
}

void RA::key_camera(bool bDown)
{
#ifdef USE_UINPUT
	raw_key(bDown, 212);
	return;
#endif
	//ta_log("TDC KEYCODE_CAMERA");
	if( m_monkeysock == INVALID_SOCKET )
		return;
	char buf[1024];
	
	//sprintf_s(buf, "type abcdefg\n");
	//sprintf_s(buf, "press %d\n", AKEYCODE_CAMERA);
	if( bDown )
		sprintf_s(buf, "key down %d\n", AKEYCODE_CAMERA);
	else
		sprintf_s(buf, "key up %d\n", AKEYCODE_CAMERA);
	WriteExact(m_monkeysock, buf, strlen(buf));
	key_checkok();
}
	