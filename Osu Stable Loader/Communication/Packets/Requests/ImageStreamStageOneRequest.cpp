#include "ImageStreamStageOneRequest.h"

#include "json.hpp"
#include "ThemidaSDK.h"

#include "../../../Utilities/Security/xorstr.hpp"
#include "../../Crypto/CryptoProvider.h"
#include "../../../Utilities/Strings/StringUtilities.h"
#include "../PacketType.h"

ImageStreamStageOneRequest::ImageStreamStageOneRequest(const std::string& sessionToken, unsigned int cheatID, const std::string& releaseStream, const PdbInformation pdbInformation)
{
	this->sessionToken = sessionToken;
	this->cheatID = cheatID;
	this->releaseStream = releaseStream;
	this->pdbInformation = pdbInformation;
}

#pragma optimize("", off)
std::vector<unsigned char> ImageStreamStageOneRequest::Serialize()
{
	VM_SHARK_BLACK_START
	STR_ENCRYPT_START

	nlohmann::json jsonPayload;

	jsonPayload[xorstr_("SessionToken")] = sessionToken;
	jsonPayload[xorstr_("CheatID")] = cheatID;
	jsonPayload[xorstr_("ReleaseStream")] = releaseStream;
	jsonPayload[xorstr_("PdbName")] = pdbInformation.PdbFileName;
	jsonPayload[xorstr_("PdbAge")] = pdbInformation.Age;
	std::array<char, 40> output;
	snprintf(output.data(), output.size(), "{%08X-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X}", 
		pdbInformation.Signature.Data1, pdbInformation.Signature.Data2, pdbInformation.Signature.Data3, pdbInformation.Signature.Data4[0], 
		pdbInformation.Signature.Data4[1], pdbInformation.Signature.Data4[2], pdbInformation.Signature.Data4[3], pdbInformation.Signature.Data4[4], 
		pdbInformation.Signature.Data4[5], pdbInformation.Signature.Data4[6], pdbInformation.Signature.Data4[7]);
	jsonPayload[xorstr_("PdbGuid")] = std::string(output.data());

	std::vector payload(CryptoProvider::GetInstance()->AESEncrypt(StringUtilities::StringToByteArray(jsonPayload.dump())));

	std::vector<unsigned char> packet;
	packet.push_back(static_cast<unsigned char>(PacketType::ImageStreamStageOne));
	packet.insert(packet.end(), payload.begin(), payload.end());

	STR_ENCRYPT_END
	VM_SHARK_BLACK_END

	return packet;
}
#pragma optimize("", on)
