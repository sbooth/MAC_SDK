#include "stdafx.h"
#include "MAC.h"
#include "AboutDlg.h"
#include "GlobalFunctions.h"

CAboutDlg::CAboutDlg(CWnd * pParent)
    : CDialog(CAboutDlg::IDD, pParent)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TEXT_PLACE_HOLDER, m_ctrlTextPlaceholder);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    ON_WM_CTLCOLOR()
    ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    if (sizeof(intn) == 8)
        SetWindowText(MAC_NAME _T(" (64-bit)"));
    else
        SetWindowText(MAC_NAME _T(" (32-bit)"));

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CAboutDlg::OnCtlColor(CDC * pDC, CWnd * pWnd, UINT nCtlColor) 
{
    HBRUSH hBrush = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    //hBrush = CreateSolidBrush(RGB(255, 255, 255));
    return hBrush;
}

void CAboutDlg::OnPaint() 
{
    CPaintDC dc(this);
    int nSavedDC = dc.SaveDC();

    CRect rectPaint; m_ctrlTextPlaceholder.GetWindowRect(&rectPaint); ScreenToClient(&rectPaint);
    dc.SetBkMode(TRANSPARENT);

    LOGFONT LogFont; 
    CFont * pfontGUI = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));

    pfontGUI->GetLogFont(&LogFont);
    LogFont.lfWidth = (LogFont.lfWidth * 180) / 100;
    LogFont.lfHeight = (LogFont.lfHeight * 180) / 100;
    LogFont.lfWeight = FW_BOLD;
    CFont fontLarge; fontLarge.CreateFontIndirect(&LogFont);
    
    pfontGUI->GetLogFont(&LogFont);
    LogFont.lfWidth = (LogFont.lfWidth * 120) / 100;
    LogFont.lfHeight = (LogFont.lfHeight * 120) / 100;
    CFont fontSmall; fontSmall.CreateFontIndirect(&LogFont);

    dc.SelectObject(&fontSmall);
    int nFontSmallHeight = dc.GetTextExtent(_T("Ay")).cy;
    dc.SelectObject(&fontLarge);
    int nFontLargeHeight = dc.GetTextExtent(_T("Ay")).cy;
    int nLineSpacing = 3;

    int nFullHeight = (nFontLargeHeight * 1) + (nFontSmallHeight * 3) + (nLineSpacing * 3);
    
    CRect rectSize;
    int nTop = rectPaint.top + ((rectPaint.Height() - nFullHeight) / 2);

    dc.SelectObject(&fontLarge);
    dc.DrawText(MAC_NAME, &rectSize, DT_CENTER | DT_NOPREFIX | DT_CALCRECT);
    dc.DrawText(MAC_NAME, CRect(rectPaint.left, nTop, rectPaint.right, nTop + rectSize.Height()), DT_CENTER | DT_NOPREFIX);
    nTop += nFontLargeHeight + nLineSpacing;
    
    dc.SelectObject(&fontSmall);
    dc.DrawText(MAC_RESOURCE_COPYRIGHT, &rectSize, DT_CENTER | DT_NOPREFIX | DT_CALCRECT);
    dc.DrawText(MAC_RESOURCE_COPYRIGHT, CRect(rectPaint.left, nTop, rectPaint.right, nTop + rectSize.Height()), DT_CENTER | DT_NOPREFIX);
    nTop += nFontSmallHeight + nLineSpacing;

    dc.SelectObject(&fontSmall);
    dc.DrawText(_T("All rights reserved."), &rectSize, DT_CENTER | DT_NOPREFIX | DT_CALCRECT);
    dc.DrawText(_T("All rights reserved."), CRect(rectPaint.left, nTop, rectPaint.right, nTop + rectSize.Height()), DT_CENTER | DT_NOPREFIX);
    nTop += nFontSmallHeight + nLineSpacing;

    CStringArrayEx aryCPU;
    #ifdef ENABLE_SSE_ASSEMBLY
        if (GetSSEAvailable(false))
            aryCPU.Add(_T("SSE: yes"));
        else
            aryCPU.Add(_T("SSE: no"));
    #else
        aryCPU.Add(_T("SSE: no"));
    #endif

    #ifdef ENABLE_AVX_ASSEMBLY
        if (GetAVX2Available())
            aryCPU.Add(_T("AVX2: yes"));
        else
            aryCPU.Add(("AVX2: no"));
    #else
        aryCPU.Add(_T("AVX2: no"));
    #endif

    #ifdef ENABLE_MMX_ASSEMBLY
        aryCPU.Add(_T("MMX: yes"));
    #else
        aryCPU.Add(_T("MMX: no"));
    #endif

    if (IsProcessElevated())
        aryCPU.Add(_T("elevated: yes"));
    else
        aryCPU.Add(_T("elevated: no"));

    CString strCPU = aryCPU.GetList(_T("; "));
    dc.DrawText(strCPU, &rectSize, DT_CENTER | DT_NOPREFIX | DT_CALCRECT);
    dc.DrawText(strCPU, CRect(rectPaint.left, nTop, rectPaint.right, nTop + rectSize.Height()), DT_CENTER | DT_NOPREFIX);
    nTop += nFontSmallHeight + nLineSpacing;
    
    dc.RestoreDC(nSavedDC);

}
