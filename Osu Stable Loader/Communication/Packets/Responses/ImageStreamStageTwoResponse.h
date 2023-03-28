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
	int ldrpHandleTlsDataOffset;
	int ldrpInvertedFunctionTablesOffset;
	int rtlInsertInvertedFunctionTableOffset;
	int entryPointOffset;
	std::vector<unsigned char> headers;
	std::vector<ImageSection> sections;
	std::vector<int> tlsCallbacks;

	ImageStreamStageTwoResponse(ImageStreamStageTwoResult result, int ldrpHandleTlsDataOffset, int ldrpInvertedFunctionTablesOffset, int rtlInsertInvertedFunctionTableOffset, int entryPointOffset, const std::vector<unsigned char>& headers, const std::vector<ImageSection>& sections, const std::vector<int>& tlsCallbacks);
public:
	ImageStreamStageTwoResult GetResult();
	int GetLdrpHandleTlsDataOffset();
	int GetLdrpInvertedFunctionTablesOffset();
	int GetRtlInsertInvertedFunctionTableOffset();
	int GetEntryPointOffset();
	const std::vector<unsigned char>& GetHeaders();
	const std::vector<ImageSection>& GetSections();
	const std::vector<int>& GetTLSCallbacks();

	static ImageStreamStageTwoResponse Deserialize(const std::vector<unsigned char>& payload);
};