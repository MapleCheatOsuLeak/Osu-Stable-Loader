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
public:
	static void Initialize(HANDLE processHandle);
	static int AllocateMemoryForImage(int imageSize);
	static std::vector<ImageResolvedImport> ResolveImports(const std::vector<ImageImport>& imports);
	static void MapImage(int ldrpHandleTlsDataOffset, int entryPointOffset, const std::vector<ImageSection>& imageSections, const std::vector<int>& tlsCallbacks);
	static void Finish();
};