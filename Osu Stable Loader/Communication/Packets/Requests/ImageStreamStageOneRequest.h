#pragma once

#include <vector>
#include <string>
#include "../../../Utilities/Windows/PdbUtilities.h"

class ImageStreamStageOneRequest
{
	std::string sessionToken;
	unsigned int cheatID;
	std::string releaseStream;
	PdbInformation pdbInformation;
public:
	ImageStreamStageOneRequest(const std::string& sessionToken, unsigned int cheatID, const std::string& releaseStream, const PdbInformation pdbInformation);
	std::vector<unsigned char> Serialize();
};