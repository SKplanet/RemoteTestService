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

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.SwingWorker;
import javax.swing.border.BevelBorder;
import javax.swing.table.DefaultTableCellRenderer;

import com.skplanet.skrc2.rui.RUIClient;
import com.skplanet.skrc2.utils.Settings;
import com.skplanet.skrc2.utils.Utils;

@SuppressWarnings("serial")
public class FTPDlg extends JDialog implements PropertyChangeListener{

	public RUIClient tc;
	protected  JTable localtable;
	protected  localFileModel localmodel;
	protected  File localdir;
	protected  JButton chRootButton;

	protected  JTable remotetable;
	protected  remoteFileModel remotemodel;
	protected  String remotedir;
	protected  JLabel remotepath;

	protected  JButton dirButton;
	protected  JButton btnClose;
	public  boolean toleft = false;
	public  enum Direction { LEFT, BOTH, RIGHT };
	private Task task;
	private boolean stopJob = true;
	public JProgressBar progressBar;
	protected  JLabel status;
	protected  Direction dirFTP;
	
	protected  String fromFile;
	protected  String toDir;
	protected  int fromFileSize;

	public JPanel localPanel;
	public JPanel remotePanel;
	protected  final byte[] recvbuffer = new byte[8192];
	
	class Task extends SwingWorker<Void, Void> {
		public int	downBufferSize;
		boolean buffercleared = true;

		public synchronized boolean getBufferCleared() {
			return buffercleared;
		}
		public synchronized void setBufferCleared(boolean b, int bufsize) {
			buffercleared = b;
			downBufferSize = bufsize;
		}
		
		public void onFileDownload( ByteBuffer buffer,  int size) {
			if( stopJob == true ) {
				setDownBuffer(null, 0);
				return;
			}
			
			if( getBufferCleared() == false ) {
				int nMax = 100;
				while(nMax-- > 0 && getBufferCleared() == false) {
					try {
						Thread.sleep(1);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
				Utils.LOG("sleep report:"+(10-nMax));
			}
			
			if( getBufferCleared() == false ) {
				Utils.LOG("Buffer not cleared fatal error.");
				setDownBuffer(null, 0);
				
				return;
			}
			
			setDownBuffer(buffer, size);
			
		}
		
		public void setDownBuffer(ByteBuffer buffer, int size) {
			if( size > 4) {
		    	int realsize = (int)(buffer.getShort() & 0xffffffffL);
		    	buffer.getShort();
		    	buffer.get(recvbuffer, 0, size-4);
	
		    	
				Utils.LOG("onFileDownload:" + realsize);
	
				setBufferCleared(false, realsize);
				downBufferSize = realsize;
			}
			else {
				setBufferCleared(false, 0);
			}
		}
		
		@Override
		public Void doInBackground() {
			// Initialize progress property.
			setProgress(0);
			
			if( dirFTP == Direction.BOTH) {
				Utils.LOG("Invaid ft Direction. Abort.");
				return null;
			}
			
			if( fromFile == null || fromFile.length() == 0 ) {
				Utils.LOG("Invaid fromFile. Abort.");
				return null;
			}
			if( toDir == null || toDir.length() == 0 ) {
				Utils.LOG("Invaid toDir. Abort.");
				return null;
			}
			Utils.LOG("=====");
			Utils.LOG("Direction:"+dirFTP);
			Utils.LOG("FromFile:"+fromFile);
			Utils.LOG("ToDir:"+toDir);
			Utils.LOG("=====");
			

			File localFile;
			FileOutputStream fos = null;
			FileInputStream fis = null;

			if( dirFTP == Direction.RIGHT) { // local to remote
				localFile = new File(fromFile);
				if( localFile.exists() ==  false ) {
					status.setText("File Read Error.");
					return null;
				}
				try {
					fis = new FileInputStream(localFile);
				} catch (FileNotFoundException e) {
					e.printStackTrace();
					status.setText("File Read Stream Error.");
					return null;
				}

				int nSep = fromFile.lastIndexOf(File.separator);
				String fFile = fromFile.substring(nSep+1, fromFile.length());
				String tFile;
				if( toDir.length() > 1 && toDir.endsWith(File.separator) ) {
					tFile = toDir + fFile;
				} else {
					tFile = toDir + File.separator + fFile;
				}
				
				tc.tcService.reqFileUpload(tFile);

			} else { // remote to local
				int nSep = fromFile.lastIndexOf(File.separator);
				String fFile = fromFile.substring(nSep+1, fromFile.length());
				localFile = new File(toDir, fFile);
				
				try {
					fos = new FileOutputStream(localFile);
				} catch (FileNotFoundException e) {
					e.printStackTrace();
					status.setText("Local File Error.");
					return null;
				}
				
				tc.tcService.reqFileDownload(fromFile);
			}
			blockUI(true);
			
			long filesize = fromFileSize;
			long readsum = 0;
			if( dirFTP == Direction.RIGHT) { // local to remote
				byte[] buffer = new byte[8192];
				int nRead;
				
				try {
					
					while( (nRead = fis.read(buffer)) > -1) {
						if( stopJob == true ) {	// user abort
							tc.tcService.cliFileErrorMessage(RUIClient.FILE_ERR_CANCELBYUSER);
							break;
						}
						readsum = readsum + nRead;
						int p = (int) (((float)readsum/ (float)filesize) * 100.0f);
						Utils.LOG("read(%d)/file(%d) : progress(%d)%%\n", readsum, filesize, p);
						if( nRead > 0)
							tc.tcService.reqFileUploadData(nRead, buffer, 0);
						
						setProgress( p);
					}
					fis.close();
					fis = null;
					
					tc.tcService.reqFileUploadData(4, null, (int)localFile.lastModified());
				} catch (IOException e) {

					e.printStackTrace();
				}
			} else {
				boolean bDown = true;
				setBufferCleared(true, 0);
				try {
					int nRead;

					while( bDown ) {
						if( getBufferCleared() == true ) {
							continue;
						}
						if( stopJob == true ) {	// user abort
							tc.tcService.cliFileErrorMessage(RUIClient.FILE_ERR_CANCELBYUSER);
							break;
						}
						
						if( downBufferSize == 0) {
							bDown = false;
							break;
						}

						nRead = downBufferSize;
						readsum = readsum + nRead;
						int p = (int) (((float)readsum/ (float)filesize) * 100.0f);
						Utils.LOG("read(%d)/file(%d) : progress(%d)%%\n", readsum, filesize, p);
						fos.write(recvbuffer, 0, nRead);
						setProgress( p);

						setBufferCleared(true,0);
					}
					fos.close();
					fos = null;
				} catch (IOException e) {

					e.printStackTrace();
					Utils.LOG("down file write exception");
					tc.tcService.cliFileErrorMessage(RUIClient.FILE_ERR_WRITE);
				}
				
			}
			
			try {
				if( fis != null ) {
						fis.close();
				}
				if( fos != null ) {
					fos.close();
				}
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			
			setProgress(0);
			

			blockUI(false);
			
			return null;
		}

		/*
		 * Executed in event dispatch thread
		 */
		public void done() {
			setProgress(0);
			blockUI(false);
			btnClose.setText("Close");
        	if( !stopJob) {
        		status.setText("Done.");
        		if( dirFTP == Direction.RIGHT) { // local to remote
					remotemodel = new remoteFileModel(remotedir, tc);
					setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
					remotetable.setModel(remotemodel);
        		} else {
					localmodel = new localFileModel(localdir);
					localtable.setModel(localmodel);
        		}
        	}
			stopJob = true;
			
		}
	}
	
	public FTPDlg(RUIClient c) {
		super(c.uiWindow, "File Transfer", true);
		tc = c;
		tc.ftpdlg = this;
		if (c.uiWindow != null) {
			Dimension parentSize = c.uiWindow.getSize();
			Point p = c.uiWindow.getLocation();

			setLocation(p.x + parentSize.width / 4, p.y + parentSize.height / 4);
		}

		progressBar = new JProgressBar(0,100);
		progressBar.setValue(0);
		progressBar.setVisible(false);
		//progressBar.setAlignmentX(RIGHT_ALIGNMENT);
		
		status = new JLabel("Select file to transfer.");
		
		JPanel pBtn = new JPanel();
		pBtn.setLayout(new BoxLayout(pBtn, BoxLayout.Y_AXIS));

		 btnClose = new JButton("Close");
		btnClose.setAlignmentX(RIGHT_ALIGNMENT);
		btnClose.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent arg0) {
				if( stopJob == false ) {
					status.setText("User Cancelled.");
					btnClose.setText("Close");
					stopJob = true;
					return;
				}
				dispose();
			}
		});
		pBtn.add(Box.createVerticalStrut(5));
		pBtn.add(progressBar);
		JPanel pStatus = new JPanel();
		pStatus.setLayout(new BoxLayout(pStatus, BoxLayout.X_AXIS));
		pStatus.add(status);
		pStatus.add(Box.createHorizontalGlue());
		pStatus.add(btnClose);
		
		pBtn.add(Box.createVerticalStrut(5));
		//pBtn.add(Box.createVerticalGlue());
		//pBtn.add(btnClose);
		pBtn.add(pStatus);
		// localPanel.add(pBtn, BorderLayout.SOUTH);

		Container currentPane = getContentPane();

		localPanel = initLocalPanel();
		remotePanel = initRemotePanel();

		dirButton = new JButton(">>>");
		dirButton.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent arg0) {
				stopJob = false;
				if( dirFTP == Direction.LEFT)
					try {
						toDir = localdir.getCanonicalPath();
					} catch (IOException e) {
						e.printStackTrace();
					}
				else
					toDir = remotedir;
				
				btnClose.setText("Cancel");
				progressBar.setVisible(true);
				task = new Task();
				task.addPropertyChangeListener(tc.ftpdlg);
				task.execute();
			}
		});
		JPanel dirPanel = new JPanel();
		dirPanel.setLayout(new BoxLayout(dirPanel, BoxLayout.Y_AXIS));
		dirButton.setAlignmentX(CENTER_ALIGNMENT);
		dirPanel.add(Box.createVerticalGlue());
		dirPanel.add(dirButton);
		dirPanel.add(Box.createVerticalGlue());

		this.setPreferredSize(new Dimension(680, 500));

		localPanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 0, 5));
		remotePanel.setBorder(BorderFactory.createEmptyBorder(5, 5, 0, 5));
		pBtn.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));

		currentPane.add(localPanel, "West");
		currentPane.add(dirPanel, "Center");
		currentPane.add(remotePanel, "East");
		currentPane.add(pBtn, "South");

		localtable.changeSelection(0, 0, false, false);
		setDirection(Direction.BOTH);

		pack();
		this.setResizable(false);
		this.setVisible(false);
		
		int mw = pStatus.getWidth() - btnClose.getWidth() - 30;
		status.setPreferredSize(new Dimension( mw , status.getHeight()));
		status.setMaximumSize(new Dimension( mw , status.getHeight()));

	}
	
	public void blockUI(boolean bBlock) {

		chRootButton.setEnabled(!bBlock);
		localtable.setEnabled(!bBlock);
		remotetable.setEnabled(!bBlock);
		dirButton.setEnabled(!bBlock);
		
		
	}

	public void doModal() {
		this.setVisible(true);
		status.setText("Select file to transfer.");
	}

	public void setDirection(Direction dir) {
		dirFTP = dir;
		if (dir == Direction.LEFT) {
			dirButton.setText("<<<");
			dirButton.setEnabled(true);
			remotetable.requestFocus();
		} else if( dir == Direction.RIGHT){
			dirButton.setText(">>>");
			dirButton.setEnabled(true);
			localtable.requestFocus();
		} else {
			dirButton.setText("---");
			dirButton.setEnabled(false);
			localtable.requestFocus();
		}
	}


	public JPanel initLocalPanel() {
		localdir = new File( Settings.getWorkingDirectory() /*System.getProperty("user.home")*/);
		localmodel = new localFileModel(localdir);
		localtable = new JTable(localmodel);
		localtable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

		//RowSorter<localFileModel> sorter = new TableRowSorter<localFileModel>(localmodel);
		//localtable.setRowSorter(sorter);

		DefaultTableCellRenderer rightRenderer = new DefaultTableCellRenderer();
		rightRenderer.setHorizontalAlignment(JLabel.RIGHT);
		localtable.getColumnModel().getColumn(1).setCellRenderer(rightRenderer);
		// localtable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
		// localtable.getColumnModel().getColumn(0).setPreferredWidth(200);
		// localtable.getColumnModel().getColumn(1).setPreferredWidth(80);

		localtable.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent e) {
				int nSel = localtable.getSelectedRow();

				if (nSel != -1) {
					if( (Boolean)localmodel.getValueAt(nSel, 3) == false) {
						setDirection(Direction.RIGHT);
						
						String child = (String) localmodel.getValueAt(nSel, 0);
						try {
							fromFile = new File(localdir, child).getCanonicalPath();
						} catch (IOException e1) {
							e1.printStackTrace();
							fromFile = null;
						}
						fromFileSize = Integer.parseInt((String) localmodel.getValueAt(nSel, 1));
					} else {
						setDirection(Direction.BOTH);
					}
				}

				if (nSel != -1 && e.getClickCount() == 2) {
					if ((Boolean) localmodel.getValueAt(nSel, 3) == true) { // direcotry
						String child = (String) localmodel.getValueAt(nSel, 0);
						localdir = new File(localdir, child);
						localmodel = new localFileModel(localdir);
						localtable.setModel(localmodel);
						try {
							chRootButton.setText(localdir.getCanonicalPath());
						} catch (IOException e1) {
							e1.printStackTrace();
						}
					}
				}
			}
		});

		try {
			chRootButton = new JButton(localdir.getCanonicalPath());

		} catch (IOException e2) {
			e2.printStackTrace();
		}
		chRootButton.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent e) {
				JFileChooser chooser = new JFileChooser();

				chooser.setCurrentDirectory(new File(localdir.getAbsolutePath()));

				chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
				chooser.setDialogTitle("Choose Directory to convert files");

				int returnVal = chooser.showOpenDialog(null);

				if (returnVal != JFileChooser.APPROVE_OPTION)
					return;

				File choice = chooser.getSelectedFile();

				if (!choice.isDirectory())
					return;
	            try {
					Settings.setWorkingDirectory(choice.getCanonicalPath());
				} catch (IOException e1) {
					e1.printStackTrace();
				}
				// FileLoader.fileStatus.setDirectory(choice);
				try {
					localdir = new File(choice.getCanonicalPath());
					localmodel.dir = localdir;
					localmodel.filenames = localdir.list();
					localmodel.fireTableDataChanged();
					chRootButton.setText(choice.getCanonicalPath());
				} catch (IOException e1) {
					e1.printStackTrace();
				}

				// fileBrowser.setRootFolder(choice.getCanonicalPath());
				// setRootFolder.setToolTipText("Change root folder: "
				// + fileBrowser.getRootfolder());
				// loadFilesInDirectory(choice);
			}

		});

		JScrollPane scrTable = new JScrollPane(localtable);
		// scrTable.setBorder(BorderFactory.createEmptyBorder(5, 0, 0, 0));

		JPanel panel = new JPanel();
		panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

		panel.add(new JLabel("Local Computer"));
		JPanel wPanel = new JPanel(new BorderLayout());
		wPanel.add(chRootButton, BorderLayout.CENTER);
		chRootButton.setSize(new Dimension(300, 18));

		panel.add(Box.createVerticalStrut(10));
		panel.add(wPanel);
		panel.add(Box.createVerticalStrut(10));

		panel.add(scrTable);
		panel.setPreferredSize(new Dimension(300, 400));

		return panel;
	}

	public JPanel initRemotePanel() {
		remotedir =  File.separator;
		remotemodel = new remoteFileModel(remotedir, tc);
		setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
		remotetable = new JTable(remotemodel);
		remotetable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

		DefaultTableCellRenderer rightRenderer = new DefaultTableCellRenderer();
		rightRenderer.setHorizontalAlignment(JLabel.RIGHT);
		remotetable.getColumnModel().getColumn(1)
				.setCellRenderer(rightRenderer);

		// remotetable.getColumnModel().getColumn(0).setPreferredWidth(200);
		// remotetable.getColumnModel().getColumn(1).setPreferredWidth(80);
		// remotetable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
		remotetable.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent e) {
				int nSel = remotetable.getSelectedRow();
				if (nSel != -1) {
					if( (Boolean)remotemodel.getValueAt(nSel, 3) == false) {
						setDirection(Direction.LEFT);
						String child = (String)remotemodel.getValueAt(nSel, 0);
						if( remotedir.length() > 1)
							fromFile = remotedir + File.separator + child;
						else
							fromFile = remotedir + child;
						fromFileSize = Integer.parseInt((String) remotemodel.getValueAt(nSel, 1));
					} else {
						setDirection(Direction.BOTH);
					}
				}

				if (nSel != -1 && e.getClickCount() == 2) {
					if( (Boolean)remotemodel.getValueAt(nSel, 3) == true) {
						// direcotry
						String child = (String) remotemodel.getValueAt(nSel, 0);
						if( child.compareTo("..") == 0 ) {
							int nSep = remotedir.lastIndexOf(File.separator);
							if( nSep == 0) nSep = 1;
							remotedir = remotedir.substring(0, nSep);
						} else {
							if( remotedir.length() > 1)
								remotedir = remotedir + File.separator + child;
							else
								remotedir = remotedir + child;
						}
						remotemodel = new remoteFileModel(remotedir, tc);
						setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
						remotetable.setModel(remotemodel);
						remotepath.setText(remotedir);
					 }
				}
			}
		});

		remotepath = new JLabel(remotedir);
		JPanel wPanel = new JPanel(new BorderLayout());
		wPanel.add(remotepath, BorderLayout.CENTER);
		remotepath.setBorder(BorderFactory
				.createBevelBorder(BevelBorder.LOWERED));
		remotepath.setSize(new Dimension(300, 18));

		JScrollPane scrTable = new JScrollPane(remotetable);
		// scrTable.setBorder(BorderFactory.createEmptyBorder(5, 0, 0, 0));

		JPanel panel = new JPanel();
		panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

		panel.add(new JLabel("Test Device"));
		panel.add(Box.createVerticalStrut(10));
		panel.add(wPanel);
		panel.add(Box.createVerticalStrut(10));

		panel.add(scrTable);
		panel.setPreferredSize(new Dimension(300, 400));

		return panel;
	}

	@Override
	public void propertyChange(PropertyChangeEvent evt) {
		if ("progress" == evt.getPropertyName()) {
            int progress = (Integer) evt.getNewValue();
            progressBar.setVisible((progress != 0 ));
            progressBar.setIndeterminate(false);
            progressBar.setValue(progress);
            Utils.LOG("Completed %d%% of task.\n", progress);
            if( progress > 0 )
            	status.setText( progress + "% Completed"); //. ("+fromFile+" to "+toDir + ")" );

        }
	}
	
	public void onFileList(int numFiles, String[] filenames, int[] sizearray, int[] datearray) {
		remotemodel.onFileList(numFiles, filenames, sizearray, datearray);
		setCursor(Cursor.getDefaultCursor());
	}
	
	public void onFileDownload(ByteBuffer buffer, int size) {
		task.onFileDownload(buffer, size);
	}
	
	public void onSvrFileError(int errcode) {
		status.setText(String.format("Server File Transfer Error.(%d)", errcode));
		stopJob = true;
		return;
	}
}
