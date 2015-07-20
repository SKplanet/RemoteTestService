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

import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import javax.swing.Action;
import javax.swing.JPopupMenu;
import javax.swing.JToggleButton;
import javax.swing.event.PopupMenuEvent;
import javax.swing.event.PopupMenuListener;

public class DropDownButton extends JToggleButton {

        private static final long serialVersionUID = 2857744715416733620L;
        
        private JPopupMenu menu;
        
        public DropDownButton(Action a) {
                super(a);
                init();
        }

        public DropDownButton(javax.swing.Icon icon, boolean selected) {
                super((javax.swing.Icon) icon, selected);
                init();
        }

        public DropDownButton(javax.swing.Icon icon) {
                super( icon);
                init();
        }

        public DropDownButton(String text, boolean selected) {
                super(text, selected);
                init();
        }

        public DropDownButton(String text, javax.swing.Icon icon, boolean selected) {
                super(text, icon, selected);
                init();
        }

        public DropDownButton(String text, javax.swing.Icon icon) {
                super(text, icon);
                init();
        }

        public DropDownButton(String text) {
                super(text);
                init();
        }

        public DropDownButton() {
                init();
        }

    private void init() {
        addItemListener(new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
                if(menu == null) return;
                boolean isSelected = e.getStateChange() == ItemEvent.SELECTED;
                                if (isSelected) {
                        menu.show(DropDownButton.this, 0, getHeight());
                }
            }
        });
        }

        public JPopupMenu getMenu() {
                return menu;
        }

        public void setMenu(JPopupMenu menu) {
                this.menu = menu;
                initMenu();
        }

        private void initMenu() {
        menu.addPopupMenuListener(new PopupMenuListener() {
            public void popupMenuWillBecomeVisible(PopupMenuEvent e) {
            }

            public void popupMenuWillBecomeInvisible(PopupMenuEvent e) {
                deselectButtonRequest();
            }

            public void popupMenuCanceled(PopupMenuEvent e) {
                deselectButtonRequest();
            }
        });
        }

        private void deselectButtonRequest() {
                setSelected(false);
        }

}