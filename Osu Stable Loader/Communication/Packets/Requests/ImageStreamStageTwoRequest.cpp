#include "ImageStreamStageTwoRequest.h"

#include "ThemidaSDK.h"
#include "json.hpp"

#include "../../Crypto/CryptoProvider.h"
#include "../../../Utilities/Strings/StringUtilities.h"
#include "../../../Utilities/Security/xorstr.hpp"
#include "../PacketType.h"

ImageStreamStageTwoRequest::ImageStreamStageTwoRequest(const std::string& sessionToken, unsigned int cheatID, int imageBaseAddress, const std::vector<ImageResolvedImport> resolvedImports)
{
	this->sessionToken = sessionToken;
	this->cheatID = cheatID;
	this->imageBaseAddress = imageBaseAddress;
	this->resolvedImports = resolvedImports;
}

std::vector<unsigned char> ImageStreamStageTwoRequest::Serialize()
{
	VM_SHARK_BLACK_START
	STR_ENCRYPT_START

	nlohmann::json jsonPayload;

	jsonPayload[xorstr_("SessionToken")] = sessionToken;
	jsonPayload[xorstr_("CheatID")] = cheatID;
	jsonPayload[xorstr_("ImageBaseAddress")] = imageBaseAddress;

	auto resolvedImportsJson = nlohmann::json().array();
	for (ImageResolvedImport resolvedImport : resolvedImports)
	{
		nlohmann::json resolvedImportJson;

		resolvedImportJson[xorstr_("DescriptorName")] = resolvedImport.DescriptorName;
		resolvedImportJson[xorstr_("FunctionNameOrOrdinal")] = resolvedImport.FunctionNameOrOrdinal;
		resolvedImportJson[xorstr_("FunctionAddress")] = resolvedImport.FunctionAddress;

		resolvedImportsJson.push_back(resolvedImportJson);
	}

	jsonPayload[xorstr_("ResolvedImports")] = resolvedImportsJson;

	std::vector payload(CryptoProvider::GetInstance()->AESEncrypt(StringUtilities::StringToByteArray(jsonPayload.dump())));

	std::vector<unsigned char> packet;
	packet.push_back(static_cast<unsigned char>(PacketType::ImageStreamStageTwo));
	packet.insert(packet.end(), payload.begin(), payload.end());

	STR_ENCRYPT_END
	VM_SHARK_BLACK_END

	return packet;
}
