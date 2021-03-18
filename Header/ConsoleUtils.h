#pragma once

#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Position.h"

// 화면 초기화 함수.
void ClearScreen();

// 커서 활성/비활성 함수.
void ToggleCursor(bool enable);

// 커서 이동 함수.
void MoveCursor(Position pos);

// 지정된 위치에 문자열을 출력하는 함수
void DrawString(const std::wstring& str, Position pos);

// 지정된 위치에 문자를 출력하는 함수
void DrawChar(wchar_t ch, Position pos);