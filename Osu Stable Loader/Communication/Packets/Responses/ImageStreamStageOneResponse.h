#pragma once

#include <vector>

#include "../../../Mapping/ImageImport.h"

enum class ImageStreamStageOneResult
{
	Success = 0,
	InvalidSession = 1,
	NotSubscribed = 2,
	UnknownError = 3
};

class ImageStreamStageOneResponse
{
	ImageStreamStageOneResult result;
	int imageSize;
	std::vector<ImageImport> imports;

	ImageStreamStageOneResponse(ImageStreamStageOneResult result, int imageSize, const std::vector<ImageImport>& imports);
public:
	ImageStreamStageOneResult GetResult();
	int GetImageSize();
	const std::vector<ImageImport>& GetImports();

	static ImageStreamStageOneResponse Deserialize(const std::vector<unsigned char>& payload);
};