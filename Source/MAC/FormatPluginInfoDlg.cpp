#include "stdafx.h"
#include "MAC.h"
#include "FormatPluginInfoDlg.h"

CFormatPluginInfoDlg::CFormatPluginInfoDlg(CString strName, CString strVersion, CString strAuthor, CString strDescription, CString strURL, CWnd * pParent)
    : CDialog(CFormatPluginInfoDlg::IDD, pParent)
{
    m_strURL = strURL;
    m_strDescription1 = strName;
    if (strVersion.GetLength() > 0)
        m_strDescription1 += _T(" (") + strVersion + _T(")");
    if (strAuthor.GetLength() > 0)
        m_strDescription1 += _T(" by ") + strAuthor;
    m_strDescription2 = strDescription;
}

void CFormatPluginInfoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_URL, m_ctrlURL);
    DDX_Text(pDX, IDC_DESCRIPTION_1, m_strDescription1);
    DDX_Text(pDX, IDC_DESCRIPTION_2, m_strDescription2);
}

BEGIN_MESSAGE_MAP(CFormatPluginInfoDlg, CDialog)
END_MESSAGE_MAP()

BOOL CFormatPluginInfoDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    if (m_strURL.IsEmpty())
    {
        m_ctrlURL.SetWindowText(_T("no webpage available"));
        m_ctrlURL.SetURL(_T(""));
        m_ctrlURL.EnableWindow(FALSE);
    }
    else
    {
        m_ctrlURL.SetWindowText(m_strURL);
        m_ctrlURL.SetURL(m_strURL);
        m_ctrlURL.SetLinkCursor(LoadCursor(NULL, IDC_HAND));
    }
    
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
