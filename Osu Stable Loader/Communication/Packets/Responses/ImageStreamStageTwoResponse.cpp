#include "ImageStreamStageTwoResponse.h"

#include "json.hpp"
#include "ThemidaSDK.h"

#include "../../Crypto/CryptoProvider.h"
#include "../../../Utilities/Strings/StringUtilities.h"
#include "../../../Utilities/Security/xorstr.hpp"
#include "../../../Mapping/ImageSection.h"

ImageStreamStageTwoResponse::ImageStreamStageTwoResponse(ImageStreamStageTwoResult result, int ldrpHandleTlsDataOffset, int ldrpInvertedFunctionTablesOffset, int rtlInsertInvertedFunctionTableOffset, int entryPointOffset, const std::vector<unsigned char>& headers, const std::vector<ImageSection>& sections, const std::vector<int>& tlsCallbacks)
{
	this->result = result;
	this->ldrpHandleTlsDataOffset = ldrpHandleTlsDataOffset;
	this->ldrpInvertedFunctionTablesOffset = ldrpInvertedFunctionTablesOffset;
	this->rtlInsertInvertedFunctionTableOffset = rtlInsertInvertedFunctionTableOffset;
	this->entryPointOffset = entryPointOffset;
	this->headers = headers;
	this->sections = sections;
	this->tlsCallbacks = tlsCallbacks;
}

ImageStreamStageTwoResult ImageStreamStageTwoResponse::GetResult()
{
	return result;
}

int ImageStreamStageTwoResponse::GetLdrpHandleTlsDataOffset()
{
	return ldrpHandleTlsDataOffset;
}

int ImageStreamStageTwoResponse::GetLdrpInvertedFunctionTablesOffset()
{
	return ldrpInvertedFunctionTablesOffset;
}

int ImageStreamStageTwoResponse::GetRtlInsertInvertedFunctionTableOffset()
{
	return rtlInsertInvertedFunctionTableOffset;
}

int ImageStreamStageTwoResponse::GetEntryPointOffset()
{
	return entryPointOffset;
}

const std::vector<unsigned char>& ImageStreamStageTwoResponse::GetHeaders()
{
	return headers;
}

const std::vector<ImageSection>& ImageStreamStageTwoResponse::GetSections()
{
	return sections;
}

const std::vector<int>& ImageStreamStageTwoResponse::GetTLSCallbacks()
{
	return tlsCallbacks;
}

#pragma optimize("", off)
ImageStreamStageTwoResponse ImageStreamStageTwoResponse::Deserialize(const std::vector<unsigned char>& payload)
{
	VM_TIGER_WHITE_START
	STR_ENCRYPT_START

	nlohmann::json jsonPayload = nlohmann::json::parse(StringUtilities::ByteArrayToString(CryptoProvider::GetInstance()->AESDecrypt(payload)));

	ImageStreamStageTwoResult result = jsonPayload[xorstr_("Result")];

	ImageStreamStageTwoResponse response = ImageStreamStageTwoResponse(result, 0, 0, 0, 0, {}, {}, {});

	if (result == ImageStreamStageTwoResult::Success)
	{
		int ldrpHandleTlsDataOffset = jsonPayload[xorstr_("LdrpHandleTlsDataOffset")];
		int ldrpInvertedFunctionTablesOffset = jsonPayload[xorstr_("LdrpInvertedFunctionTablesOffset")];
		int rtlInsertInvertedFunctionTableOffset = jsonPayload[xorstr_("RtlInsertInvertedFunctionTableOffset")];
		int entryPointOffset = jsonPayload[xorstr_("EntryPointOffset")];
		std::vector<unsigned char> headers = CryptoProvider::GetInstance()->Base64Decode(jsonPayload[xorstr_("Headers")]);

		std::vector<ImageSection> sections = {};
		for (auto& jsonSection : jsonPayload[xorstr_("Sections")].items())
		{
			auto jsonSectionItem = jsonSection.value();

			ImageSection section = ImageSection();
			section.Address = static_cast<int>(jsonSectionItem[xorstr_("Address")]);
			section.Data = CryptoProvider::GetInstance()->Base64Decode(jsonSectionItem[xorstr_("Data")]);
			section.Protection = static_cast<int>(jsonSectionItem[xorstr_("Protection")]);
			section.ProtectionSize = static_cast<int>(jsonSectionItem[xorstr_("ProtectionSize")]);

			sections.push_back(section);
		}

		std::vector<int> tlsCallbacks = jsonPayload[xorstr_("TLSCallbacks")].get<std::vector<int>>();

		response = ImageStreamStageTwoResponse(result, ldrpHandleTlsDataOffset, ldrpInvertedFunctionTablesOffset, rtlInsertInvertedFunctionTableOffset, entryPointOffset, headers, sections, tlsCallbacks);
	}

	STR_ENCRYPT_END
		VM_TIGER_WHITE_END

	return response;
}
#pragma optimize("", on)
