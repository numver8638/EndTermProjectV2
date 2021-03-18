#pragma once

#include <set>

class Thread;	// Defined in "Thread.h"

// 테트리스 서버
class Server {
	friend class ClientThread;

private:
	// 현재 작동하고 있는 스레드의 집합.
	std::set<Thread*> m_threads;

public:

	// 서버 시작 메서드.
	int Start();
};