#include "ConsoleUtils.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void ClearScreen() {
	auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD screen = { 0, 0 };
	CONSOLE_SCREEN_BUFFER_INFO info;
	DWORD _; // placeholder

	if (!GetConsoleScreenBufferInfo(handle, &info)) {
		return;
	}

	auto length = info.dwSize.X * info.dwSize.Y;

	FillConsoleOutputCharacterW(handle, L' ', length, screen, &_);
}

void ToggleCursor(bool enable) {
	auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;

	if (!GetConsoleCursorInfo(handle, &info)) {
		return;
	}

	info.bVisible = enable ? TRUE : FALSE;

	SetConsoleCursorInfo(handle, &info);
}

void MoveCursor(Position pos) {
	auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coord = { pos.X, pos.Y };

	SetConsoleCursorPosition(handle, coord);
}

// FIX: Bug in drawing wide width character.
void DrawString(const std::wstring& str, Position pos) {
	auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD _; // placeholder
	COORD coord = { pos.X, pos.Y };

	for (const auto ch : str) {
		FillConsoleOutputCharacterW(handle, ch, 1, coord, &_);

		coord.X++;
	}
}

void DrawChar(wchar_t ch, Position pos) {
	auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD _; // placeholder
	COORD coord = { pos.X, pos.Y };

	FillConsoleOutputCharacterW(handle, ch, 1, coord, &_);
}