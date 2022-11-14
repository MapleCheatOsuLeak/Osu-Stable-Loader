#pragma once

#include <vector>
#include <string>

class ImageStreamStageOneRequest
{
	std::string sessionToken;
	unsigned int cheatID;
	std::string releaseStream;
public:
	ImageStreamStageOneRequest(const std::string& sessionToken, unsigned int cheatID, const std::string& releaseStream);
	std::vector<unsigned char> Serialize();
};