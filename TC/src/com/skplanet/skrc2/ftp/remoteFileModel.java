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

import javax.swing.table.AbstractTableModel;

import com.skplanet.skrc2.rui.RUIClient;

@SuppressWarnings("serial")
class remoteFileModel extends AbstractTableModel {
	public RUIClient tc;
	protected String dir;
	protected String[] filenames;
	protected int numfiles;
	protected int[] sizearray;
	protected int[] datearray;

	protected String[] columnNames = new String[] { "name", "size"
			/*, "last modified", "directory?", "readable?", "writable?"*/ };

	@SuppressWarnings("rawtypes")
	protected Class[] columnClasses = new Class[] { String.class, String.class};

	public void sort() {
		int shortestStringIndex;
		
		if( numfiles == 0 || filenames == null || filenames.length == 0)
			return;

		for (int j = 0; j < numfiles - 1; j++) {
			shortestStringIndex = j;

			for (int i = j + 1; i < numfiles; i++) {
				// We keep track of the index to the smallest string
				if( sizearray[i] < sizearray[shortestStringIndex]) {
					shortestStringIndex = i;
				}
				else if (( sizearray[i] < 0  && sizearray[shortestStringIndex]<0) || 
						   ( sizearray[i] >=0 && sizearray[shortestStringIndex]>=0)){
					if (filenames[i].trim().compareTo(filenames[shortestStringIndex].trim()) < 0) {
						shortestStringIndex = i;
					}
				}
			}
			// We only swap with the smallest string
			if (shortestStringIndex != j) {
				String temp = filenames[j];
				int size = sizearray[j];
				int date = datearray[j];
				
				filenames[j] = filenames[shortestStringIndex];
				sizearray[j] = sizearray[shortestStringIndex];
				datearray[j] = datearray[shortestStringIndex];
				
				filenames[shortestStringIndex] = temp;
				sizearray[shortestStringIndex] = size;
				datearray[shortestStringIndex] = date;
			}
		}
	}
	
	// This table model works for any one given directory
	public remoteFileModel(String dir, RUIClient c) {
		this.tc = c;
		this.dir = dir;
		tc.tcService.reqFileList(dir);
		numfiles = 0;
		this.filenames = null; //dir.list(); // Store a list of files in the directory
	}

	// These are easy methods
	public int getColumnCount() {
		return 2;
	} // A constant for this model

	public int getRowCount() {
		if( filenames==null) return 1;
		return numfiles  + 1 ;
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
		 if( row == 0 ) {
		 switch(col) {
		 case 0: return "..";
		 case 1: return "<Folder>";
		 case 2: return null; //new Date();
    	 case 3: return Boolean.TRUE;
		 default: return null;
		 }
		 }
		 row = row-1;
		 
		switch (col) {
		case 0:
			return filenames[row];
		case 1:
			if( sizearray[row] == -1) {
				return "<Folder>";
			} else {
				return Integer.toString(sizearray[row]);
			}
		case 3:
			return (sizearray[row] == -1) ? Boolean.TRUE : Boolean.FALSE;
		default:
			return null;
		}
	}
	
	public void onFileList(int nFiles, String[] fnames, int[] sarray, int[] darray) {
		numfiles = nFiles;
		filenames = fnames;
		sizearray = sarray;
		datearray = darray;
		sort();
		this.fireTableDataChanged();
	}

}