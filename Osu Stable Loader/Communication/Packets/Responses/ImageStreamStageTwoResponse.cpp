#include "ImageStreamStageTwoResponse.h"

#include "json.hpp"
#include "ThemidaSDK.h"

#include "../../Crypto/CryptoProvider.h"
#include "../../../Utilities/Strings/StringUtilities.h"
#include "../../../Utilities/Security/xorstr.hpp"
#include "../../../Mapping/ImageSection.h"

ImageStreamStageTwoResponse::ImageStreamStageTwoResponse(ImageStreamStageTwoResult result, int ldrpHandleTlsDataOffset, int entryPointOffset, const std::vector<ImageSection>& sections, const std::vector<int>& tlsCallbacks)
{
	this->result = result;
	this->ldrpHandleTlsDataOffset = ldrpHandleTlsDataOffset;
	this->entryPointOffset = entryPointOffset;
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

int ImageStreamStageTwoResponse::GetEntryPointOffset()
{
	return entryPointOffset;
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
	VM_SHARK_BLACK_START
	STR_ENCRYPT_START

	nlohmann::json jsonPayload = nlohmann::json::parse(StringUtilities::ByteArrayToString(CryptoProvider::GetInstance()->AESDecrypt(payload)));

	ImageStreamStageTwoResult result = jsonPayload[xorstr_("Result")];

	ImageStreamStageTwoResponse response = ImageStreamStageTwoResponse(result, 0, 0, {}, {});

	if (result == ImageStreamStageTwoResult::Success)
	{
		int ldrpHandleTlsDataOffset = jsonPayload[xorstr_("LdrpHandleTlsDataOffset")];
		int entryPointOffset = jsonPayload[xorstr_("EntryPointOffset")];

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

		response = ImageStreamStageTwoResponse(result, ldrpHandleTlsDataOffset, entryPointOffset, sections, tlsCallbacks);
	}

	STR_ENCRYPT_END
	VM_SHARK_BLACK_END

	return response;
}
#pragma optimize("", on)
