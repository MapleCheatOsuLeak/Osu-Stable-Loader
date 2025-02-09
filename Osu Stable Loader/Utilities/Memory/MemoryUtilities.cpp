#include "MemoryUtilities.h"

#include <cstdlib>
#include <TlHelp32.h>

#include "../Security/xorstr.hpp"

std::vector<char> MemoryUtilities::IntToByteArray(int value)
{
    std::vector<char> result(4);

    for (int i = 0; i < 4; i++)
        result[i] = (value >> (i * 8));

    return result;
}

int MemoryUtilities::CallRemoteFunction(HANDLE processHandle, int functionAddress, std::vector<int> arguments)
{
    // push arguments
    std::vector<char> shellcode;
    for (int i = arguments.size() - 1; i >= 0; i--)
    {
        if (arguments[i] >= -128 && arguments[i] <= 127)
        {
            shellcode.push_back(0x6A);
            shellcode.push_back(static_cast<char>(arguments[i]));
        }
        else
        {
            shellcode.push_back(0x68);
            std::vector<char> bytes = IntToByteArray(arguments[i]);
            shellcode.insert(shellcode.end(), bytes.begin(), bytes.end());
        }
    }

    // mov eax, functionAddress
    std::vector<char> functionAddressBytes = IntToByteArray(functionAddress);
    shellcode.push_back(0xB8);
    shellcode.insert(shellcode.end(), functionAddressBytes.begin(), functionAddressBytes.end());

    // call eax
    shellcode.push_back(0xFF);
    shellcode.push_back(0xD0);

    // mov [resultPointer], eax
    LPVOID resultPointer = VirtualAllocEx(processHandle, NULL, sizeof(int), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!resultPointer)
        return 0;

    std::vector<char> resultPointerBytes = IntToByteArray(reinterpret_cast<int>(resultPointer));
    shellcode.push_back(0xA3);
    shellcode.insert(shellcode.end(), resultPointerBytes.begin(), resultPointerBytes.end());

    // xor eax, eax
    shellcode.push_back(0x31);
    shellcode.push_back(0xC0);

    //ret
    shellcode.push_back(0xC3);

    LPVOID shellcodeAddress = VirtualAllocEx(processHandle, NULL, shellcode.size(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READ);
    if (!shellcodeAddress)
        return 0;

    if (!WriteProcessMemory(processHandle, shellcodeAddress, shellcode.data(), shellcode.size(), NULL))
    {
        VirtualFreeEx(processHandle, resultPointer, sizeof(int), MEM_RELEASE);
        VirtualFreeEx(processHandle, shellcodeAddress, shellcode.size(), MEM_RELEASE);

        return 0;
    }

    if (MilkThread == nullptr)
        MilkThread = new ::MilkThread(reinterpret_cast<uintptr_t>(shellcodeAddress), processHandle, true);

    MilkThread->SetFunctionPointer(reinterpret_cast<uintptr_t>(shellcodeAddress));
    HANDLE spawnedThread = MilkThread->Start();

    if (spawnedThread == nullptr)
        return 0;

    WaitForSingleObject(spawnedThread, INFINITE);
	
    int result;
    ReadProcessMemory(processHandle, resultPointer, &result, sizeof(int), NULL);

    VirtualFreeEx(processHandle, resultPointer, sizeof(int), MEM_RELEASE);
	VirtualFreeEx(processHandle, shellcodeAddress, shellcode.size(), MEM_RELEASE);

    return result;
}

int MemoryUtilities::CallRemoteFunctionThiscall(HANDLE processHandle, int functionAddress, int instance, std::vector<int> arguments)
{
    std::vector<char> shellcode;
	
    // move instance to ecx (mov ecx, instance)
    shellcode.push_back(0xB9);
    std::vector<char> bytes = IntToByteArray(instance);
    shellcode.insert(shellcode.end(), bytes.begin(), bytes.end());

    // push arguments (push arg)
    for (int i = arguments.size() - 1; i >= 0; i--)
    {
        if (arguments[i] >= -128 && arguments[i] <= 127)
        {
            // push byte
            shellcode.push_back(0x6A);
            shellcode.push_back(static_cast<char>(arguments[i]));
        }
        else
        {
			// push dword
            shellcode.push_back(0x68);
            std::vector<char> bytes = IntToByteArray(arguments[i]);
            shellcode.insert(shellcode.end(), bytes.begin(), bytes.end());
        }
    }

    // move function address to eax (mov eax, functionAddress)
    std::vector<char> functionAddressBytes = IntToByteArray(functionAddress);
    shellcode.push_back(0xB8);
    shellcode.insert(shellcode.end(), functionAddressBytes.begin(), functionAddressBytes.end());

    // call function (call eax)
    shellcode.push_back(0xFF);
    shellcode.push_back(0xD0);

    // mov [resultPointer], eax
    LPVOID resultPointer = VirtualAllocEx(processHandle, NULL, sizeof(int), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!resultPointer)
        return 0;

	// move eax to resultPointer (mov [resultPointer], eax)
    std::vector<char> resultPointerBytes = IntToByteArray(reinterpret_cast<int>(resultPointer));
    shellcode.push_back(0xA3);
    shellcode.insert(shellcode.end(), resultPointerBytes.begin(), resultPointerBytes.end());

	// clear eax (xor eax, eax)
    shellcode.push_back(0x31);
    shellcode.push_back(0xC0);

    // return (ret)
    shellcode.push_back(0xC3);

    LPVOID shellcodeAddress = VirtualAllocEx(processHandle, NULL, shellcode.size(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READ);
    if (!shellcodeAddress)
        return 0;

    if (!WriteProcessMemory(processHandle, shellcodeAddress, shellcode.data(), shellcode.size(), NULL))
    {
        VirtualFreeEx(processHandle, resultPointer, sizeof(int), MEM_RELEASE);
        VirtualFreeEx(processHandle, shellcodeAddress, shellcode.size(), MEM_RELEASE);

        return 0;
    }

    if (MilkThread == nullptr)
        MilkThread = new ::MilkThread(reinterpret_cast<uintptr_t>(shellcodeAddress), processHandle, true);

    MilkThread->SetFunctionPointer(reinterpret_cast<uintptr_t>(shellcodeAddress));
    HANDLE spawnedThread = MilkThread->Start();

    if (spawnedThread == nullptr)
        return 0;

    WaitForSingleObject(spawnedThread, INFINITE);

    int result;
    ReadProcessMemory(processHandle, resultPointer, &result, sizeof(int), NULL);

    VirtualFreeEx(processHandle, resultPointer, sizeof(int), MEM_RELEASE);
    VirtualFreeEx(processHandle, shellcodeAddress, shellcode.size(), MEM_RELEASE);

    return result;
}

int MemoryUtilities::CallRemoteFunctionFastcall(HANDLE processHandle, int functionAddress, std::vector<int> arguments)
{
    std::vector<char> shellcode;

    // push arguments (push arg | mov ecx, arg | mov edx, arg)
    for (int i = arguments.size() - 1; i >= 0; i--)
    {
        if (i == 0)
        {
            // mov ecx, arguments[0]
            shellcode.push_back(0xB9);
            std::vector<char> bytes = IntToByteArray(arguments[i]);
            shellcode.insert(shellcode.end(), bytes.begin(), bytes.end());
        }
        else if (i == 1)
        {
            // mov edx, arguments[1]
            shellcode.push_back(0xBA);
            std::vector<char> bytes2 = IntToByteArray(arguments[i]);
            shellcode.insert(shellcode.end(), bytes2.begin(), bytes2.end());
        }
        else
        {
            if (arguments[i] >= -128 && arguments[i] <= 127)
            {
                // push byte
                shellcode.push_back(0x6A);
                shellcode.push_back(static_cast<char>(arguments[i]));
            }
            else
            {
                // push dword
                shellcode.push_back(0x68);
                std::vector<char> bytes = IntToByteArray(arguments[i]);
                shellcode.insert(shellcode.end(), bytes.begin(), bytes.end());
            }
        }
    }

    // move function address to eax (mov eax, functionAddress)
    std::vector<char> functionAddressBytes = IntToByteArray(functionAddress);
    shellcode.push_back(0xB8);
    shellcode.insert(shellcode.end(), functionAddressBytes.begin(), functionAddressBytes.end());

    // call function (call eax)
    shellcode.push_back(0xFF);
    shellcode.push_back(0xD0);

    // mov [resultPointer], eax
    LPVOID resultPointer = VirtualAllocEx(processHandle, NULL, sizeof(int), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!resultPointer)
        return 0;

    // move eax to resultPointer (mov [resultPointer], eax)
    std::vector<char> resultPointerBytes = IntToByteArray(reinterpret_cast<int>(resultPointer));
    shellcode.push_back(0xA3);
    shellcode.insert(shellcode.end(), resultPointerBytes.begin(), resultPointerBytes.end());

    // clear eax (xor eax, eax)
    shellcode.push_back(0x31);
    shellcode.push_back(0xC0);

    // return (ret)
    shellcode.push_back(0xC3);

    LPVOID shellcodeAddress = VirtualAllocEx(processHandle, NULL, shellcode.size(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READ);
    if (!shellcodeAddress)
        return 0;

    if (!WriteProcessMemory(processHandle, shellcodeAddress, shellcode.data(), shellcode.size(), NULL))
    {
        VirtualFreeEx(processHandle, resultPointer, sizeof(int), MEM_RELEASE);
        VirtualFreeEx(processHandle, shellcodeAddress, shellcode.size(), MEM_RELEASE);

        return 0;
    }

    if (MilkThread == nullptr)
        MilkThread = new ::MilkThread(reinterpret_cast<uintptr_t>(shellcodeAddress), processHandle, true);

    MilkThread->SetFunctionPointer(reinterpret_cast<uintptr_t>(shellcodeAddress));
    HANDLE spawnedThread = MilkThread->Start();

    if (spawnedThread == nullptr)
        return 0;

    WaitForSingleObject(spawnedThread, INFINITE);

    int result;
    ReadProcessMemory(processHandle, resultPointer, &result, sizeof(int), NULL);

    VirtualFreeEx(processHandle, resultPointer, sizeof(int), MEM_RELEASE);
    VirtualFreeEx(processHandle, shellcodeAddress, shellcode.size(), MEM_RELEASE);

    return result;
}

DWORD MemoryUtilities::GetProcessIDByName(const char* processName)
{
    PROCESSENTRY32 processInfo;
    processInfo.dwSize = sizeof processInfo;

    HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (processSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    Process32First(processSnapshot, &processInfo);
    if (strcmp(processName, processInfo.szExeFile) == 0)
    {
        CloseHandle(processSnapshot);
        return processInfo.th32ProcessID;
    }

    while (Process32Next(processSnapshot, &processInfo))
    {
        if (strcmp(processName, processInfo.szExeFile) == 0)
        {
            CloseHandle(processSnapshot);
            return processInfo.th32ProcessID;
        }
    }

    CloseHandle(processSnapshot);

    return 0;
}

HMODULE MemoryUtilities::GetRemoteModuleHandleA(HANDLE processHandle, const char* moduleName)
{
    HANDLE tlh = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetProcessId(processHandle));

    MODULEENTRY32 modEntry;

    modEntry.dwSize = sizeof(MODULEENTRY32);

    Module32First(tlh, &modEntry);
    do
    {
        if (_stricmp(moduleName, modEntry.szModule) == 0)
        {
            CloseHandle(tlh);

            return modEntry.hModule;
        }
    } while (Module32Next(tlh, &modEntry));

    CloseHandle(tlh);

    return NULL;
}

HMODULE MemoryUtilities::RemoteLoadLibraryA(HANDLE processHandle, const char* libraryPath)
{
    if (!libraryPath)
        return NULL;

    const int loadLibraryARemote = GetRemoteProcAddress(processHandle, xorstr_("Kernel32.dll"), xorstr_("LoadLibraryA"));
    if (!loadLibraryARemote)
        return NULL;
	
    void* libraryPathAddress = VirtualAllocEx(processHandle, NULL, strlen(libraryPath) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!libraryPathAddress)
        return NULL;

    if (!WriteProcessMemory(processHandle, libraryPathAddress, libraryPath, strlen(libraryPath) + 1, NULL))
    {
        VirtualFreeEx(processHandle, libraryPathAddress, strlen(libraryPath) + 1, MEM_RELEASE);

        return NULL;
    }
	
    const HMODULE result = reinterpret_cast<HMODULE>(CallRemoteFunction(processHandle, loadLibraryARemote, {reinterpret_cast<int>(libraryPathAddress)}));
	
    VirtualFreeEx(processHandle, libraryPathAddress, strlen(libraryPath) + 1, MEM_RELEASE);

    return result;
}

int MemoryUtilities::GetRemoteProcAddress(HANDLE processHandle, const char* moduleName, const char* procName)
{
    if (CachedLocalKernel == nullptr) {
        HMODULE localKernel = GetModuleHandleA(xorstr_("Kernel32.dll"));
        if (!localKernel)
            return NULL;
        CachedLocalKernel = localKernel;
    }
	
    if (CachedRemoveKernel == nullptr)
    {
        HMODULE remoteKernel = GetRemoteModuleHandleA(processHandle, xorstr_("Kernel32.dll"));
        if (!remoteKernel)
            return NULL;
        CachedRemoveKernel = remoteKernel;
    }

    const int remoteGetProcAddress = reinterpret_cast<int>(CachedRemoveKernel) + reinterpret_cast<int>(GetProcAddress) - reinterpret_cast<int>(CachedLocalKernel);
	
    void* remoteProcName = nullptr;
    char* p;
    long ordinal = strtol(procName, &p, 10);
    bool isOrdinal = false;
    if (*p)
    {
        remoteProcName = VirtualAllocEx(processHandle, NULL, strlen(procName) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (!remoteProcName)
            return NULL;

        if (!WriteProcessMemory(processHandle, remoteProcName, procName, strlen(procName) + 1, NULL))
        {
            VirtualFreeEx(processHandle, remoteProcName, strlen(procName) + 1, MEM_RELEASE);

            return NULL;
        }
    }
    else isOrdinal = true;

    HMODULE remoteModule = GetRemoteModuleHandleA(processHandle, moduleName);
    if (!remoteModule)
        remoteModule = RemoteLoadLibraryA(processHandle, moduleName);

    if ((!remoteProcName && !ordinal) || !remoteModule)
        return NULL;

    const int result = CallRemoteFunction(processHandle, remoteGetProcAddress, { reinterpret_cast<int>(remoteModule), isOrdinal ? ordinal : reinterpret_cast<int>(remoteProcName) });

    if (remoteProcName)
        VirtualFreeEx(processHandle, remoteProcName, strlen(procName) + 1, MEM_RELEASE);

    return result;
}
