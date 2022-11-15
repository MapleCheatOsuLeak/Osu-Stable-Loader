#include "UserDataManager.h"

#include <windows.h>

#include "../Utilities/Security/xorstr.hpp"

bool UserDataManager::Initialize(unsigned int timeout)
{
    unsigned int timer = 0;

    unsigned char* azukiMagic = new unsigned char[sizeof(LoaderUserData) + sizeof(unsigned int)];
    azukiMagic[0] = 0x61;
    azukiMagic[1] = 0x7A;
    azukiMagic[2] = 0x75;
    azukiMagic[3] = 0x6B;
    azukiMagic[4] = 0x69;
    azukiMagic[5] = 0x6D;
    azukiMagic[6] = 0x61;
    azukiMagic[7] = 0x67;
    azukiMagic[8] = 0x69;
    azukiMagic[9] = 0x63;

    HANDLE mtx = CreateMutexA(NULL, FALSE, xorstr_("QVPj0LSOL81Lko4d"));

    while (*reinterpret_cast<unsigned int*>(azukiMagic) != 0xdeadbeef)
    {
        Sleep(50);

        timer += 50;

        if (timer > timeout)
            return false;
    }

    CloseHandle(mtx);

    loaderUserData = reinterpret_cast<LoaderUserData*>(azukiMagic + sizeof(unsigned int));

    return true;
}

std::string UserDataManager::GetSessionToken()
{
    return loaderUserData->SessionToken;
}

std::string UserDataManager::GetUsername()
{
	return loaderUserData->Username;
}

std::string UserDataManager::GetDiscordID()
{
	return loaderUserData->DiscordID;
}

std::string UserDataManager::GetDiscordAvatarHash()
{
	return loaderUserData->DiscordAvatarHash;
}

unsigned int UserDataManager::GetCheatID()
{
	return loaderUserData->CheatID;
}

std::string UserDataManager::GetReleaseStream()
{
	return loaderUserData->ReleaseStream;
}

CheatUserData UserDataManager::CreateCheatUserData()
{
	CheatUserData cheatUserData = CheatUserData();
    sprintf(cheatUserData.SessionToken, "%s", GetSessionToken().c_str());
    sprintf(cheatUserData.Username, "%s", GetUsername().c_str());
    sprintf(cheatUserData.DiscordID, "%s", GetDiscordID().c_str());
    sprintf(cheatUserData.DiscordAvatarHash, "%s", GetDiscordAvatarHash().c_str());

    return cheatUserData;
}
