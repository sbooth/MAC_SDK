#include "stdafx.h"
#include "MAC.h"
#include "OptionsDlg.h"

#include "OptionsProcessingDlg.h"
#include "OptionsOutputDlg.h"
#include "APEButtons.h"

COptionsDlg::COptionsDlg(CWnd * pParent)
    : CDialog(COptionsDlg::IDD, pParent)
{
}

void COptionsDlg::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PAGE_LIST, m_ctrlPageList);
    DDX_Control(pDX, IDC_PAGE_FRAME, m_ctrlPageFrame);
}

BEGIN_MESSAGE_MAP(COptionsDlg, CDialog)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_PAGE_LIST, OnItemchangedPageList)
    ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL COptionsDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    m_ctrlPageList.SetImageList(theApp.GetImageList(CMACApp::Image_OptionsList), LVSIL_SMALL);

    m_aryPages.Add(new OPTIONS_PAGE(new COptionsProcessingDlg, "Processing", TBB_OPTIONS_PAGE_PROCESSING, IDD_OPTIONS_PROCESSING, this));
    m_aryPages.Add(new OPTIONS_PAGE(new COptionsOutputDlg, "Output", TBB_OPTIONS_PAGE_OUTPUT, IDD_OPTIONS_OUTPUT, this));

    for (int z = 0; z < m_aryPages.GetSize(); z++)
        m_ctrlPageList.InsertItem(z, m_aryPages[z]->m_strCaption, m_aryPages[z]->m_nIcon);

    m_ctrlPageList.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsDlg::OnItemchangedPageList(NMHDR * pNMHDR, LRESULT * pResult) 
{
    int nPage = GetSelectedPage();
    if (nPage < 0)
    {
        // if no page is selected (off the bottom) just select the first page
        m_ctrlPageList.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
    }
    else
    {
        UpdatePage();
    }
    
    *pResult = 0;
}

BOOL COptionsDlg::UpdatePage()
{
    // hide all the pages
    for (int z = 0; z < m_aryPages.GetSize(); z++)
        m_aryPages[z]->m_pDialog->ShowWindow(SW_HIDE);
    m_ctrlPageFrame.SetWindowText(_T(""));

    // get the new page
    int nPage = GetSelectedPage();

    // update
    if ((nPage >= 0) && (nPage < m_aryPages.GetSize()))
    {
        // frame text
        m_ctrlPageFrame.SetWindowText(m_aryPages[nPage]->m_strCaption);

        // position / visibility
        const int nBorderWidth = theApp.GetSize(4, 0).cx;
        const int nTopBorder = theApp.GetSize(10, 0).cx;
        CRect rectFrame; m_ctrlPageFrame.GetWindowRect(&rectFrame); ScreenToClient(&rectFrame);
    
        m_aryPages[nPage]->m_pDialog->SetWindowPos(NULL, rectFrame.left + nBorderWidth, rectFrame.top + nBorderWidth + nTopBorder, 
            0, 0, SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOSIZE);
    }

    return TRUE;
}

int COptionsDlg::GetSelectedPage()
{
    int nPage = -1;
    POSITION Pos = m_ctrlPageList.GetFirstSelectedItemPosition();
    if (Pos != NULL)
        nPage = m_ctrlPageList.GetNextSelectedItem(Pos);
    return nPage;
}

void COptionsDlg::OnDestroy() 
{
    for (int z = 0; z < m_aryPages.GetSize(); z++)
        SAFE_DELETE(m_aryPages[z])
    m_aryPages.RemoveAll();
    
    CDialog::OnDestroy();
}

void COptionsDlg::OnOK() 
{
    for (int z = 0; z < m_aryPages.GetSize(); z++)
        m_aryPages[z]->m_pDialog->SendMessage(UM_SAVE_PAGE_OPTIONS);

    theApp.GetSettings()->Save();

    CDialog::OnOK();
}
