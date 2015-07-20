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

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.JTextField;
import javax.swing.SwingWorker;
import javax.swing.filechooser.FileNameExtensionFilter;

import com.skplanet.skrc2.rui.RUIClient;
import com.skplanet.skrc2.utils.Settings;
import com.skplanet.skrc2.utils.Utils;

@SuppressWarnings("serial")
public class AppInstDlg extends JDialog implements ActionListener, PropertyChangeListener {

	public RUIClient tc;
	public JLabel msg;
	public JTextField filePath;
	public JButton    brwButton;
	public JProgressBar progressBar;
	public JButton    installButton;
	public JButton    closeButton;
	private boolean stopJob = true;
	private Task task;

	class Task extends SwingWorker<Void, Void> {
		/*
		 * Main task. Executed in background thread.
		 */
		@Override
		public Void doInBackground() {
			// Initialize progress property.
			setProgress(0);
			File apkFile = new File(filePath.getText());
			if( apkFile.exists() ==  false ) {
				msg.setText("APK File is not exist.");
				return null;
			}
			
			FileInputStream fis;
			final int buffer_size = 8192;
			try {
				fis = new FileInputStream(apkFile);
			} catch (FileNotFoundException e) {
				msg.setText("APK File could not open.");
				return null;
			}
			msg.setText("Transferring Application Data...");
			tc.tcService.reqAppInstall(filePath.getText(), 1);

			byte[] buffer = new byte[buffer_size];
			int read;
			long filesize = apkFile.length();
			long readsum = 0;
			try {
				while( (read = fis.read(buffer)) >= 0 ) {
					if( stopJob == true ) {	// user abort
						tc.tcService.reqAppInstall(filePath.getText(), 0);
						break;
					}
					if( read == 0 ) {
						tc.tcService.sendAppInstallData(null, 0);
					}
					readsum = readsum + read;
					int p = (int) (((float)readsum/ (float)filesize) * 100.0f);
					Utils.LOG("read(%d)/file(%d) : progress(%d)%%\n", readsum, filesize, p);
					tc.tcService.sendAppInstallData(buffer,  read);
					if( readsum == filesize ) {
						tc.tcService.sendAppInstallData(null, 0);
						break;
					}					
					setProgress( p);
				}
				fis.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
			
			setProgress(0);
			if( stopJob ) {
				msg.setText("Aborted.");
			} else {
				msg.setText("Installing Application...");
			}
			return null;
		}

		/*
		 * Executed in event dispatch thread
		 */
		public void done() {
			//Toolkit.getDefaultToolkit().beep();
			installButton.setEnabled(true);
			closeButton.setText("Close");
			stopJob = true;
			dispose();
		}
	}
	
	public AppInstDlg(RUIClient c) {
		super(c.uiWindow, "App Install", true);
		tc = c;
		if( c.uiWindow != null ) {
			Dimension parentSize = c.uiWindow.getSize();
			Point p = c.uiWindow.getLocation();
			
			setLocation(p.x + parentSize.width / 4, p.y + parentSize.height / 4);
		}
		
		JPanel msgPane = new JPanel();
		msgPane.setLayout(new FlowLayout(FlowLayout.LEFT));
		msg = new JLabel("Choose application to install:");
		msgPane.add(msg);
		
		JPanel apkPane = new JPanel();
		GridBagLayout gBag = new GridBagLayout();
		apkPane.setLayout(gBag);
		GridBagConstraints gbc = new GridBagConstraints();
		
		JLabel fpLabel = new JLabel("File Path:");
		gbc.gridx = 0;
		gbc.insets = new Insets(1,5,1,5);
		gbc.anchor = GridBagConstraints.WEST;
		gBag.setConstraints(fpLabel, gbc);
		apkPane.add(fpLabel);
		
		filePath = new JTextField();
		gbc.gridx = 1;
		gbc.weightx = 1.0;
		gbc.anchor = GridBagConstraints.CENTER;
		gbc.fill = GridBagConstraints.HORIZONTAL;
		gBag.setConstraints(filePath, gbc);
		apkPane.add(filePath);

		brwButton = new JButton("...");
		brwButton.addActionListener(this);
		gbc.gridx = 2;
		gbc.weightx = 0;
		gbc.anchor = GridBagConstraints.EAST;
		gbc.fill = GridBagConstraints.NONE;
		gBag.setConstraints(brwButton, gbc);
		apkPane.add(brwButton);
		
		progressBar = new JProgressBar(0,100);
		progressBar.setValue(0);
		gbc.gridx = 0;
		gbc.gridwidth = 3;
		gbc.gridy = 1;
		gbc.weightx = 1.0;
		gbc.anchor = GridBagConstraints.CENTER;
		gbc.fill = GridBagConstraints.HORIZONTAL;
		gBag.setConstraints(progressBar, gbc);
		progressBar.setVisible(false);
		apkPane.add(progressBar);
		
		JPanel cmdPane = new JPanel();
		cmdPane.setLayout(new FlowLayout(FlowLayout.RIGHT));
		installButton = new JButton("Install");
		installButton.addActionListener(this);
		closeButton = new JButton("Close");
		closeButton.addActionListener(this);
		cmdPane.add(installButton);
		cmdPane.add(closeButton);
		
		getContentPane().add(msgPane, BorderLayout.PAGE_START);
		getContentPane().add(apkPane, BorderLayout.CENTER);
		getContentPane().add(cmdPane, BorderLayout.PAGE_END);

		setDefaultCloseOperation(DISPOSE_ON_CLOSE);
		this.setPreferredSize(new Dimension(500,150));
		this.setResizable(false);
		pack();
		setVisible(false);
	}
	
	public void doModal() {
		setVisible(true);
	}
	@Override
	public void actionPerformed(ActionEvent e) {
		if( e.getSource() == installButton ) {
			//check file exist
			File checkFile = new File(filePath.getText());
			if( checkFile.exists() == false ) {
				JOptionPane.showMessageDialog(null, "File is not exist!", RUIClient.TC_PRODUCT_NAME, JOptionPane.ERROR_MESSAGE);
				return;
			}
			
			// do start
			stopJob = false;
			progressBar.setVisible(true);
			task = new Task();
			task.addPropertyChangeListener(this);
			task.execute();
			installButton.setEnabled(false);
			closeButton.setText("Stop");
		}
		else if ( e.getSource() == closeButton ) {
			if( stopJob) {
				this.dispose();
			} else {
				stopJob = true;
				tc.tcService.reqAppInstall(filePath.getText(), 0);
				installButton.setEnabled(true);
				closeButton.setText("Close");
			}
		}
		else if ( e.getSource() == brwButton ) {
			JFileChooser fc = new JFileChooser( Settings.getWorkingDirectory()); //System.getProperty("user.home"));
			FileNameExtensionFilter filter = new FileNameExtensionFilter(
	                "APK", "apk");  //description,......확장자
	        fc.setFileFilter(filter);    //필터 셋팅
	        fc.setFileSelectionMode(JFileChooser.FILES_ONLY);

			int retVal = fc.showOpenDialog(this);
			if (retVal == JFileChooser.APPROVE_OPTION) {  
	            //get the currently selected file  
	            //File thefile = fc.getSelectedFile();  
	            File fileAPK= fc.getSelectedFile();
	            String nameOfFile = "";  
	            nameOfFile = fileAPK.getPath();

	            filePath.setText(nameOfFile);
	            try {
					Settings.setWorkingDirectory(fc.getCurrentDirectory().getCanonicalPath());
				} catch (IOException e1) {
					e1.printStackTrace();
				}
			}
		}

	}
	@Override
	public void propertyChange(PropertyChangeEvent evt) {
		if ("progress" == evt.getPropertyName()) {
            int progress = (Integer) evt.getNewValue();
            progressBar.setVisible((progress != 0 ));
            progressBar.setIndeterminate(false);
            progressBar.setValue(progress);
            Utils.LOG("Completed %d%% of task.\n", progress);
        }
	}

	public void onResult(int nResult, String reason) {
		if( nResult  == 1 ) {
			if( reason != null && reason.length() > 0 ) {
				msg.setText(reason);
			} else {
				msg.setText("Success.");
			}
		}
		else if ( nResult == 2 ) {
			if( reason != null && reason.length() > 0 ) {
				msg.setText(reason);
			} else {
				msg.setText("File creation failed.");
			}
		}
		else if ( nResult == 3 ) {
			if( reason != null && reason.length() > 0 ) {
				msg.setText(reason);
			} else {
				msg.setText("Install failed.");
			}
		}
		else if ( nResult == 4 ) {
			if( reason != null && reason.length() > 0 ) {
				msg.setText(reason);
			} else {
				msg.setText("User Cancelled.");
			}
		}
		else {
			msg.setText("Unknown response from server.");
		}
	}
}
