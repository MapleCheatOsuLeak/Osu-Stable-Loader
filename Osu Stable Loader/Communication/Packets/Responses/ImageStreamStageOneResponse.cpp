#include "ImageStreamStageOneResponse.h"

#include "json.hpp"
#include "ThemidaSDK.h"

#include "../../Crypto/CryptoProvider.h"
#include "../../../Utilities/Strings/StringUtilities.h"
#include "../../../Utilities/Security/xorstr.hpp"

ImageStreamStageOneResponse::ImageStreamStageOneResponse(ImageStreamStageOneResult result, int imageSize, const std::vector<ImageImport>& imports)
{
	this->result = result;
	this->imageSize = imageSize;
	this->imports = imports;
}

ImageStreamStageOneResult ImageStreamStageOneResponse::GetResult()
{
	return result;
}

int ImageStreamStageOneResponse::GetImageSize()
{
	return imageSize;
}

const std::vector<ImageImport>& ImageStreamStageOneResponse::GetImports()
{
	return imports;
}

#pragma optimize("", off)
ImageStreamStageOneResponse ImageStreamStageOneResponse::Deserialize(const std::vector<unsigned char>& payload)
{
	VM_TIGER_WHITE_START
	STR_ENCRYPT_START

	nlohmann::json jsonPayload = nlohmann::json::parse(StringUtilities::ByteArrayToString(CryptoProvider::GetInstance()->AESDecrypt(payload)));

	ImageStreamStageOneResult result = jsonPayload[xorstr_("Result")];

	ImageStreamStageOneResponse response = ImageStreamStageOneResponse(result, 0, {});

	if (result == ImageStreamStageOneResult::Success)
	{
		int imageSize = jsonPayload[xorstr_("ImageSize")];
		std::vector<ImageImport> imports = {};

		for (auto& jsonImport : jsonPayload[xorstr_("Imports")].items())
		{
			auto jsonImportItem = jsonImport.value();

			ImageImport import = ImageImport();
			import.DescriptorName = jsonImportItem[xorstr_("DescriptorName")];
			import.FunctionNameOrOrdinal = jsonImportItem[xorstr_("FunctionNameOrOrdinal")];

			imports.push_back(import);
		}

		response = ImageStreamStageOneResponse(result, imageSize, imports);
	}

	STR_ENCRYPT_END
	VM_TIGER_WHITE_END

	return response;
}
#pragma optimize("", on)
