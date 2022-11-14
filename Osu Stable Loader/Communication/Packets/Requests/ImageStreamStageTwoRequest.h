#pragma once

#include <vector>

#include "../../../Mapping/ImageResolvedImport.h"

class ImageStreamStageTwoRequest
{
	std::string sessionToken;
	unsigned int cheatID;
	int imageBaseAddress;
	std::vector<ImageResolvedImport> resolvedImports;
public:
	ImageStreamStageTwoRequest(const std::string& sessionToken, unsigned int cheatID, int imageBaseAddress, const std::vector<ImageResolvedImport> resolvedImports);
	std::vector<unsigned char> Serialize();
};