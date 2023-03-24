#pragma once
#include <string>
#include <Windows.h>
#include "../Security/xorstr.hpp"

class WindowsUtilities
{
public:
	static std::string GetWindowsFolderPath();
	static BOOL Is64BitOS();
	static std::string GetNtdllPath();
};