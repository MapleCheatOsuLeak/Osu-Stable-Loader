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
	VM_LION_BLACK_START
	DWORD oldProtection;
	VirtualProtectEx(_processHandle, _codeCaveLocation, 5, PAGE_EXECUTE_READWRITE, &oldProtection);
	const uintptr_t jumpLocation = _function - 5 - reinterpret_cast<uintptr_t>(_codeCaveLocation);
	unsigned char jmp[1] { 0xE9 };
	WriteProcessMemory(_processHandle, _codeCaveLocation, jmp, sizeof jmp, nullptr);
	WriteProcessMemory(_processHandle, _codeCaveLocation + 1, &jumpLocation, sizeof uintptr_t, nullptr);
	VirtualProtectEx(_processHandle, _codeCaveLocation, 5, oldProtection, &oldProtection);

	_codeCavePrepared = true;
	VM_LION_BLACK_END
}

void MilkThread::cleanCodeCave()
{
	VM_LION_BLACK_START
	DWORD oldProtection;
	VirtualProtectEx(_processHandle, _codeCaveLocation, 5, PAGE_EXECUTE_READWRITE, &oldProtection);
	unsigned char alignment[5] { 0xCC,0xCC,0xCC,0xCC,0xCC };
	WriteProcessMemory(_processHandle, _codeCaveLocation, &alignment, sizeof alignment, nullptr);
	VirtualProtectEx(_processHandle, _codeCaveLocation, 5, oldProtection, &oldProtection);

	_codeCavePrepared = false;
	VM_LION_BLACK_END
}

HANDLE MilkThread::Start()
{
	VM_LION_BLACK_START
	if (_codeCaveLocation == nullptr)
		return nullptr;

	if (!_codeCavePrepared)
		prepareCodeCave();

	auto ret = CreateRemoteThread(_processHandle, nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(_codeCaveLocation),
		nullptr, NULL, nullptr);
	return ret;
	VM_LION_BLACK_END
}

void MilkThread::SetFunctionPointer(uintptr_t function)
{
	VM_LION_BLACK_START
	_function = function;
	cleanCodeCave();
	VM_LION_BLACK_END
}