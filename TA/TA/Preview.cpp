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


// Preview.cpp : implementation file
//

#include "stdafx.h"
#include "TA.h"
#include "Preview.h"


// CPreview
void CPreview::OnUpdateComplete(RA* pra, void* pData, int size, int bFullsize)
{
	if( !pra || !pData)
		return;

	CPreview* pDlg = (CPreview*)pData;

	pDlg->Invalidate();
	/*CRect rt;
	pDlg->m_preview.GetClientRect(rt);
	pDlg->m_preview.ClientToScreen(rt);
	pDlg->ScreenToClient(rt);
	pDlg->InvalidateRect(rt, 0);*/

}

IMPLEMENT_DYNAMIC(CPreview, CStatic)

CPreview::CPreview()
{
	m_pra = NULL;
	m_Image = NULL;
	ratio = 0;
	m_lastOrientation = 0;
}

CPreview::~CPreview()
{
	m_pra->SetAdminUpdateCompleteCallback(NULL, NULL);
}


BEGIN_MESSAGE_MAP(CPreview, CStatic)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()



// CPreview message handlers


void CPreview::setRA(RA* pra)
{
	m_pra = pra;
	m_pra->SetAdminUpdateCompleteCallback(OnUpdateComplete, (void*)this);
}

void CPreview::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CStatic::OnPaint() for painting messages
	if( m_pra && m_pra->m_fbcsock != INVALID_SOCKET) {
		if( m_Image != NULL )
			delete m_Image;

		m_Image = NULL;

		DWORD dwResult = WaitForSingleObject(m_pra->m_fbBufferMutex, 500);

		if( dwResult != WAIT_OBJECT_0 )
		{
			m_pra->ta_log("FBC WARN fbc Buffer mutex wait over 500 milsec (%d) - Preview OnPaint", m_pra->m_fbBufferMutex);
			return;
		}
		m_Image = Image::FromStream(m_pra->m_pIStream);
		ReleaseMutex(m_pra->m_fbBufferMutex);
		
		if( m_Image != NULL )
		{
			//CWnd *p = this->GetDlgItem(IDC_STATIC_PREVIEW);
			CRect rt;
			GetClientRect(rt);
			Graphics g(GetSafeHwnd());
			//SolidBrush blackBrush(Color(255, 0, 0, 0));

			//g.FillRectangle(&blackBrush, 0,0,rt.Width(), rt.Height());

			double canvasWidth, canvasHeight, originalWidth, originalHeight;
			char devR = m_pra->m_devStatus & TDC_STATUS_ROTATE;
			if( m_lastOrientation != devR )
			{
				ratio = 0;
				m_lastOrientation = devR;
				CWnd* parent = GetParent();
				if( parent && parent->GetSafeHwnd())
				{
					CRect prt;
					parent->GetWindowRect(prt);
					parent->SetWindowPos(0,0,0,prt.Width()-rt.Width()+rt.Height(), prt.Height()-rt.Height() + rt.Width(), SWP_NOMOVE | SWP_NOZORDER);
					return;
				}
			}

			if( devR == 1 )
				m_Image->RotateFlip(Gdiplus::Rotate90FlipXY);
			else if ( devR == 2 )
				m_Image->RotateFlip(Gdiplus::Rotate180FlipXY);
			else if ( devR == 3 )
				m_Image->RotateFlip(Gdiplus::Rotate90FlipXY);

			rt.DeflateRect(1,1);
			canvasWidth = rt.Width();
			canvasHeight = rt.Height();
			originalWidth = m_Image->GetWidth();
			originalHeight = m_Image->GetHeight();

						// Figure out the ratio
			double ratioX = (double) canvasWidth / (double) originalWidth;
			double ratioY = (double) canvasHeight / (double) originalHeight;
			// use whichever multiplier is smaller
			double newratio = ratioX < ratioY ? ratioX : ratioY;
				
			if( ratio != newratio )
			{
				SolidBrush blackBrush(Color(255, 0, 0, 0));

				g.FillRectangle(&blackBrush, 0,0,rt.Width(), rt.Height());
				ratio = newratio;
			}

			// now we can get the new height and width
			nheight = (int)(originalHeight * ratio);
			nwidth = (int)(originalWidth * ratio);

			// Now calculate the X,Y position of the upper-left corner 
			// (one of these will always be zero)
			offx = (int)((canvasWidth - (originalWidth * ratio)) / 2);
			offy = (int)((canvasHeight - (originalHeight * ratio)) / 2);

			//extern Bitmap *pbm;
			//g.DrawImage(pbm, offx, offy, nwidth, nheight);
			g.DrawImage(m_Image, offx, offy, nwidth, nheight);
			this->ValidateRect(rt);
		}
	}
	
}


void CPreview::OnSize(UINT nType, int cx, int cy)
{
	CStatic::OnSize(nType, cx, cy);

	ratio = 0;
}


void CPreview::OnLButtonDown(UINT nFlags, CPoint point)
{
	if( m_pra ) {
		if( calcPoint(point) )
			m_pra->mouse_event(point.x, point.y, 1);
	}

	CStatic::OnLButtonDown(nFlags, point);
}


void CPreview::OnLButtonUp(UINT nFlags, CPoint point)
{
	if( m_pra ) {
		if( calcPoint(point) )
			m_pra->mouse_event(point.x, point.y, 0);
	}

	CStatic::OnLButtonUp(nFlags, point);
}


void CPreview::OnMouseMove(UINT nFlags, CPoint point)
{
	if( m_pra && nFlags & MK_LBUTTON) {
		if( calcPoint(point) )
			m_pra->mouse_event(point.x, point.y, 2);
	}

	CStatic::OnMouseMove(nFlags, point);
}


void CPreview::OnRButtonDown(UINT nFlags, CPoint point)
{
	if( m_pra )
	{
		m_pra->key_back(1);
		m_pra->key_back(0);
	}

	CStatic::OnRButtonDown(nFlags, point);
}


int CPreview::calcPoint(POINT & pt)
{
	if( pt.x < offx  || pt.x > offx+nwidth ||
		pt.y < offy || pt.y > offy+nheight) {
		return 0;
	}

	//TRACE("Before calc:%d:(%d,%d)-", m_pra->m_devOrientation, pt.x, pt.y);
	char devR = m_pra->m_devStatus & TDC_STATUS_ROTATE;
	if( devR == 0 ) {
		pt.x = (pt.x - offx)*m_pra->m_fbcWidth / nwidth;
		pt.y = (pt.y - offy)*m_pra->m_fbcHeight /nheight;
	}
	else if (devR == 2) {
		pt.x = m_pra->m_fbcWidth - (pt.x - offx)*m_pra->m_fbcWidth / nwidth;
		pt.y = m_pra->m_fbcHeight - (pt.y - offy)*m_pra->m_fbcHeight /nheight;
	}
	else if (devR == 1){
		pt.x = (pt.x - offx)*m_pra->m_fbcHeight / nwidth;
		pt.y = (pt.y - offy)*m_pra->m_fbcWidth / nheight;
	}
	else if (devR == 3){
		pt.x = m_pra->m_fbcHeight - (pt.x - offx)*m_pra->m_fbcHeight / nwidth;
		pt.y = m_pra->m_fbcWidth - (pt.y - offy)*m_pra->m_fbcWidth / nheight;
	}
	//TRACE("(%d,%d)\n", pt.x, pt.y);
	return 1;
}
