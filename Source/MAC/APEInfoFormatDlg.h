#pragma once
#include "afxwin.h"
#include "afxcmn.h"

class CAPEInfoFormatDlg : public CDialog
{
    DECLARE_DYNAMIC(CAPEInfoFormatDlg)

public:

    CAPEInfoFormatDlg(CWnd* pParent = NULL);
    virtual ~CAPEInfoFormatDlg();

    BOOL SetFiles(CStringArray & aryFiles);

    enum { IDD = IDD_APE_INFO_FORMAT };

protected:

    virtual void DoDataExchange(CDataExchange* pDX);
    DECLARE_MESSAGE_MAP()

    CString GetSummary(const CString & strFilename);
    CStringArray m_aryFiles;
public:
    CRichEditCtrl m_ctrlFormat;
};
