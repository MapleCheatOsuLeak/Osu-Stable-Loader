#include "MilkMemory.h"

#include <ThemidaSDK.h>
//#pragma optimize("", off)

MilkMemory::MilkMemory(HANDLE processHandle)
{
	VM_FISH_RED_START
	_processHandle = processHandle;
	_memoryRegions = std::vector<MemoryRegion>();
	cacheMemoryRegions();

	//CODE_CAVE_SEARCH_OFFSET = reinterpret_cast<uint32_t>(GetModuleHandleExA(0, "kernel32.dll", _processHandle)) - 0x10000000;

	VM_FISH_RED_END
}

MilkMemory::~MilkMemory()
{
	VM_FISH_RED_START
	_memoryRegions.clear();
	_memoryRegions.shrink_to_fit();
	VM_FISH_RED_END
}

void MilkMemory::cacheMemoryRegions()
{
	VM_MUTATE_ONLY_START
		_memoryRegions.clear();

	MEMORY_BASIC_INFORMATION32 mbi{};
	LPVOID address = nullptr;
	bool currentRegionInvalid = false;
	while (VirtualQueryEx(_processHandle, address, reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&mbi), sizeof mbi) != 0)
	{
		address = reinterpret_cast<LPVOID>(mbi.BaseAddress + mbi.RegionSize);
		if (mbi.BaseAddress == NULL)
			continue;

		// Search for only PE Headers in memory
		if (mbi.AllocationProtect == PAGE_EXECUTE_WRITECOPY &&
			mbi.State == MEM_COMMIT &&
			mbi.Protect == PAGE_READONLY &&
			mbi.Type == MEM_IMAGE &&
			mbi.RegionSize == 0x1000)
		{
			// Read the entire section into a buffer
			auto buffer = std::vector<uint8_t>(mbi.RegionSize);
			const auto address = reinterpret_cast<uint8_t*>(mbi.BaseAddress);
			SIZE_T readBytes;
			ReadProcessMemory(_processHandle, address, buffer.data(), mbi.RegionSize, &readBytes);

			PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(buffer.data());
			if (dosHeader->e_magic != 0x5a4d)
				continue;
			PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(buffer.data() + dosHeader->e_lfanew);

			// The current region has CFG enabled and therefore should not be checked for code caves.
			// We also check if the region is part of a COM module, if it is we also skip that
			if (ntHeaders->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_GUARD_CF ||
				ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress != NULL)
				currentRegionInvalid = true;

			buffer.clear();
		}

		if (currentRegionInvalid)
		{
			// If we've found empty space within loaded modules, reset status.
			if (mbi.State == MEM_FREE && mbi.Protect == PAGE_NOACCESS)
				currentRegionInvalid = false;
		}
		else
			if (mbi.Protect >= PAGE_READONLY && mbi.Protect <= PAGE_EXECUTE_WRITECOPY)
				_memoryRegions.emplace_back(mbi);
	}
	VM_MUTATE_ONLY_END
}

__forceinline std::vector<uint8_t> MilkMemory::ReadMemory(uint32_t startAddress, SIZE_T size)
{
	auto buffer = std::vector<uint8_t>(size);
	const auto address = reinterpret_cast<uint8_t*>(startAddress);
	SIZE_T readBytes;
	ReadProcessMemory(_processHandle, address, buffer.data(), size, &readBytes);
	if (readBytes == NULL)
		return { };

	return buffer;
}

std::vector<MemoryRegion>* MilkMemory::GetMemoryRegions()
{
	VM_LION_BLACK_START
	return &_memoryRegions;
	VM_LION_BLACK_END
}

uint32_t* MilkMemory::FindCodeCave()
{
	VM_MUTATE_ONLY_START
		for (auto const& region : _memoryRegions)
		{
			if ((region.State & MEM_COMMIT) && (region.Protect & PAGE_EXECUTE_READ) &&
				(region.Type & MEM_IMAGE) && region.RegionSize >= CODE_CAVE_MINIMUM_REGIONSIZE &&
				!(region.AllocationProtect & PAGE_READONLY))
			{
				uint32_t readingAddress = region.BaseAddress;
				while (readingAddress < region.BaseAddress + region.RegionSize)
				{
					auto memoryBuffer = ReadMemory(readingAddress, 1024);

					int occurrences = 0;
					for (size_t i = 0; i < memoryBuffer.size(); i++)
					{
						auto const& byte = memoryBuffer[i];
						if (byte == 0xCC)
							occurrences++;
						if (occurrences == CODE_CAVE_MINIMUM_SIZE) {
							uint32_t* addr = reinterpret_cast<uint32_t*>(readingAddress += i - (CODE_CAVE_MINIMUM_SIZE - 1));
							return addr;
						}
						if (occurrences > 0 && byte != 0xCC)
							occurrences = 0;
					}

					readingAddress += memoryBuffer.size();
				}
			}
		}
	VM_MUTATE_ONLY_END
		return nullptr;
}