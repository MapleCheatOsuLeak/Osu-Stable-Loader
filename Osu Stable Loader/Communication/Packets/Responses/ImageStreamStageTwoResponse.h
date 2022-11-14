#pragma once

#include <vector>

#include "../../Mapping/ImageSection.h"

enum class ImageStreamStageTwoResult
{
	Success = 0,
	InvalidSession = 1,
	NotSubscribed = 2,
	UnknownError = 3
};

class ImageStreamStageTwoResponse
{
	ImageStreamStageTwoResult result;
	std::vector<int> callbacks;
	std::vector<ImageSection> sections;

	ImageStreamStageTwoResponse(ImageStreamStageTwoResult result, const std::vector<int>& callbacks, const std::vector<ImageSection>& Sections);
public:
	ImageStreamStageTwoResult GetResult();
	const std::vector<int>& GetCallbacks();
	const std::vector<ImageSection>& GetSections();

	static ImageStreamStageTwoResponse Deserialize(const std::vector<unsigned char>& payload);
};