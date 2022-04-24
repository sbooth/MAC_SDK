#include "stdafx.h"
#include "MAC.h"
#include "OptionsOutputDlg.h"
#include "OptionsShared.h"
#include "FolderDialog.h"
#include "APEButtons.h"

COptionsOutputDlg::COptionsOutputDlg(CWnd * pParent)
    : CDialog(COptionsOutputDlg::IDD, pParent)
{
    m_bMirrorTimeStamp = FALSE;
}


void COptionsOutputDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_OTHER_PICTURE, m_ctrlOtherPicture);
    DDX_Control(pDX, IDC_DELETE_AFTER_SUCCESS_COMBO, m_ctrlDeleteAfterSuccessCombo);
    DDX_Control(pDX, IDC_BEHAVIOR_PICTURE, m_ctrlBehaviorPicture);
    DDX_Control(pDX, IDC_OUTPUT_EXISTS_COMBO, m_ctrlOutputExistsCombo);
    DDX_Control(pDX, IDC_OUTPUT_LOCATION_RECREATE_DIRECTORY_STRUCTURE_STATIC, m_ctrlOutputLocationRecreateDirectoryStructureStatic);
    DDX_Control(pDX, IDC_OUTPUT_LOCATION_RECREATE_DIRECTORY_STRUCTURE_LEVELS_EDIT, m_ctrlOutputLocationRecreateDirectoryStructureEdit);
    DDX_Control(pDX, IDC_OUTPUT_LOCATION_RECREATE_DIRECTORY_STRUCTURE_CHECK, m_ctrlOutputLocationRecreateDirectoryStructureCheck);
    DDX_Control(pDX, IDC_OUTPUT_LOCATION_DIRECTORY_COMBO, m_ctrlOutputLocationDirectoryCombo);
    DDX_Control(pDX, IDC_OUTPUT_LOCATION_DIRECTORY_BROWSE, m_ctrlOutputLocationDirectoryBrowse);
    DDX_Control(pDX, IDC_OUTPUT_LOCATION_PICTURE, m_ctrlOutputLocationPicture);
    DDX_CBString(pDX, IDC_APL_FILENAME_TEMPLATE_COMBO, m_strAPLFilenameTemplate);
    DDX_Check(pDX, IDC_MIRROR_TIME_STAMP_CHECK, m_bMirrorTimeStamp);
}


BEGIN_MESSAGE_MAP(COptionsOutputDlg, CDialog)
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_APL_FILENAME_TEMPLATE_HELP, OnAplFilenameTemplateHelp)
    ON_BN_CLICKED(IDC_OUTPUT_LOCATION_DIRECTORY_BROWSE, OnOutputLocationDirectoryBrowse)
    ON_BN_CLICKED(IDC_OUTPUT_LOCATION_SPECIFIED_DIRECTORY, OnOutputLocationSpecifiedDirectory)
    ON_BN_CLICKED(IDC_OUTPUT_LOCATION_SAME_DIRECTORY, OnOutputLocationSameDirectory)
    ON_BN_CLICKED(IDC_OUTPUT_LOCATION_RECREATE_DIRECTORY_STRUCTURE_CHECK, OnOutputLocationRecreateDirectoryStructureCheck)
    ON_BN_CLICKED(IDOK, OnOK)
    ON_BN_CLICKED(IDCANCEL, OnCancel)
    ON_REGISTERED_MESSAGE(UM_SAVE_PAGE_OPTIONS, OnSaveOptions)
END_MESSAGE_MAP()

BOOL COptionsOutputDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    // images
    HICON hIcon = theApp.GetImageList(CMACApp::Image_OptionsPages)->ExtractIcon(TBB_OPTIONS_OUTPUT_LOCATION);
    m_ctrlOutputLocationPicture.SetIcon(hIcon);
    m_aryIcons.Add(hIcon);

    hIcon = theApp.GetImageList(CMACApp::Image_OptionsPages)->ExtractIcon(TBB_OPTIONS_OUTPUT_BEHAVIOR);
    m_ctrlBehaviorPicture.SetIcon(hIcon);
    m_aryIcons.Add(hIcon);

    hIcon = theApp.GetImageList(CMACApp::Image_OptionsPages)->ExtractIcon(TBB_OPTIONS_OUTPUT_OTHER);
    m_ctrlOtherPicture.SetIcon(hIcon);
    m_aryIcons.Add(hIcon);

    // output location settings
    ((CButton *) GetDlgItem(IDC_OUTPUT_LOCATION_SAME_DIRECTORY + theApp.GetSettings()->m_nOutputLocationMode))->SetCheck(TRUE);
    m_ctrlOutputLocationDirectoryCombo.SetWindowText(theApp.GetSettings()->m_strOutputLocationDirectory);
    m_ctrlOutputLocationRecreateDirectoryStructureCheck.SetCheck(theApp.GetSettings()->m_bOutputLocationRecreateDirectoryStructure);
    CString strLevels; strLevels.Format(_T("%d"), theApp.GetSettings()->m_nOutputLocationRecreateDirectoryStructureLevels);
    m_ctrlOutputLocationRecreateDirectoryStructureEdit.SetWindowText(strLevels);

    // behavior
    m_ctrlOutputExistsCombo.SetCurSel(theApp.GetSettings()->m_nOutputExistsMode);
    m_ctrlDeleteAfterSuccessCombo.SetCurSel(theApp.GetSettings()->m_nOutputDeleteAfterSuccessMode);

    // other
    m_bMirrorTimeStamp = theApp.GetSettings()->m_bOutputMirrorTimeStamp;
    m_strAPLFilenameTemplate = theApp.GetSettings()->m_strAPLFilenameTemplate;

    // update
    UpdateData(FALSE);
    UpdateDialogState();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsOutputDlg::UpdateDialogState()
{
    int nOutputLocation = GetCheckedRadioButton(IDC_OUTPUT_LOCATION_SAME_DIRECTORY, IDC_OUTPUT_LOCATION_SPECIFIED_DIRECTORY) - IDC_OUTPUT_LOCATION_SAME_DIRECTORY;
    
    m_ctrlOutputLocationDirectoryCombo.EnableWindow((nOutputLocation == 1));
    m_ctrlOutputLocationDirectoryBrowse.EnableWindow((nOutputLocation == 1));
    m_ctrlOutputLocationRecreateDirectoryStructureCheck.EnableWindow((nOutputLocation == 1));
    m_ctrlOutputLocationRecreateDirectoryStructureEdit.EnableWindow((nOutputLocation == 1) &&
        m_ctrlOutputLocationRecreateDirectoryStructureCheck.GetCheck());
    m_ctrlOutputLocationRecreateDirectoryStructureStatic.EnableWindow((nOutputLocation == 1));
}

void COptionsOutputDlg::OnDestroy() 
{
    CDialog::OnDestroy();
    
    for (int z = 0; z < m_aryIcons.GetSize(); z++)
        DestroyIcon(m_aryIcons[z]);
    m_aryIcons.RemoveAll();
}

LRESULT COptionsOutputDlg::OnSaveOptions(WPARAM wParam, LPARAM lParam)
{
    UpdateData(TRUE);

    // output location
    theApp.GetSettings()->m_nOutputLocationMode = GetCheckedRadioButton(IDC_OUTPUT_LOCATION_SAME_DIRECTORY, IDC_OUTPUT_LOCATION_SPECIFIED_DIRECTORY) - IDC_OUTPUT_LOCATION_SAME_DIRECTORY;
    m_ctrlOutputLocationDirectoryCombo.GetWindowText(theApp.GetSettings()->m_strOutputLocationDirectory);
    theApp.GetSettings()->m_bOutputLocationRecreateDirectoryStructure = m_ctrlOutputLocationRecreateDirectoryStructureCheck.GetCheck();
    CString strLevels; m_ctrlOutputLocationRecreateDirectoryStructureEdit.GetWindowText(strLevels);
    theApp.GetSettings()->m_nOutputLocationRecreateDirectoryStructureLevels = cap(_ttoi(strLevels), 0, 7);

    // behavior
    theApp.GetSettings()->m_nOutputExistsMode = m_ctrlOutputExistsCombo.GetCurSel();
    theApp.GetSettings()->m_nOutputDeleteAfterSuccessMode = m_ctrlDeleteAfterSuccessCombo.GetCurSel();

    // other
    theApp.GetSettings()->m_bOutputMirrorTimeStamp = m_bMirrorTimeStamp;
    theApp.GetSettings()->m_strAPLFilenameTemplate = m_strAPLFilenameTemplate;

    return TRUE;
}

void COptionsOutputDlg::OnAplFilenameTemplateHelp() 
{
    CString strMessage;
    
    strMessage = _T("You can create filenames using any combination of letters and keywords.\r\n\r\n")
        _T("These keywords will be replaced by the corresponding values for each track:\r\n")
        _T("     ARTIST\r\n")
        _T("     ALBUM\r\n")
        _T("     TITLE\r\n")
        _T("     TRACK#\r\n")
        _T("\r\nExample:\r\n") 
        _T("     Naming template: \"ARTIST - ALBUM - TRACK# - TITLE\"\r\n")
        _T("     Resulting filename: \"Bush - Sixteen Stone - 09 - Monkey.apl\"");
    
    ::MessageBox(GetSafeHwnd(), strMessage, _T("Filename Help"), MB_OK | MB_ICONINFORMATION);
}

void COptionsOutputDlg::OnOutputLocationDirectoryBrowse() 
{
    CString strPath;
    m_ctrlOutputLocationDirectoryCombo.GetWindowText(strPath);

    CFolderDialog FolderDialog(strPath);
    if (FolderDialog.DoModal() == IDOK)
    {
        m_ctrlOutputLocationDirectoryCombo.SetWindowText(FolderDialog.GetPathName());
    }    
}

void COptionsOutputDlg::OnOutputLocationSpecifiedDirectory() 
{
    UpdateDialogState();
}

void COptionsOutputDlg::OnOutputLocationSameDirectory() 
{
    UpdateDialogState();
}

void COptionsOutputDlg::OnOutputLocationRecreateDirectoryStructureCheck() 
{
    UpdateDialogState();    
}

void COptionsOutputDlg::OnOK()
{

}

void COptionsOutputDlg::OnCancel()
{
    
}
