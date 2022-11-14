#pragma once

#include <string>

#include "CheatUserData.h"
#include "LoaderUserData.h"

class UserDataManager
{
	static inline LoaderUserData* loaderUserData = nullptr;
public:
	static bool Initialize(unsigned int timeout);
	static std::string GetSessionToken();
	static std::string GetUsername();
	static std::string GetDiscordID();
	static std::string GetDiscordAvatarHash();
	static unsigned int GetCheatID();
	static std::string GetReleaseStream();
	static CheatUserData CreateCheatUserData();
};