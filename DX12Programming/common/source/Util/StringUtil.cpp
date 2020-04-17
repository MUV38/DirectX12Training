#include "Util/StringUtil.h"

#include <codecvt>
#include <locale>

namespace util
{
	// wstring to string
	std::string toString(const std::wstring& wstr)
	{
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr);
	}

	// string to wstring
	std::wstring toWstring(const std::string& str)
	{
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str);
	}
} // namespace util