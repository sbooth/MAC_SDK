#pragma once

#define ID_SET_COMPRESSION_FIRST    40000
#define ID_SET_COMPRESSION_LAST        50000

class MAC_FILE;

class IFormat
{
public:

    virtual ~IFormat() { }
    
    virtual CString GetName() { return _T(""); }

    virtual int Process(MAC_FILE * pInfo) { return ERROR_UNDEFINED; }

    virtual BOOL BuildMenu(CMenu * pMenu, int nBaseID) { return FALSE; }
    virtual BOOL ProcessMenuCommand(int nCommand) { return FALSE; }

    virtual CString GetInputExtensions(MAC_MODES Mode) { return _T(""); }
    virtual CString GetOutputExtension(MAC_MODES Mode, const CString & strInputFilename, int nLevel) { return _T(""); }
};