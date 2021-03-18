#include "Thread.h"

#include <cassert>

#include <Windows.h>
#include <process.h>

// static
unsigned __stdcall Thread::ExecuteThread(void* self) {
	return reinterpret_cast<Thread*>(self)->Run();
}

bool Thread::Start() {
	assert(m_handle == 0);

	m_handle = _beginthreadex(nullptr, 0, ExecuteThread, this, 0, &m_id);

	return m_handle != 0;
}

void Thread::Stop(int retval) {
	if (m_handle != 0) {
		TerminateThread(reinterpret_cast<HANDLE>(m_handle), retval);
		m_handle = 0;
		m_id = 0;
	}
}

int Thread::Wait() {
	if (m_handle != 0) {
		HANDLE handle = reinterpret_cast<HANDLE>(m_handle);

		// 스레드가 종료될 때 까지 대기 후 자원 해제.
		WaitForSingleObject(handle, INFINITE);
		
		DWORD retval = 0;
		GetExitCodeThread(handle, &retval);

		CloseHandle(handle);

		// 해제한 자원 사용 방지용.
		m_handle = 0;
		m_id = 0;

		return retval;
	}
	else {
		return 0;
	}
}

bool Thread::IsRunning() const {
	DWORD status;

	if (m_handle != 0) {
		GetExitCodeThread(reinterpret_cast<HANDLE>(m_handle), &status);

		// 반환 코드가 STILL_ACTIVE 이면 현재 실행중.
		return status == STILL_ACTIVE;
	}
	else {
		return false;
	}
}