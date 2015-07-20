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


package com.skplanet.skrc2.ftp;

import java.io.File;
import java.util.Date;

import javax.swing.table.AbstractTableModel;

@SuppressWarnings("serial")
class localFileModel extends AbstractTableModel {
	protected File dir;
	protected String[] filenames;

	protected String[] columnNames = new String[] { "name", "size"
	/* , "last modified", "directory?", "readable?", "writable?" */};

	@SuppressWarnings("rawtypes")
	protected Class[] columnClasses = new Class[] { String.class, String.class };


	public void sort(String[] Array) {
		int shortestStringIndex;

		if( Array == null || Array.length == 0)
			return;
		
		int[] iArray = new int[ Array.length];
		for( int i = 0; i < iArray.length; i++) {
			iArray[i] = 1;
			File f = new File(dir, Array[i]);
			if( f.isDirectory() )
				iArray[i] = 0;
		}
		
		for (int j = 0; j < Array.length - 1; j++) {
			shortestStringIndex = j;

			for (int i = j + 1; i < Array.length; i++) {
				// We keep track of the index to the smallest string
				if( iArray[i] < iArray[shortestStringIndex]) {
					shortestStringIndex = i;
				}
				else if ( iArray[i] == iArray[shortestStringIndex]) {
					if (Array[i].trim().compareTo(Array[shortestStringIndex].trim()) < 0) {
						shortestStringIndex = i;
					}
				}
			}
			// We only swap with the smallest string
			if (shortestStringIndex != j) {
				String temp = Array[j];
				int itemp = iArray[j];
				Array[j] = Array[shortestStringIndex];
				iArray[j] = iArray[shortestStringIndex];
				Array[shortestStringIndex] = temp;
				iArray[shortestStringIndex] = itemp;
			}
		}
	}

	
	// This table model works for any one given directory
	public localFileModel(File dir) {
		this.dir = dir;
		this.filenames = dir.list(); // Store a list of files in the directory
		sort(filenames);
	}

	// These are easy methods
	public int getColumnCount() {
		return 2;
	} // A constant for this model

	public int getRowCount() {
		if( filenames == null ) return 1;
		return filenames.length + 1;
	} // # of files in dir

	// Information about each column
	public String getColumnName(int col) {
		return columnNames[col];
	}

	@SuppressWarnings({ "unchecked", "rawtypes" })
	public Class getColumnClass(int col) {
		return columnClasses[col];
	}

	// The method that must actually return the value of each cell
	public Object getValueAt(int row, int col) {
		if (row == 0) {
			switch (col) {
			case 0:
				return "..";
			case 1:
				return "<Folder>";
			case 2:
				return null; // new Date();
			case 3:
				return Boolean.TRUE;
				// case 4: return Boolean.TRUE;
				// case 5: return Boolean.TRUE;
			default:
				return null;
			}
		}
		row = row - 1;

		File f = new File(dir, filenames[row]);
		switch (col) {
		case 0:
			return filenames[row];
		case 1:
			if (f.isDirectory()) {
				return "<Folder>";
			} else {
				return Long.toString(f.length());
			}
		case 2:
			return new Date(f.lastModified());
		case 3:
			return f.isDirectory() ? Boolean.TRUE : Boolean.FALSE;
			// case 4:
			// return f.canRead() ? Boolean.TRUE : Boolean.FALSE;
			// case 5:
			// return f.canWrite() ? Boolean.TRUE : Boolean.FALSE;
		default:
			return null;
		}
	}
}