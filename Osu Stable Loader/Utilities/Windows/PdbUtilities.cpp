#include "PdbUtilities.h"
#include <iostream>
#include <fstream>
#include <vector>

PdbInformation PdbUtilities::GetPdbInformation(std::string modulePath, bool is32Bit)
{
	std::ifstream inFile(modulePath, std::ios_base::binary);
	inFile.seekg(0, std::ios_base::end);
	size_t length = inFile.tellg();
	inFile.seekg(0, std::ios_base::beg);

	std::vector<char> buffer;
	buffer.reserve(length);
	std::copy(std::istreambuf_iterator<char>(inFile),
		std::istreambuf_iterator<char>(),
		std::back_inserter(buffer));

	PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(buffer.data());
	PIMAGE_NT_HEADERS32 ntHeaders32 = reinterpret_cast<PIMAGE_NT_HEADERS32>(buffer.data() + dosHeader->e_lfanew);
	PIMAGE_NT_HEADERS64 ntHeaders64 = reinterpret_cast<PIMAGE_NT_HEADERS64>(buffer.data() + dosHeader->e_lfanew);

	const uintptr_t debugDirectory = is32Bit ? ntHeaders32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress :
		ntHeaders64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;

	std::unique_ptr<uint8_t[]> moduleMemory = nullptr;
	IMAGE_SECTION_HEADER* sectionsArray = nullptr;
	int sectionCount = 0;
	if (is32Bit)
	{
		moduleMemory = std::make_unique<uint8_t[]>(ntHeaders32->OptionalHeader.SizeOfImage);
		sectionsArray = reinterpret_cast<IMAGE_SECTION_HEADER*>(ntHeaders32 + 1);
		sectionCount = ntHeaders32->FileHeader.NumberOfSections;
	}
	else
	{
		moduleMemory = std::make_unique<uint8_t[]>(ntHeaders64->OptionalHeader.SizeOfImage);
		sectionsArray = reinterpret_cast<IMAGE_SECTION_HEADER*>(ntHeaders64 + 1);
		sectionCount = ntHeaders64->FileHeader.NumberOfSections;
	}

	for (int i = 0; i < sectionCount; i++)
	{
		if (sectionsArray[i].Characteristics & 0x800)
			continue;

		memcpy_s(moduleMemory.get() + sectionsArray[i].VirtualAddress, sectionsArray[i].SizeOfRawData, buffer.data() + sectionsArray[i].PointerToRawData, sectionsArray[i].SizeOfRawData);
	}

	if (debugDirectory <= NULL)
		throw std::exception(); // todo: replace with corruptmemory

	PdbInformation pdbInformation = PdbInformation();

	for (auto imgDbgDirectory = (IMAGE_DEBUG_DIRECTORY*)(moduleMemory.get() + debugDirectory); imgDbgDirectory->SizeOfData; imgDbgDirectory++)
	{
		if (imgDbgDirectory->Type != IMAGE_DEBUG_TYPE_CODEVIEW)
			continue;

		struct codeviewInfo_t
		{
			ULONG CvSignature;
			GUID Signature;
			ULONG Age;
			char PdbFileName[ANYSIZE_ARRAY];
		};

		const auto codeviewInfo = (codeviewInfo_t*)(buffer.data() + imgDbgDirectory->PointerToRawData);

		std::string pdbFileName(codeviewInfo->PdbFileName);

		pdbInformation.PdbFileName = pdbFileName;
		pdbInformation.Age = codeviewInfo->Age;
		pdbInformation.Signature = codeviewInfo->Signature;
	}

	return pdbInformation;
}
