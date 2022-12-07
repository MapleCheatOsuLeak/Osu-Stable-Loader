#pragma once
#include "MilkMemory.h"

class MilkThread
{
	MilkMemory _milkMemory;
	uintptr_t _function;
	HANDLE _processHandle;

	uint32_t* _codeCaveLocation;
	bool _codeCavePrepared;

	void prepareCodeCave();
public:
	MilkThread(uintptr_t function, HANDLE processHandle, bool lazy = false);
	~MilkThread();

	HANDLE Start();

	void CleanCodeCave();
	void SetFunctionPointer(uintptr_t function);
};