#pragma once

#include <string>
#include <Windows.h>

struct PdbInformation
{
	GUID Signature;
	ULONG Age;
	std::string PdbFileName;
};

class PdbUtilities
{
public:
	static PdbInformation GetPdbInformation(std::string modulePath, bool is32Bit);
};