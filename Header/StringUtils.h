#pragma once

#include <string>

// std::string 에서 std::wstring으로 변환하는 함수.
inline std::wstring StringToWString(const std::string& str) {
	std::wstring wstr;

	wchar_t* buffer = new wchar_t[str.length()];

	auto length = std::mbstowcs(buffer, str.c_str(), str.length());

	wstr.assign(buffer, length);

	delete buffer;

	return wstr;
}

// std::wstring 에서 std::string으로 변환하는 함수.
inline std::string WStringToString(const std::wstring& wstr) {
	std::string str;

	char* buffer = new char[wstr.length() * 2];

	auto length = std::wcstombs(buffer, wstr.c_str(), wstr.length() * 2);

	str.assign(buffer, length);

	delete buffer;

	return str;
}