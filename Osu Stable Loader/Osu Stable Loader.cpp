#include <windows.h>

#include "ThemidaSDK.h"

#include "Communication/Communication.h"
#include "Utilities/Security/xorstr.hpp"
#include "UserData/UserDataManager.h"
#include "Mapping/ImageMapper.h"
#include "Utilities/Memory/MemoryUtilities.h"

#pragma optimize("", off)
bool InitializeProcess(unsigned int timeout)
{
	VM_FISH_RED_START

	unsigned int timer = 0;
	DWORD pid = 0;
	while (pid == 0)
	{
		if (timer >= timeout)
			return false;

		Sleep(250);

		pid = MemoryUtilities::GetProcessIDByName(xorstr_("osu!.exe"));
		if (pid != 0)
		{
			HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
			if (!processHandle || !MemoryUtilities::GetRemoteModuleHandleA(processHandle, xorstr_("osu!auth.dll")))
				pid = 0;
			else
			{
				ImageMapper::Initialize(processHandle);

				Sleep(5000);
			}
		}
		
		timer += 250;
	}

	VM_FISH_RED_END

	return true;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	VM_FISH_RED_START

	MemoryUtilities::AdjustPrivileges();

	int protectionVar = 0x501938CA;
	CHECK_PROTECTION(protectionVar, 0x9CCC379)
		if (protectionVar != 0x9CCC379)
			return 0;

    if (!UserDataManager::Initialize(20000))
        return 0;

	if (!InitializeProcess(120000))
		return 0;

    if (!Communication::Connect())
        return 0;

    while (Communication::GetIsConnected())
        Sleep(50);

	ImageMapper::Finish();

	VM_FISH_RED_END

    return 0;
}
#pragma optimize("", on)
