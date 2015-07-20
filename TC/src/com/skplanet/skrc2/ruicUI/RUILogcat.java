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

import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.Iterator;
import java.util.Locale;
import java.util.Vector;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.ListSelectionModel;
import javax.swing.SwingConstants;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.table.AbstractTableModel;

import com.skplanet.skrc2.rui.RUIService;
import com.skplanet.skrc2.utils.Settings;
import com.skplanet.skrc2.utils.Utils;

@SuppressWarnings("serial")
public class RUILogcat extends JPanel implements MouseListener {
	private RUIService tcService = null;
	private GridBagLayout gBag;
	private JLabel levelLabel;
	private JComboBox levelCombo;
	String[] levelList = { "Verbose", "Debug", "Info", "Warning", "Error", "Fatal"};
	private JLabel tagLabel;
	private JTextField tagText;
	private Action level;
	private Action tag;
	private Action save;
	private Action apply;
	private Action clear;
	private Action close;
	private JButton applyButton;
	private JButton saveButton;
	private JButton clearButton;
	private JButton closeButton;
	private JTable logTable;
	private logTableModel modelTable;
    private JScrollPane logContentsPane;
	public Vector<LogData> logContents;
    private ByteBuffer logBuffer = ByteBuffer.allocate(40960);
    private byte[] line = new byte[4096];
    private boolean mousePressed = false;
    
    private String filterString;
    private int		filterLevel;	
    
	@SuppressWarnings({ "unchecked", "rawtypes" })
	
	public RUILogcat(RUIService c) {
		tcService = c;
		
		filterString = "";
	    filterLevel = 0;
		gBag = new GridBagLayout();
		this.setLayout(gBag);
		
		level = new levelAction();
		tag = new tagAction();
		save = new saveAction();
		apply = new applyAction();
		clear = new clearAction();
		close = new closeAction();
		
		applyButton = new JButton(apply);
		applyButton.setIcon(Utils.getButtonIcon("apply.png"));
		applyButton.setHideActionText(true);
		
		saveButton = new JButton(save);
		saveButton.setIcon(Utils.getButtonIcon("save.png"));
		saveButton.setHideActionText(true);
		saveButton.setPreferredSize(new Dimension(14,14));
		
		clearButton = new JButton(clear);
		clearButton.setIcon(Utils.getButtonIcon("clear.png"));
		clearButton.setHideActionText(true);
		
		closeButton = new JButton(close);
		closeButton.setIcon(Utils.getButtonIcon("close.png"));
		closeButton.setHideActionText(true);

		levelLabel = new JLabel("Level:");
		levelLabel.setHorizontalAlignment(SwingConstants.RIGHT);
		levelCombo = new JComboBox(levelList);
		levelCombo.setAction(level);
		levelCombo.setSelectedIndex(0);

		tagLabel = new JLabel("Tag:");
		tagLabel.setHorizontalAlignment(SwingConstants.RIGHT);
		
		tagText = new JTextField();
		tagText.setAction(tag);
		tagText.getDocument().addDocumentListener( new DocumentListener() {

			@Override
			public void changedUpdate(DocumentEvent arg0) {
					applyButton.setEnabled(true);
			}

			@Override
			public void insertUpdate(DocumentEvent arg0) {
					applyButton.setEnabled(true);
			}

			@Override
			public void removeUpdate(DocumentEvent arg0) {
					applyButton.setEnabled(true);
			}
			
		});
	
		
		logContents = new Vector();
		
		modelTable = new logTableModel();
		logTable = new JTable(modelTable);
		this.logTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
	    this.logTable.setAutoResizeMode(JTable.AUTO_RESIZE_LAST_COLUMN);
	    this.logTable.getColumnModel().getColumn(0).setPreferredWidth(20);	// level
	    this.logTable.getColumnModel().getColumn(0).setMaxWidth(40);
	    this.logTable.getColumnModel().getColumn(1).setPreferredWidth(120);	// time
	    this.logTable.getColumnModel().getColumn(1).setMaxWidth(150);
	    this.logTable.getColumnModel().getColumn(2).setPreferredWidth(50);	// pid
	    this.logTable.getColumnModel().getColumn(2).setMaxWidth(50);
	    this.logTable.getColumnModel().getColumn(3).setPreferredWidth(120);	// tag
	    this.logTable.getColumnModel().getColumn(4).setPreferredWidth(500);	// tag
		
		logContentsPane = new JScrollPane(logTable);
		logTable.setFillsViewportHeight(true);
		
		GridBagConstraints gbc = new GridBagConstraints();
		gbc.gridx = 0;
		gbc.gridy = 0;
		gbc.anchor = GridBagConstraints.WEST;
		gbc.insets = new Insets(1,3,1,3);
		this.gBag.setConstraints(levelLabel, gbc);
		this.add(levelLabel);
		
		gbc.gridx = 1;
		gbc.gridy = 0;
		gbc.anchor = GridBagConstraints.CENTER;
		this.gBag.setConstraints(levelCombo, gbc);
		this.add(levelCombo);	
		
		gbc.gridx = 2;
		gbc.gridy = 0;
		gbc.anchor = GridBagConstraints.CENTER;
		this.gBag.setConstraints(tagLabel, gbc);
		this.add(tagLabel);			
		
		gbc.gridx = 3;
		gbc.gridy = 0;
		gbc.weightx = 1.0;
		gbc.fill = GridBagConstraints.HORIZONTAL;
		this.gBag.setConstraints(tagText, gbc);
		this.add(tagText);	
		
		gbc.gridx = 4;
		gbc.gridy = 0;
		gbc.anchor = GridBagConstraints.CENTER;
		gbc.fill = GridBagConstraints.NONE;
		gbc.insets = new Insets(1,1,1,1);
		gbc.weightx = 0;
		this.gBag.setConstraints(applyButton, gbc);
		this.add(applyButton);	
		
		gbc.gridx = 5;
		gbc.gridy = 0;
		gbc.anchor = GridBagConstraints.CENTER;
		gbc.fill = GridBagConstraints.NONE;
		gbc.weightx = 0;
		this.gBag.setConstraints(clearButton, gbc);
		this.add(clearButton);	

		gbc.gridx = 6;
		gbc.gridy = 0;
		gbc.anchor = GridBagConstraints.CENTER;
		gbc.fill = GridBagConstraints.NONE;
		gbc.weightx = 0;
		this.gBag.setConstraints(saveButton, gbc);
		this.add(saveButton);	

		gbc.gridx = 7;
		gbc.gridy = 0;
		//gbc.insets = new Insets(1,3,1,1);
		gbc.anchor = GridBagConstraints.EAST;
		this.gBag.setConstraints(closeButton, gbc);
		this.add(closeButton);	

		gbc.gridx = 0;
		gbc.gridy = 1;
		gbc.gridwidth = 8;
		gbc.weighty = 1.0;
		gbc.fill = GridBagConstraints.BOTH;
		gbc.insets = new Insets(0,0,0,0);
		this.gBag.setConstraints(logContentsPane, gbc);
		this.add(logContentsPane);
		
		
	    this.setPreferredSize(new Dimension(500,60));
		applyButton.setEnabled(false);


	    
	}
	//- level
	class levelAction extends AbstractAction {
		public levelAction() {
			super("Level"); // Specify the name of the action
		}

		@Override
		public void actionPerformed(ActionEvent arg0) {
			applyFilter();
		}
	}

	//- tag
	class tagAction extends AbstractAction {
		public tagAction() {
			super("Tag"); // Specify the name of the action
		}

		@Override
		public void actionPerformed(ActionEvent arg0) {
			applyFilter();
		}
	}

	//- save
	class saveAction extends AbstractAction {
		public saveAction() {
			super("Save"); // Specify the name of the action
		}

		@Override
		public void actionPerformed(ActionEvent arg0) {
			saveLog();
		}
	}
	//- apply
	class applyAction extends AbstractAction {
		public applyAction() {
			super("Apply"); // Specify the name of the action
		}

		@Override
		public void actionPerformed(ActionEvent arg0) {
			applyFilter();
		}
	}
	//- clear
	class clearAction extends AbstractAction {
		public clearAction() {
			super("Clear"); // Specify the name of the action
		}

		@Override
		public void actionPerformed(ActionEvent arg0) {
			resetContents();
		}
	}
	//- close
	class closeAction extends AbstractAction {
		public closeAction() {
			super("Close"); // Specify the name of the action
		}

		@Override
		public void actionPerformed(ActionEvent arg0) {
			tcService.tc.showLogcat(false);
		}
	}
	public void setTCService(RUIService c)
	{
		tcService = c;
	}

	public void applyFilter() {
		if( tagText != null ) {
			filterString = tagText.getText();
		}
		
		if( levelCombo != null) {
			filterLevel = levelCombo.getSelectedIndex();
		}
		
		this.applyButton.setEnabled(false);
	}
	
	public void saveLog() {
		JFileChooser fc = new JFileChooser();
		FileNameExtensionFilter filter = new FileNameExtensionFilter(
                "log", "log");  //description,......확장자
        fc.setFileFilter(filter);    //필터 셋팅
        fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
		try{  
			String homePath = Settings.getWorkingDirectory(); //System.getProperty("user.home");
			//SimpleDateFormat formatter = new SimpleDateFormat ( "yyyyMMddkkmmssSSSS", Locale.KOREA );
			SimpleDateFormat formatter = new SimpleDateFormat ( "yyyyMMddkkmmss", Locale.KOREA );
			java.util.Date currentTime = new java.util.Date();
			String dTime = formatter.format ( currentTime );
			
			String name = String.format("%s/%s-%s.log", homePath, tcService.devBuildModel, dTime);
					
            //create a file object containing the cannonical path of the desired file   
            //File f = new File(new File("x.html").getCanonicalPath());  
            File f = new File(new File(name).getCanonicalPath());  
            //set the selected file  
            
            fc.setSelectedFile(f);  
        }catch (IOException ex3){  
        }
		int retVal = fc.showSaveDialog(this.tcService.tc.uiWindow);
		if (retVal == JFileChooser.APPROVE_OPTION) {  
            //get the currently selected file  
            //File thefile = fc.getSelectedFile();  
            File flogFile= fc.getSelectedFile();
            String nameOfFile = "";  
            nameOfFile = flogFile.getPath();
            Utils.LOG("save log:"+nameOfFile);
            try {
				Settings.setWorkingDirectory(fc.getCurrentDirectory().getCanonicalPath());
			} catch (IOException e1) {
				e1.printStackTrace();
			}
            
            FileOutputStream fos;
			try {
				fos = new FileOutputStream(flogFile);
				Iterator<LogData> it = logContents.iterator();
				LogData lData;
				for(; it.hasNext();) {
					lData = it.next();
					lData.write(fos);
				}
	    		fos.close();
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
			
		}		
	}
	public void addLog(String level, String time, String pid, String tag, String text) {
		LogData ld = new LogData(level, time, pid, tag, text);
		logContents.add(ld);
		int rowsel = logTable.getSelectedRow();
		Rectangle rtCell = logTable.getCellRect(rowsel, 0, false);
		Rectangle rtTable = logTable.getVisibleRect();
		if( (rtCell.y + rtCell.height) < rtTable.y)
			rowsel = -1;
		modelTable.fireTableDataChanged();

		if( rowsel >= 0 && rowsel < logTable.getRowCount()) {
			logTable.setRowSelectionInterval(rowsel, rowsel);
		} else {
			if( mousePressed == false)
				logTable.scrollRectToVisible(logTable.getCellRect(logContents.size()-1, 0, false));
		}
	}
	
	class logTableModel extends AbstractTableModel {
		
		String[] columnNames = { "Level", "Time", "PID", "Tag", "Text"};
		@Override
		public int getColumnCount() {
			return columnNames.length;
		}
		@Override
		public String getColumnName(int col) {
			return columnNames[col];
		}
		@Override
		public int getRowCount() {
			return logContents.size();
		}
		@Override
		public Object getValueAt(int row, int col) {
			return ((LogData) (logContents.get(row))).get(col);
			//return data[row][col];
		}
		
	}
	
	public void resetContents() {
		logBuffer.clear();
		logContents.clear();
		logTable.removeAll();
	}
	public void onLogcat(String log) {
		logBuffer.put(log.getBytes());
		logBuffer.flip();
		parseLog();
	}
	
	private void parseLog() {

		byte ch, chn;
		int pos = 0;

		logBuffer.mark();
		ch = logBuffer.get();
		while(pos < logBuffer.remaining()) {
			chn = logBuffer.get();
			if( ch == 0 || chn == 0)
				break;
			
			if( ch == 0x0d && chn == 0x0a ) {
				line[pos] = 0;
				//Utils.LOG("log line size:%d\n", pos);
				parseLine(pos-1);
				logBuffer.mark();
				chn = logBuffer.get();
				pos = 0;
			} else {
				line[pos] = ch;
				pos++;
			}
			ch = chn;
		}
		logBuffer.reset();
		logBuffer.compact();
	}
	
	private void parseLine(int pos) {
		
		String logLine = new String(line, 0, pos);
		String level;
		String time;
		String pid;
		String tag;
		String text;
		
		String temp[];
		//Utils.LOG("---(%s)\n", logLine);
		temp = logLine.split("/", 2);
		if( temp.length < 2) 
			return;
		level = temp[0]; // new String(temp[0]);
		
		int nLevel;
		if( level.equalsIgnoreCase("v"))
			nLevel = 0;
		else if( level.equalsIgnoreCase("d"))
			nLevel = 1;
		else if( level.equalsIgnoreCase("i"))
			nLevel = 2;
		else if( level.equalsIgnoreCase("w"))
			nLevel = 3;
		else if( level.equalsIgnoreCase("e"))
			nLevel = 4;
		else if( level.equalsIgnoreCase("f"))
			nLevel = 5;
		else	// begin of log of some error line
			return;
		
		if(  nLevel < filterLevel)	// level filter
			return;
		
		if( !filterString.isEmpty() ) {	// tag filter
			if( logLine.indexOf(filterString) < 0 )
				return;
		}

		temp = temp[1].split("\\(",2);
		if( temp.length < 2 )
			return;
		tag = temp[0];

		temp = temp[1].split("\\)",2);
		if( temp.length < 2 )
			return;
		pid = temp[0];

		temp = temp[1].split(":",2);
		text = temp[1];
				
		SimpleDateFormat formatter = new SimpleDateFormat ( "yyyyMMdd kkmmssSSS", Locale.KOREA );
		java.util.Date currentTime = new java.util.Date();
		time = formatter.format ( currentTime );
		
		addLog(level, time, pid, tag, text);
	}

	@Override
	public void mouseClicked(MouseEvent arg0) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void mouseEntered(MouseEvent arg0) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void mouseExited(MouseEvent arg0) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void mousePressed(MouseEvent arg0) {
		mousePressed = true;
		
	}

	@Override
	public void mouseReleased(MouseEvent arg0) {
		mousePressed = false;
		
	}
}
