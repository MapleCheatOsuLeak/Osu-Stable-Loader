#pragma once

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <vector>
#include "../../Dependencies/Milk/MilkThread.h"

class MemoryUtilities
{
public:
    static inline MilkThread* MilkThread = nullptr;

	static __forceinline void AdjustPrivileges()
	{
        HANDLE token;
        TOKEN_PRIVILEGES tp;
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        tp.Privileges[0].Luid.LowPart = 20; // 20 = SeDebugPrivilege
        tp.Privileges[0].Luid.HighPart = 0;

        if (OpenProcessToken((HANDLE)-1, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
        {
            AdjustTokenPrivileges(token, FALSE, &tp, 0, NULL, 0);
            CloseHandle(token);
        }
    }

    static std::vector<char> IntToByteArray(int value);
    static int CallRemoteFunction(HANDLE processHandle, int functionAddress, std::vector<int> arguments);
	static DWORD GetProcessIDByName(const char* processName);
	static HMODULE GetRemoteModuleHandleA(HANDLE processHandle, const char* moduleName);
	static HMODULE RemoteLoadLibraryA(HANDLE processHandle, const char* libraryPath);
	static int GetRemoteProcAddress(HANDLE processHandle, const char* moduleName, const char* procName);
};