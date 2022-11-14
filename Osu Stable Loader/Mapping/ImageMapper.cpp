#include "ImageMapper.h"

#include "../Utilities/Memory/MemoryUtilities.h"
#include "../UserData/UserDataManager.h"

void ImageMapper::Initialize(HANDLE processHandle)
{
	ImageMapper::processHandle = processHandle;
}

int ImageMapper::AllocateMemoryForImage(int imageSize)
{
	imageBaseAddress = VirtualAllocEx(processHandle, NULL, imageSize, MEM_COMMIT, PAGE_READONLY);

	return reinterpret_cast<int>(imageBaseAddress);
}

std::vector<ImageResolvedImport> ImageMapper::ResolveImports(const std::vector<ImageImport>& imports)
{
	std::vector<ImageResolvedImport> resolvedImports;

	for (const ImageImport& import : imports)
	{
		ImageResolvedImport resolvedImport = ImageResolvedImport();
		resolvedImport.DescriptorName = import.DescriptorName;
		resolvedImport.FunctionNameOrOrdinal = import.FunctionNameOrOrdinal;
		resolvedImport.FunctionAddress = MemoryUtilities::GetRemoteProcAddress(processHandle, import.DescriptorName.c_str(), import.FunctionNameOrOrdinal.c_str());

		resolvedImports.push_back(resolvedImport);
	}

	return resolvedImports;
}

void ImageMapper::MapImage(const std::vector<ImageSection>& imageSections, const std::vector<int>& callbacks)
{
	for (ImageSection section : imageSections)
	{
		DWORD oldProtect;
		VirtualProtectEx(processHandle, reinterpret_cast<LPVOID>(section.Address), section.Data.size(), PAGE_EXECUTE_READWRITE, &oldProtect);

		WriteProcessMemory(processHandle, reinterpret_cast<LPVOID>(section.Address), section.Data.data(), section.Data.size(), NULL);

		VirtualProtectEx(processHandle, reinterpret_cast<LPVOID>(section.Address), section.Data.size(), oldProtect, &oldProtect);

		VirtualProtectEx(processHandle, reinterpret_cast<LPVOID>(section.Address), section.AlignedSize, section.Protection, &oldProtect);
	}

	for (int callbackOffset : callbacks)
	{
		if (&callbackOffset == &callbacks.back())
		{
			CheatUserData userData = UserDataManager::CreateCheatUserData();

			LPVOID userDataAddress = VirtualAllocEx(processHandle, NULL, sizeof(CheatUserData), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			WriteProcessMemory(processHandle, userDataAddress, &userData, sizeof(CheatUserData), NULL);

			MemoryUtilities::CallRemoteFunction(processHandle, reinterpret_cast<int>(imageBaseAddress) + callbackOffset, { reinterpret_cast<int>(imageBaseAddress), DLL_PROCESS_ATTACH, reinterpret_cast<int>(userDataAddress) });
		}
		else
			MemoryUtilities::CallRemoteFunction(processHandle, reinterpret_cast<int>(imageBaseAddress) + callbackOffset, { reinterpret_cast<int>(imageBaseAddress), DLL_PROCESS_ATTACH, 0 });

	}
}

void ImageMapper::Finish()
{
	CloseHandle(processHandle);
}
