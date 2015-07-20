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
버튼 스타일                         의미
MB_ABORTRETRYIGNORE                [취소], [재시도], [무시]
MB_OK                              [확인]
MB_OKCANCEL                        [확인], [취소]
MB_RETRYCANCEL                     [재시도], [취소]
MB_YESNO                           [예], [아니오]
MB_YESNOCANCEL                     [예], [아니오], [취소]
 
 
아이콘 스타일                       의미
MB_ICONEXCLAMATION                 [느낌표]
MB_ICONINFORMATION                 [느낌표]
MB_ICONQUESTION                    [물음표]
MB_ICONSTOP                        [X]
 
 
디폴트 버튼                         의미
MB_DEFBUTTON1                      첫 번째 버튼
MB_DEFBUTTON2                      두 번째 버튼
MB_DEFBUTTON3                      세 번째 버튼
 
 
모달리티                            의미
MB_APPLMODAL                       메시지 박스를 종료시켜야 프로그램을 계속 진행할 수 있음
MB_SYSTEMMODAL                     메시지 박스를 종료시켜야 시스템을 사용할 수 있음
 
 
반환값                             의미
IDABORT                            [취소(Abort)]가 눌러 졌음
IDCANCEL                           [취소(Cancel)]이 눌러 졌음
IDIGNORE                           [무시]가 눌러 졌음
IDNO                               [아니오]가 눌러 졌음
IDOK                               [확인]이 눌러 졌음
IDRETRY                            [재시도]가 눌러 졌음
IDYES                              [예]가 눌러 졌음
*/
int Message(UINT idMsg, UINT type)
{
	return AfxMessageBox(idMsg, type);
}
