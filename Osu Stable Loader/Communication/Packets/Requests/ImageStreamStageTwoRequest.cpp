#include "ImageStreamStageTwoRequest.h"

#include "ThemidaSDK.h"
#include "json.hpp"

#include "../../Crypto/CryptoProvider.h"
#include "../../../Utilities/Strings/StringUtilities.h"
#include "../../../Utilities/Security/xorstr.hpp"
#include "../PacketType.h"

ImageStreamStageTwoRequest::ImageStreamStageTwoRequest(const std::string& sessionToken, unsigned int cheatID, int imageBaseAddress, const PdbInformation& pdbInformation, const std::vector<ImageResolvedImport>& resolvedImports)
{
	this->sessionToken = sessionToken;
	this->cheatID = cheatID;
	this->imageBaseAddress = imageBaseAddress;
	this->pdbInformation = pdbInformation;
	this->resolvedImports = resolvedImports;
}

#pragma optimize("", off)
std::vector<unsigned char> ImageStreamStageTwoRequest::Serialize()
{
	VM_SHARK_BLACK_START
	STR_ENCRYPT_START

	nlohmann::json jsonPayload;

	jsonPayload[xorstr_("SessionToken")] = sessionToken;
	jsonPayload[xorstr_("CheatID")] = cheatID;
	jsonPayload[xorstr_("ImageBaseAddress")] = imageBaseAddress;

	jsonPayload[xorstr_("NtdllPdbName")] = pdbInformation.PdbFileName;
	jsonPayload[xorstr_("NtdllPdbAge")] = pdbInformation.Age;
	std::array<char, 34> output;
	snprintf(output.data(), output.size(), "%08x%04hx%04hx%02x%02x%02x%02x%02x%02x%02x%02x",
		pdbInformation.Signature.Data1, pdbInformation.Signature.Data2, pdbInformation.Signature.Data3, pdbInformation.Signature.Data4[0],
		pdbInformation.Signature.Data4[1], pdbInformation.Signature.Data4[2], pdbInformation.Signature.Data4[3], pdbInformation.Signature.Data4[4],
		pdbInformation.Signature.Data4[5], pdbInformation.Signature.Data4[6], pdbInformation.Signature.Data4[7]);
	jsonPayload[xorstr_("NtdllPdbGuid")] = std::string(output.data());

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
#pragma optimize("", on)
