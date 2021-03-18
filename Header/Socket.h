#pragma once

#include <cassert>
#include <string>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>

// 소켓 오류 코드를 나타내는 열거형.
enum class SocketStatus {
	Success,			// 성공
	InvalidAddress,		// 유효하지 않은 주소
	CreateFailed,		// 생성 실패
	BindFailed,			// bind() 실패
	ListenFailed,		// listen() 실패
	ConnectFailed,		// connect() 실패
	ConnectionRefused,	// 통신 도중 끊김
};

// 소켓을 나타내는 클래스.
class Socket {
private:
	SOCKET m_handle = INVALID_SOCKET;
	SOCKADDR_IN m_address;

	Socket(SOCKET handle, SOCKADDR_IN addr)
		: m_handle(handle), m_address(addr) {}

	SocketStatus ReadBuffer(char* buffer, int bufferLength);

	SocketStatus WriteBuffer(const char* buffer, int bufferLength);

public:
	Socket() = default;

	// 복사 생성자 삭제. 자원을 복사하는 것이 모순.
	Socket(const Socket&) = delete;

	Socket(Socket&& rhs)
		: Socket(rhs.m_handle, rhs.m_address) {
		rhs.m_handle = INVALID_SOCKET;
	}

	// 복사 대입 연산자 삭제. 자원을 복사하는 것이 모순.
	Socket& operator =(const Socket&) = delete;

	Socket& operator =(Socket&& rhs) {
		assert(m_handle == INVALID_SOCKET && "cannot assign to opened socket.");

		m_handle = rhs.m_handle;
		m_address = rhs.m_address;

		rhs.m_handle = INVALID_SOCKET;

		return *this;
	}

	// 지정된 주소의 소켓과 연결하는 메서드. connect()와 동일.
	SocketStatus Connect(const char* address, unsigned short port);

	// 소켓에 지정된 주소를 바인딩 하는 메서드. bind() & listen() 와 동일.
	SocketStatus Bind(const char* address, unsigned short port);

	// 소켓 접속을 받아들이고 연결된 소켓을 반환하는 메서드. accept()와 동일.
	Socket Accept();

	// 현재 accept 요청이 있는지 확인하는 메서드.
	bool HasAcceptRequest() const;

	// 현재 읽을 수 있는 데이터가 있는지 확인하는 메서드.
	// 내부적으로 HasAcceptRequest()와 코드가 동일하므로 그대로 호출.
	bool HasReceivedData() const { return HasAcceptRequest(); }

	// 연결을 종료하고 자원을 해제하는 메서드.
	void Close();

	// 소켓에서 char 만큼의 데이터를 읽어오는 메서드.
	SocketStatus Read(char &c);

	// 소켓에서 short 만큼의 데이터를 읽어오는 메서드.
	SocketStatus Read(short &s);

	// 소켓에서 int 만큼의 데이터를 읽어오는 메서드.
	SocketStatus Read(int &i);

	// 소켓에서 string 데이터를 읽어오는 메서드. 최대 127 글자의 문자열만 허용.
	SocketStatus Read(std::string &s);

	// 소켓에 char 만큼의 데이터를 쓰는 메서드.
	SocketStatus Write(char b);

	// 소켓에 short 만큼의 데이터를 쓰는 메서드.
	SocketStatus Write(short s);

	// 소켓에 int 만큼의 데이터를 쓰는 메서드.
	SocketStatus Write(int i);

	// 소켓에 string 데이터를 쓰는 메서드. 최대 127 글자의 문자열만 허용.
	SocketStatus Write(const std::string& s);

	// 현재 소켓이 연결 되어있는지 판단하는 메서드.
	bool IsConnected() const { return m_handle != INVALID_SOCKET; }

	// 소켓 오류 코드를 문자열로 바꾸는 정적 메서드. 디버깅용.
	static const char* StatusToString(SocketStatus status);
};