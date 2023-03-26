#pragma once

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <vector>

#include "ImageImport.h"
#include "ImageResolvedImport.h"
#include "ImageSection.h"

class ImageMapper
{
	static inline HANDLE processHandle;
	static inline void* imageBaseAddress;
	static inline unsigned int sizeOfHeaders;

	static void mapHeaders(const std::vector<unsigned char>& headers);
	static void unmapHeaders();
	static void mapSections(const std::vector<ImageSection>& imageSections);
	static void fixStaticTLS(int ldrpHandleTlsDataOffset);
	static void callInitializationRoutines(int entryPointOffset, const std::vector<int>& tlsCallbacks);
public:
	static void Initialize(HANDLE processHandle);
	static int AllocateMemoryForImage(int imageSize);
	static std::vector<ImageResolvedImport> ResolveImports(const std::vector<ImageImport>& imports);
	static void MapImage(int ldrpHandleTlsDataOffset, int entryPointOffset, const std::vector<unsigned char>& headers, const std::vector<ImageSection>& imageSections, const std::vector<int>& tlsCallbacks);
	static void Finish();
};