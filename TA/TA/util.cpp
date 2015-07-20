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
#include "resource.h"
#include "TA.h"
#include "TADlg.h"
/*
��ư ��Ÿ��                         �ǹ�
MB_ABORTRETRYIGNORE                [���], [��õ�], [����]
MB_OK                              [Ȯ��]
MB_OKCANCEL                        [Ȯ��], [���]
MB_RETRYCANCEL                     [��õ�], [���]
MB_YESNO                           [��], [�ƴϿ�]
MB_YESNOCANCEL                     [��], [�ƴϿ�], [���]
 
 
������ ��Ÿ��                       �ǹ�
MB_ICONEXCLAMATION                 [����ǥ]
MB_ICONINFORMATION                 [����ǥ]
MB_ICONQUESTION                    [����ǥ]
MB_ICONSTOP                        [X]
 
 
����Ʈ ��ư                         �ǹ�
MB_DEFBUTTON1                      ù ��° ��ư
MB_DEFBUTTON2                      �� ��° ��ư
MB_DEFBUTTON3                      �� ��° ��ư
 
 
��޸�Ƽ                            �ǹ�
MB_APPLMODAL                       �޽��� �ڽ��� ������Ѿ� ���α׷��� ��� ������ �� ����
MB_SYSTEMMODAL                     �޽��� �ڽ��� ������Ѿ� �ý����� ����� �� ����
 
 
��ȯ��                             �ǹ�
IDABORT                            [���(Abort)]�� ���� ����
IDCANCEL                           [���(Cancel)]�� ���� ����
IDIGNORE                           [����]�� ���� ����
IDNO                               [�ƴϿ�]�� ���� ����
IDOK                               [Ȯ��]�� ���� ����
IDRETRY                            [��õ�]�� ���� ����
IDYES                              [��]�� ���� ����
*/
int Message(UINT idMsg, UINT type)
{
	return AfxMessageBox(idMsg, type);
}
