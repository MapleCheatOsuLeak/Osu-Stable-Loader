#pragma once

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <vector>

#include "ImageImport.h"
#include "ImageResolvedImport.h"
#include "ImageSection.h"

template<typename T>
struct _RTL_INVERTED_FUNCTION_TABLE_ENTRY
{
	T ExceptionDirectory;   // PIMAGE_RUNTIME_FUNCTION_ENTRY
	T ImageBase;
	uint32_t ImageSize;
	uint32_t SizeOfTable;
};

template<typename T>
struct _RTL_INVERTED_FUNCTION_TABLE8
{
	ULONG Count;
	ULONG MaxCount;
	ULONG Epoch;
	UCHAR Overflow;
	_RTL_INVERTED_FUNCTION_TABLE_ENTRY<T> Entries[0x200];
};

enum InsertInvertedFunctionTableResult
{
    Failed = 0,
    Success = 1,
    SuccessWithSEH = 2
};

class ImageMapper
{
	static inline HANDLE processHandle;
	static inline void* imageBaseAddress;
	static inline unsigned int sizeOfHeaders;

	static void mapHeaders(const std::vector<unsigned char>& headers);
	static void unmapHeaders();
	static void mapSections(const std::vector<ImageSection>& imageSections);
	static void fixStaticTLS(int ldrpHandleTlsDataOffset);
    static InsertInvertedFunctionTableResult insertInvertedFunctionTable(int ldrpInvertedFunctionTablesOffset, int rtlInsertInvertedFunctionTableOffset, std::vector<unsigned char> headers);
	static void enableExceptions(int ldrpInvertedFunctionTablesOffset, int rtlInsertInvertedFunctionTableOffset, std::vector<unsigned char> headers);
	static void callInitializationRoutines(int entryPointOffset, const std::vector<int>& tlsCallbacks);
public:
	static void Initialize(HANDLE processHandle);
	static int AllocateMemoryForImage(int imageSize);
	static std::vector<ImageResolvedImport> ResolveImports(const std::vector<ImageImport>& imports);
	static void MapImage(int ldrpHandleTlsDataOffset, int ldrpInvertedFunctionTablesOffset, int rtlInsertInvertedFunctionTableOffset, int entryPointOffset, const std::vector<unsigned char>& headers, const std::vector<ImageSection>& imageSections, const std::vector<int>& tlsCallbacks);
	static void Finish();
};