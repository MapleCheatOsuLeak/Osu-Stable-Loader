#pragma once

struct LoaderUserData
{
public:
    char Username[25];
    char SessionToken[33];
    char DiscordID[33];
    char DiscordAvatarHash[33];
    unsigned int CheatID;
    char ReleaseStream[17];
};