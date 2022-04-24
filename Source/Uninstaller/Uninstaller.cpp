#include "All.h"

/**************************************************************************************************
Main (the main function)
**************************************************************************************************/
int _tmain(int argc, TCHAR* argv[])
{
    TCHAR cPath[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, cPath, MAX_PATH);

    TCHAR cDrive[_MAX_DRIVE], cDir[_MAX_DIR], cName[_MAX_FNAME], cExt[_MAX_EXT];
    cDrive[0] = 0; cDir[0] = 0; cName[0] = 0; cExt[0] = 0;
    _tsplitpath_s(cPath, cDrive, _MAX_DRIVE, cDir, _MAX_DIR, cName, _MAX_FNAME, cExt, _MAX_EXT);

    TCHAR cUninstall[MAX_PATH] = { 0 };
    _tcscat_s(cUninstall, MAX_PATH, cDrive);
    _tcscat_s(cUninstall, MAX_PATH, cDir);
    _tcscat_s(cUninstall, MAX_PATH, _T("uninstall.exe"));

    ShellExecute(NULL, NULL, cUninstall, NULL, NULL, SW_SHOW);

    return 0;
}
