#pragma once

#include <string>

namespace util
{
	//! @brief wstring to string
	std::string toString(const std::wstring& wstr);

	//! @brief string to wstring
	std::wstring toWstring(const std::string& str);
} // namespace util