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


package com.skplanet.skrc2.ruicUI;

import java.io.FileOutputStream;
import java.io.IOException;

public class LogData {
	public byte[] level = new byte[20];
	public byte[] time = new byte[30];
	public byte[] pid = new byte[20];
	public String tag = ""; //new String();
	public String text = ""; //new String();
	
	public LogData(String l, String t, String p, String ta, String te) {
		level = l.getBytes();
		time = t.getBytes();
		pid = p.getBytes();
		tag = ta;
		text = te;
	}
	
	public String get(int index) {
		if( index == 0 ) {
			return new String(level);
		}
		else if( index == 1 ) {
			return new String(time);
		}
		else if( index == 2 ) {
			return new String(pid);
		}
		else if( index == 3 ) {
			return tag;
		}
		else 
			return text;
	}

	public void write(FileOutputStream fos) throws IOException {
		fos.write(level);
		fos.write('\t');
		fos.write(time);
		fos.write('\t'); 	fos.write('(');
		fos.write(pid);
		fos.write(')'); fos.write('\t');
		fos.write(tag.getBytes());
		fos.write('\t'); fos.write(':'); fos.write('\t');
		fos.write(text.getBytes());
		fos.write('\r'); fos.write('\n');
	}
}
