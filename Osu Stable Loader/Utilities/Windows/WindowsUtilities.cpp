#include "WindowsUtilities.h"

std::string WindowsUtilities::GetWindowsFolderPath()
{
	TCHAR windir[MAX_PATH];
	GetWindowsDirectory(windir, MAX_PATH);

	return std::string(windir);
}

// Returns true when called on a 64bit os.
BOOL WindowsUtilities::Is64BitOS()
{
    BOOL bIs64BitOS = FALSE;

    typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

    LPFN_ISWOW64PROCESS
        fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
            GetModuleHandle(xorstr_("kernel32")), xorstr_("IsWow64Process"));

    if (NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(), &bIs64BitOS))
        {
            //error
        }
    }
    return bIs64BitOS;
}

std::string WindowsUtilities::GetNtdllPath()
{
    BOOL is64BitOS = Is64BitOS();
    if (is64BitOS)
        return GetWindowsFolderPath() + xorstr_("\\SysWOW64\\ntdll.dll");
    
    return GetWindowsFolderPath() + xorstr_("\\ntdll.dll");
}