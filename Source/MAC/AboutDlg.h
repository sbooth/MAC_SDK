#pragma once

class CAboutDlg : public CDialog
{
public:
    CAboutDlg(CWnd * pParent = NULL);

    enum { IDD = IDD_ABOUT };
    CStatic    m_ctrlTextPlaceholder;

protected:

    virtual void DoDataExchange(CDataExchange * pDX);
    virtual BOOL OnInitDialog();

    afx_msg HBRUSH OnCtlColor(CDC * pDC, CWnd * pWnd, UINT nCtlColor);
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()
};
