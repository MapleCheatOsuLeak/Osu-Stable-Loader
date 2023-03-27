#include "ImageMapper.h"

#include <winternl.h>
#include <ThemidaSDK.h>

#include "../Utilities/Memory/MemoryUtilities.h"
#include "../UserData/UserDataManager.h"

#pragma optimize("", off)
void ImageMapper::mapHeaders(const std::vector<unsigned char>& headers)
{
	sizeOfHeaders = headers.size();
	WriteProcessMemory(processHandle, imageBaseAddress, headers.data(), headers.size(), NULL);
}

void ImageMapper::unmapHeaders()
{
	unsigned char* buffer = new unsigned char[sizeOfHeaders];
	memset(buffer, 0, sizeOfHeaders);

	WriteProcessMemory(processHandle, imageBaseAddress, &buffer, sizeOfHeaders, NULL);

	delete[] buffer;
}

void ImageMapper::mapSections(const std::vector<ImageSection>& imageSections)
{
	for (ImageSection section : imageSections)
	{
		WriteProcessMemory(processHandle, reinterpret_cast<LPVOID>(section.Address), section.Data.data(), section.Data.size(), NULL);

		DWORD oldProtect;
		VirtualProtectEx(processHandle, reinterpret_cast<LPVOID>(section.Address), section.ProtectionSize, section.Protection, &oldProtect);
	}
}

void ImageMapper::fixStaticTLS(int ldrpHandleTlsDataOffset)
{
	int ldrpHandleTlsData = reinterpret_cast<uintptr_t>(MemoryUtilities::GetRemoteModuleHandleA(processHandle, "ntdll.dll")) + ldrpHandleTlsDataOffset;
	
	LDR_DATA_TABLE_ENTRY dataTableEntry;
	memset(&dataTableEntry, 0, sizeof(dataTableEntry));
	dataTableEntry.DllBase = imageBaseAddress;

	LPVOID dataTableEntryAddress = VirtualAllocEx(processHandle, NULL, sizeof(_LDR_DATA_TABLE_ENTRY), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(processHandle, dataTableEntryAddress, &dataTableEntry, sizeof(_LDR_DATA_TABLE_ENTRY), NULL);

	MemoryUtilities::CallRemoteFunctionThiscall(processHandle, ldrpHandleTlsData, (int)dataTableEntryAddress, {});
}

void ImageMapper::callInitializationRoutines(int entryPointOffset, const std::vector<int>& tlsCallbacks)
{
	// calling tls callbacks
	
	for (auto& callbackOffset : tlsCallbacks)
		MemoryUtilities::CallRemoteFunction(processHandle, reinterpret_cast<int>(imageBaseAddress) + callbackOffset, { reinterpret_cast<int>(imageBaseAddress), DLL_PROCESS_ATTACH, 0 });

	// calling entry point

	CheatUserData userData = UserDataManager::CreateCheatUserData();

	LPVOID userDataAddress = VirtualAllocEx(processHandle, NULL, sizeof(CheatUserData), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(processHandle, userDataAddress, &userData, sizeof(CheatUserData), NULL);

	MemoryUtilities::CallRemoteFunction(processHandle, reinterpret_cast<int>(imageBaseAddress) + entryPointOffset, { reinterpret_cast<int>(imageBaseAddress), DLL_PROCESS_ATTACH, reinterpret_cast<int>(userDataAddress) });
}

void ImageMapper::Initialize(HANDLE processHandle)
{
	VM_SHARK_BLACK_START

	ImageMapper::processHandle = processHandle;

	VM_SHARK_BLACK_END
}

int ImageMapper::AllocateMemoryForImage(int imageSize)
{
	VM_SHARK_BLACK_START

	imageBaseAddress = VirtualAllocEx(processHandle, NULL, imageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	VM_SHARK_BLACK_END

	return reinterpret_cast<int>(imageBaseAddress);
}

std::vector<ImageResolvedImport> ImageMapper::ResolveImports(const std::vector<ImageImport>& imports)
{
	VM_TIGER_WHITE_START

	std::vector<ImageResolvedImport> resolvedImports;

	for (const ImageImport& import : imports)
	{
		ImageResolvedImport resolvedImport = ImageResolvedImport();
		resolvedImport.DescriptorName = import.DescriptorName;
		resolvedImport.FunctionNameOrOrdinal = import.FunctionNameOrOrdinal;
		resolvedImport.FunctionAddress = MemoryUtilities::GetRemoteProcAddress(processHandle, import.DescriptorName.c_str(), import.FunctionNameOrOrdinal.c_str());

		resolvedImports.push_back(resolvedImport);
	}

	VM_TIGER_WHITE_END

	return resolvedImports;
}

void ImageMapper::MapImage(int ldrpHandleTlsDataOffset, int entryPointOffset, const std::vector<unsigned char>& headers, const std::vector<ImageSection>& imageSections, const std::vector<int>& tlsCallbacks)
{
	VM_FISH_RED_START

	mapHeaders(headers);
	mapSections(imageSections);
	fixStaticTLS(ldrpHandleTlsDataOffset);
	callInitializationRoutines(entryPointOffset, tlsCallbacks);
	unmapHeaders();

	VM_FISH_RED_END
}

void ImageMapper::Finish()
{
	VM_SHARK_BLACK_START

	CloseHandle(processHandle);

	VM_SHARK_BLACK_END
}
#pragma optimize("", on)
