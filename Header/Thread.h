#pragma once

#include <cassert>
#include <cstdint>

// 스레드를 나타내는 클래스.
class Thread {
private:
	uintptr_t m_handle = 0;	// 스레드 핸들
	unsigned int m_id = 0;	// 스레드 번호

	static unsigned __stdcall ExecuteThread(void* self);

public:
	Thread() = default;

	// 복사 생성자 삭제. 자원은 복사한다 라는 의미가 모순.
	Thread(const Thread&) = delete;

	Thread(Thread&& rhs)
		: m_handle(rhs.m_handle), m_id(rhs.m_id) {
		rhs.m_handle = 0;
		rhs.m_id = 0;
	}

	// 복사 대입 연산자 삭제. 자원은 복사한다 라는 의미가 모순.
	Thread& operator =(const Thread&) = delete;

	Thread& operator =(Thread&& rhs) {
		assert(m_handle == 0 && "cannot assign to started thread.");
	
		m_handle = rhs.m_handle;
		m_id = rhs.m_id;
		rhs.m_handle = 0;
		rhs.m_id = 0;

		return *this;
	}

	virtual ~Thread() = default;

	// 스레드가 실행할 메서드.
	virtual int Run() = 0;

	// 스레드 시작 메서드.
	bool Start();

	// 스레드 강제 종료 메서드.
	void Stop(int retval = 0);

	// 스레드 번호 반환.
	unsigned int GetThreadID() const { return m_id; }

	// 스레드가 종료될 때 까지 대기하고 종료하는 메서드.
	int Wait();

	// 현재 스레드가 실행되고 있는지 여부를 반환하는 메서드.
	bool IsRunning() const;
};