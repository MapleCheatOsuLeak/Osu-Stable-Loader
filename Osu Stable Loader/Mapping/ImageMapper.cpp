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

InsertInvertedFunctionTableResult ImageMapper::insertInvertedFunctionTable(int ldrpInvertedFunctionTablesOffset, int rtlInsertInvertedFunctionTableOffset, std::vector<unsigned char> headers)
{
	int ldrpInvertedFunctionTables = reinterpret_cast<uintptr_t>(MemoryUtilities::GetRemoteModuleHandleA(processHandle, "ntdll.dll")) + ldrpInvertedFunctionTablesOffset;

	int rtlInsertInvertedFunctionTable = reinterpret_cast<uintptr_t>(MemoryUtilities::GetRemoteModuleHandleA(processHandle, "ntdll.dll")) + rtlInsertInvertedFunctionTableOffset;

	// Read PE Header
	PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(headers.data());
	if (dosHeader->e_magic != 0x5a4d)
		return InsertInvertedFunctionTableResult::Failed;
	PIMAGE_NT_HEADERS32 ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS32>(headers.data() + dosHeader->e_lfanew);

	auto table = _RTL_INVERTED_FUNCTION_TABLE8<DWORD>();
	ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(ldrpInvertedFunctionTables), &table, sizeof(_RTL_INVERTED_FUNCTION_TABLE8<DWORD>), NULL);
	for (size_t i = 0; i < table.Count; i++)
		if (table.Entries[i].ImageBase == reinterpret_cast<DWORD>(imageBaseAddress))
			return InsertInvertedFunctionTableResult::Success;

	MemoryUtilities::CallRemoteFunctionFastcall(processHandle, rtlInsertInvertedFunctionTable, { reinterpret_cast<int>(imageBaseAddress), static_cast<int>(ntHeaders->OptionalHeader.SizeOfImage) });
	ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(ldrpInvertedFunctionTables), &table, sizeof(_RTL_INVERTED_FUNCTION_TABLE8<DWORD>), NULL);

	for (size_t i = 0; i < table.Count; i++)
	{
		if (table.Entries[i].ImageBase != reinterpret_cast<DWORD>(imageBaseAddress))
			continue;

		if (table.Entries[i].SizeOfTable != 0)
			return InsertInvertedFunctionTableResult::SuccessWithSEH; // maple should usually return here

		// Create fake Exception directory
		// Directory will be filled later, during exception handling
		LPVOID mem = VirtualAllocEx(processHandle, NULL, sizeof(uint32_t) * 0x800, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (!mem)
			return InsertInvertedFunctionTableResult::Failed;

		// EncodeSystemPointer(mem)
		uint32_t size = sizeof(uint32_t);
		uintptr_t encodedPointer = 0;
		uint32_t userDataCookie;
		ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(0x7FFE0000 + 0x330), &userDataCookie, sizeof(uint32_t), NULL);
		encodedPointer = _rotr(userDataCookie ^ reinterpret_cast<uint32_t>(mem), userDataCookie & 0x1F);

		// m_LdrpInvertedFunctionTable->Entries[i].ExceptionDirectory
		uintptr_t fieldOffset = reinterpret_cast<uintptr_t>(&table.Entries[i].ExceptionDirectory) - reinterpret_cast<uintptr_t>(&table);

		// In Win10 LdrpInvertedFunctionTable is located in mrdata section
		// mrdata is read-only by default 
		DWORD flOld = 0;
		VirtualProtectEx(processHandle, reinterpret_cast<LPVOID>(ldrpInvertedFunctionTables + fieldOffset), sizeof(uintptr_t), PAGE_READWRITE, &flOld);
		WriteProcessMemory(processHandle, reinterpret_cast<LPVOID>(ldrpInvertedFunctionTables + fieldOffset), &encodedPointer, sizeof(uintptr_t), NULL);
		VirtualProtectEx(processHandle, reinterpret_cast<LPVOID>(ldrpInvertedFunctionTables + fieldOffset), sizeof(uintptr_t), flOld, &flOld);

		return InsertInvertedFunctionTableResult::Success;
	}
	return InsertInvertedFunctionTableResult::Failed;
}

// TODO: this only works for x86
void ImageMapper::enableExceptions(int ldrpInvertedFunctionTablesOffset, int rtlInsertInvertedFunctionTableOffset, std::vector<unsigned char> headers)
{
	auto status = insertInvertedFunctionTable(ldrpInvertedFunctionTablesOffset, rtlInsertInvertedFunctionTableOffset, headers);

	if (status == InsertInvertedFunctionTableResult::Failed)
		return;
	if (status == InsertInvertedFunctionTableResult::SuccessWithSEH)
	{
		//printf("  [~] success with safeseh\n");
		return;
	}
	// TODO: implement setting up VEH
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

void ImageMapper::MapImage(int ldrpHandleTlsDataOffset, int ldrpInvertedFunctionTablesOffset, int rtlInsertInvertedFunctionTableOffset, int entryPointOffset, const std::vector<unsigned char>& headers, const std::vector<ImageSection>& imageSections, const std::vector<int>& tlsCallbacks)
{
	VM_FISH_RED_START

	// copying away constness
	auto nonConstHeaders = std::vector<unsigned char>(headers);

	mapHeaders(headers);
	mapSections(imageSections);
	fixStaticTLS(ldrpHandleTlsDataOffset);
	enableExceptions(ldrpInvertedFunctionTablesOffset, rtlInsertInvertedFunctionTableOffset, nonConstHeaders);
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
