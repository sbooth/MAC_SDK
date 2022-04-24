#pragma once

#include "resource.h"
class CFormatArray;
class CMACSettings;

class CMACApp : public CWinApp
{
public:
    // construction / destruction
    CMACApp();
    virtual ~CMACApp();

    // initialize
    virtual BOOL InitInstance();

    // data access
    CFormatArray * GetFormatArray();
    CMACSettings * GetSettings();
    enum EImageList
    {
        Image_Toolbar,
        Image_OptionsList,
        Image_OptionsPages
    };
    CImageList * GetImageList(EImageList Image);
    CSize GetSize(int x, int y, double dAdditional = 1.0);
    int GetSizeReverse(int nSize);

    // message map
    DECLARE_MESSAGE_MAP()

private:
    // helper objects
    CSmartPtr<CFormatArray> m_sparyFormats;
    CSmartPtr<CMACSettings> m_spSettings;
    CSmartPtr<CImageList> m_spImageListToolbar;
    CSmartPtr<CImageList> m_spImageListOptionsList;
    CSmartPtr<CImageList> m_spImageListOptionsPages;
    double m_dScale;
    HANDLE m_hSingleInstance;
    bool m_bAnotherInstanceRunning;
};

extern CMACApp theApp;
