#include "MilkThread.h"

#include <string>
#include "WinUser.h"
#include <ThemidaSDK.h>

#pragma optimize("", off)

MilkThread::MilkThread(uintptr_t function, HANDLE processHandle, bool lazy) : _milkMemory(processHandle)
{
	VM_FISH_RED_START
		_function = function;
	_processHandle = processHandle;

	_codeCaveLocation = _milkMemory.FindCodeCave();

	if (!lazy)
		Start();
	VM_FISH_RED_END
}

MilkThread::~MilkThread()
{
	VM_FISH_RED_START
	_codeCaveLocation = nullptr;
	_function = NULL;
	_milkMemory.~MilkMemory();
	VM_FISH_RED_END
}

void MilkThread::prepareCodeCave()
{
	VM_FISH_WHITE_START
	DWORD oldProtection;
	VirtualProtectEx(_processHandle, _codeCaveLocation, 6, PAGE_EXECUTE_READWRITE, &oldProtection);
	const uintptr_t jumpLocation = _function;
	unsigned char push[1] { 0x68 };
    unsigned char ret[1] { 0xC3 };
	WriteProcessMemory(_processHandle, _codeCaveLocation, push, sizeof push, nullptr);
	WriteProcessMemory(_processHandle, reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(_codeCaveLocation) + 1), &jumpLocation, sizeof uintptr_t, nullptr);
	WriteProcessMemory(_processHandle, reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(_codeCaveLocation) + 5), ret, sizeof ret, nullptr);
	VirtualProtectEx(_processHandle, _codeCaveLocation, 6, oldProtection, &oldProtection);

	_codeCavePrepared = true;
	VM_FISH_WHITE_END
}

void MilkThread::CleanCodeCave()
{
	VM_FISH_WHITE_START
	DWORD oldProtection;
	VirtualProtectEx(_processHandle, _codeCaveLocation, 6, PAGE_EXECUTE_READWRITE, &oldProtection);
	unsigned char alignment[6] { 0xCC,0xCC,0xCC,0xCC,0xCC,0xCC };
	WriteProcessMemory(_processHandle, _codeCaveLocation, &alignment, sizeof alignment, nullptr);
	VirtualProtectEx(_processHandle, _codeCaveLocation, 6, oldProtection, &oldProtection);

	_codeCavePrepared = false;
	VM_FISH_WHITE_END
}

HANDLE MilkThread::Start()
{
	VM_LION_WHITE_START
	if (_codeCaveLocation == nullptr)
		return nullptr;

	if (!_codeCavePrepared)
		prepareCodeCave();

	auto ret = CreateRemoteThread(_processHandle, nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(_codeCaveLocation),
		nullptr, NULL, nullptr);
	return ret;
	VM_LION_WHITE_END
}

void MilkThread::SetFunctionPointer(uintptr_t function)
{
	VM_FISH_WHITE_START
	_function = function;
	CleanCodeCave();
	VM_FISH_WHITE_END
}