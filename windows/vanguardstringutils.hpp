/*
	Utility code for the Windows-specific code to interoperate with
	Vanguard's platform-agnostic code.
*/

#pragma once

#include <string>

Platform::String^	StringFromStdString(const std::string& inputString);
std::string			StdStringFromString(Platform::String ^inputString);
