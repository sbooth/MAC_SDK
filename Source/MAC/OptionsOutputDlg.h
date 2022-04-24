#pragma once

class COptionsOutputDlg : public CDialog
{
public:

    COptionsOutputDlg(CWnd * pParent = NULL);

    enum { IDD = IDD_OPTIONS_OUTPUT };
    CStatic    m_ctrlOtherPicture;
    CComboBox    m_ctrlDeleteAfterSuccessCombo;
    CStatic    m_ctrlBehaviorPicture;
    CComboBox    m_ctrlOutputExistsCombo;
    CStatic    m_ctrlOutputLocationRecreateDirectoryStructureStatic;
    CEdit    m_ctrlOutputLocationRecreateDirectoryStructureEdit;
    CButton    m_ctrlOutputLocationRecreateDirectoryStructureCheck;
    CComboBox    m_ctrlOutputLocationDirectoryCombo;
    CButton    m_ctrlOutputLocationDirectoryBrowse;
    CStatic    m_ctrlOutputLocationPicture;
    CString    m_strAPLFilenameTemplate;
    BOOL    m_bMirrorTimeStamp;

protected:

    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    afx_msg void OnDestroy();
    afx_msg void OnAplFilenameTemplateHelp();
    afx_msg void OnOutputLocationDirectoryBrowse();
    afx_msg void OnOutputLocationSpecifiedDirectory();
    afx_msg void OnOutputLocationSameDirectory();
    afx_msg void OnOutputLocationRecreateDirectoryStructureCheck();
    afx_msg void OnOK();
    afx_msg void OnCancel();
    afx_msg LRESULT OnSaveOptions(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

    CArray<HICON, HICON> m_aryIcons;
    void UpdateDialogState();
};
