#include "ImageStreamStageOneRequest.h"

#include "json.hpp"
#include "ThemidaSDK.h"

#include "../../../Utilities/Security/xorstr.hpp"
#include "../../Crypto/CryptoProvider.h"
#include "../../../Utilities/Strings/StringUtilities.h"
#include "../PacketType.h"

ImageStreamStageOneRequest::ImageStreamStageOneRequest(const std::string& sessionToken, unsigned int cheatID, const std::string& releaseStream)
{
	this->sessionToken = sessionToken;
	this->cheatID = cheatID;
	this->releaseStream = releaseStream;
}

#pragma optimize("", off)
std::vector<unsigned char> ImageStreamStageOneRequest::Serialize()
{
	VM_TIGER_WHITE_START
	STR_ENCRYPT_START

	nlohmann::json jsonPayload;

	jsonPayload[xorstr_("SessionToken")] = sessionToken;
	jsonPayload[xorstr_("CheatID")] = cheatID;
	jsonPayload[xorstr_("ReleaseStream")] = releaseStream;

	std::vector payload(CryptoProvider::GetInstance()->AESEncrypt(StringUtilities::StringToByteArray(jsonPayload.dump())));

	std::vector<unsigned char> packet;
	packet.push_back(static_cast<unsigned char>(PacketType::ImageStreamStageOne));
	packet.insert(packet.end(), payload.begin(), payload.end());

	STR_ENCRYPT_END
		VM_TIGER_WHITE_END

	return packet;
}
#pragma optimize("", on)
