#pragma once

// 현재 연결 상태를 나타내는 열거형
enum class ConnectionState {
	Login,			// 로그인
	GameInit,		// 게임 초기화
	GameStart,		// 게임 시작
	GameOver,		// 게임 종료
	Disconnecting,	// 연결 종료 중
	Error,			// 에러
	Disconnected,	// 연결 종료
};