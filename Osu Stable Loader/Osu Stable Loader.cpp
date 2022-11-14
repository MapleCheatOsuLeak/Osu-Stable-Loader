#include <windows.h>

#include "Communication/Communication.h"
#include "Utilities/Security/xorstr.hpp"
#include "UserData/UserDataManager.h"
#include "Mapping/ImageMapper.h"
#include "Utilities/Memory/MemoryUtilities.h"

bool InitializeProcess(unsigned int timeout)
{
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
				ImageMapper::Initialize(processHandle);
		}
		
		timer += 250;
	}

	return true;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	MemoryUtilities::AdjustPrivileges();

    if (!UserDataManager::Initialize(20000))
        return 0;

	if (!InitializeProcess(120000))
		return 0;

    if (!Communication::Connect())
        return 0;

    while (Communication::GetIsConnected())
        Sleep(50);

	ImageMapper::Finish();

    return 0;
}
