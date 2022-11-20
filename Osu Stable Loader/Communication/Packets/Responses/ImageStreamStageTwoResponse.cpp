#include "ImageStreamStageTwoResponse.h"

#include "json.hpp"
#include "ThemidaSDK.h"

#include "../../Crypto/CryptoProvider.h"
#include "../../../Utilities/Strings/StringUtilities.h"
#include "../../../Utilities/Security/xorstr.hpp"
#include "../../../Mapping/ImageSection.h"

ImageStreamStageTwoResponse::ImageStreamStageTwoResponse(ImageStreamStageTwoResult result, const std::vector<int>& callbacks, const std::vector<ImageSection>& sections)
{
	this->result = result;
	this->callbacks = callbacks;
	this->sections = sections;
}

ImageStreamStageTwoResult ImageStreamStageTwoResponse::GetResult()
{
	return result;
}

const std::vector<int>& ImageStreamStageTwoResponse::GetCallbacks()
{
	return callbacks;
}

const std::vector<ImageSection>& ImageStreamStageTwoResponse::GetSections()
{
	return sections;
}

#pragma optimize("", off)
ImageStreamStageTwoResponse ImageStreamStageTwoResponse::Deserialize(const std::vector<unsigned char>& payload)
{
	VM_SHARK_BLACK_START
	STR_ENCRYPT_START

	nlohmann::json jsonPayload = nlohmann::json::parse(StringUtilities::ByteArrayToString(CryptoProvider::GetInstance()->AESDecrypt(payload)));

	ImageStreamStageTwoResult result = jsonPayload[xorstr_("Result")];

	ImageStreamStageTwoResponse response = ImageStreamStageTwoResponse(result, {}, {});

	if (result == ImageStreamStageTwoResult::Success)
	{
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

		std::vector<int> callbacks = jsonPayload[xorstr_("Callbacks")].get<std::vector<int>>();

		response = ImageStreamStageTwoResponse(result, callbacks, sections);
	}

	STR_ENCRYPT_END
	VM_SHARK_BLACK_END

	return response;
}
#pragma optimize("", on)
