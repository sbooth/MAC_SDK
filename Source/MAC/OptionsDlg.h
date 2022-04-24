#pragma once

#include "OptionsShared.h"

class COptionsDlg : public CDialog
{
public:

    COptionsDlg(CWnd* pParent = NULL);

    enum { IDD = IDD_OPTIONS };
    CListCtrl    m_ctrlPageList;
    CButton    m_ctrlPageFrame;

protected:

    virtual void DoDataExchange(CDataExchange * pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK();

    afx_msg void OnItemchangedPageList(NMHDR * pNMHDR, LRESULT * pResult);
    afx_msg void OnDestroy();
    DECLARE_MESSAGE_MAP()

    CArray<OPTIONS_PAGE *, OPTIONS_PAGE *> m_aryPages;

    BOOL UpdatePage();
    int GetSelectedPage();
};
