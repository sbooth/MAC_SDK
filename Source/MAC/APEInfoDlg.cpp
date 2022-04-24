#include "stdafx.h"
#include "MAC.h"
#include "APEInfoDlg.h"

CAPEInfoDlg::CAPEInfoDlg(CStringArray & aryFiles, CWnd * pParent) :
    CDialog(CAPEInfoDlg::IDD, pParent),
    m_dlgFormat(this)
{
    m_aryFiles.Copy(aryFiles);
}

void CAPEInfoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_FILE_LIST, m_ctrlFiles);
    DDX_Control(pDX, IDC_TABS, m_ctrlTabs);
}

BEGIN_MESSAGE_MAP(CAPEInfoDlg, CDialog)
    ON_BN_CLICKED(IDC_FILES_SELECT_ALL, OnBnClickedFilesSelectAll)
    ON_BN_CLICKED(IDC_FILES_SELECT_NONE, OnBnClickedFilesSelectNone)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_FILE_LIST, OnLvnItemchangedFileList)
END_MESSAGE_MAP()

BOOL CAPEInfoDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    CRect rectFiles; m_ctrlFiles.GetWindowRect(&rectFiles); ScreenToClient(&rectFiles);
    m_ctrlFiles.InsertColumn(0, _T("Files"), LVCFMT_LEFT, rectFiles.Width() - GetSystemMetrics(SM_CXVSCROLL) - 2);
    for (int z = 0; z < m_aryFiles.GetSize(); z++)
    {
        CFilename fnFile(m_aryFiles[z]);
        m_ctrlFiles.InsertItem(z, fnFile.GetNameAndExtension(), -1);
    }

    m_ctrlTabs.InsertItem(0, _T("Format"));
    //m_ctrlTabs.InsertItem(1, _T("Other Stuff"));

    CRect rectTabs; m_ctrlTabs.GetWindowRect(&rectTabs); ScreenToClient(&rectTabs);
    CRect rectTab; m_ctrlTabs.GetItemRect(0, &rectTab);
    int nTabTitleBarHeight = rectTab.Height() + 2;

    m_dlgFormat.Create(IDD_APE_INFO_FORMAT, this);
    m_dlgFormat.SetWindowPos(&m_ctrlTabs, rectTabs.left + 8, rectTabs.top + nTabTitleBarHeight + 8, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE);

    m_dlgFormat.SetFiles(m_aryFiles);
    
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAPEInfoDlg::OnBnClickedFilesSelectAll()
{
    for (int z = 0; z < m_ctrlFiles.GetItemCount(); z++)
        m_ctrlFiles.SetItemState(z, LVIS_SELECTED, LVIS_SELECTED);
}

void CAPEInfoDlg::OnBnClickedFilesSelectNone()
{
    for (int z = 0; z < m_ctrlFiles.GetItemCount(); z++)
        m_ctrlFiles.SetItemState(z, 0, LVIS_SELECTED);
}

void CAPEInfoDlg::OnLvnItemchangedFileList(NMHDR *pNMHDR, LRESULT *pResult)
{
    //LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    
    CStringArray arySelected;
    POSITION Pos = m_ctrlFiles.GetFirstSelectedItemPosition();
    while (Pos)
    {
        int nIndex = m_ctrlFiles.GetNextSelectedItem(Pos);
        arySelected.Add(m_aryFiles[nIndex]);
    }

    m_dlgFormat.SetFiles(arySelected);

    *pResult = 0;
}
