#pragma once

#include <vector>

#include "../../../Mapping/ImageResolvedImport.h"
#include "../../../Utilities/Windows/PdbUtilities.h"

class ImageStreamStageTwoRequest
{
	std::string sessionToken;
	unsigned int cheatID;
	int imageBaseAddress;
	PdbInformation pdbInformation;
	std::vector<ImageResolvedImport> resolvedImports;
public:
	ImageStreamStageTwoRequest(const std::string& sessionToken, unsigned int cheatID, int imageBaseAddress, const PdbInformation& pdbInformation, const std::vector<ImageResolvedImport>& resolvedImports);
	std::vector<unsigned char> Serialize();
};