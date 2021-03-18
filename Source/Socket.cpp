#include "Socket.h"

#include <cassert>
#include <limits>

#define BACKLOG_COUNT (10)

#ifdef max
	// std::numeric_limits<>::max() 를 가리기 때문에 매크로가 정의되어있다면 해제.
	#undef max
#endif // max

SocketStatus Socket::Connect(const char* address, unsigned short port) {
	assert(m_handle == INVALID_SOCKET);

	m_address.sin_family = AF_INET;
	m_address.sin_port = htons(port);
	m_address.sin_addr.s_addr = inet_addr(address);

	if (m_address.sin_addr.s_addr == INADDR_NONE) {
		return SocketStatus::InvalidAddress;
	}

	m_handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_handle == INVALID_SOCKET) {
		return SocketStatus::CreateFailed;
	}

	if (connect(m_handle, reinterpret_cast<const SOCKADDR*>(&m_address), sizeof(m_address)) != 0) {
		closesocket(m_handle);
		m_handle = INVALID_SOCKET;

		return SocketStatus::ConnectFailed;
	}

	return SocketStatus::Success;
}

SocketStatus Socket::Bind(const char* address, unsigned short port) {
	assert(m_handle == INVALID_SOCKET);

	m_address.sin_family = AF_INET;
	m_address.sin_port = htons(port);
	m_address.sin_addr.s_addr = inet_addr(address);

	if (m_address.sin_addr.s_addr == INADDR_NONE) {
		return SocketStatus::InvalidAddress;
	}

	m_handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_handle == INVALID_SOCKET) {
		return SocketStatus::CreateFailed;
	}

	if (bind(m_handle, reinterpret_cast<const SOCKADDR*>(&m_address), sizeof(m_address)) != 0) {
		closesocket(m_handle);
		m_handle = INVALID_SOCKET;

		return SocketStatus::BindFailed;
	}

	if (listen(m_handle, BACKLOG_COUNT) != 0) {
		closesocket(m_handle);
		m_handle = INVALID_SOCKET;

		return SocketStatus::ListenFailed;
	}

	return SocketStatus::Success;
}

Socket Socket::Accept() {
	assert(m_handle != INVALID_SOCKET);

	SOCKET handle;
	SOCKADDR_IN address;
	int addressSize = sizeof(SOCKADDR_IN);

	handle = accept(m_handle, reinterpret_cast<SOCKADDR*>(&address), &addressSize);

	return Socket(handle, address);
}

bool Socket::HasAcceptRequest() const {
	if (m_handle != 0) {
		fd_set set;
		timeval timeout;

		timeout.tv_sec = timeout.tv_usec = 0;

		FD_ZERO(&set);
		FD_SET(m_handle, &set);

		select(1, &set, nullptr, nullptr, &timeout);

		return FD_ISSET(m_handle, &set);
	}
	else {
		return false;
	}
}

void Socket::Close() {
	if (m_handle != INVALID_SOCKET) {
		shutdown(m_handle, SD_BOTH);
		closesocket(m_handle);

		m_handle = INVALID_SOCKET;
	}
}

SocketStatus Socket::Read(char &c) {
	assert(m_handle != INVALID_SOCKET);

	return ReadBuffer(&c, 1);
}

SocketStatus Socket::Read(short &s) {
	assert(m_handle != INVALID_SOCKET);

	union {
		short value;
		char buffer[sizeof(short)];
	};

	auto status = ReadBuffer(buffer, sizeof(short));

	if (status == SocketStatus::Success) {
		s = value;
	}

	return status;
}

SocketStatus Socket::Read(int &i) {
	assert(m_handle != INVALID_SOCKET);

	union {
		int value;
		char buffer[sizeof(int)];
	};

	auto status = ReadBuffer(buffer, sizeof(int));

	if (status == SocketStatus::Success) {
		i = value;
	}

	return status;
}

SocketStatus Socket::Read(std::string &s) {
	assert(m_handle != INVALID_SOCKET);

	char length;
	auto status = Read(length);
	auto buffer = new char[length];

	if (status != SocketStatus::Success) {
		return status;
	}

	status = ReadBuffer(buffer, length);

	s.assign(buffer, length);

	return status;
}

SocketStatus Socket::Write(char c) {
	assert(m_handle != INVALID_SOCKET);

	return WriteBuffer(&c, 1);
}

SocketStatus Socket::Write(short s) {
	assert(m_handle != INVALID_SOCKET);

	union {
		short value;
		char buffer[sizeof(short)];
	};

	value = s;

	return WriteBuffer(buffer, sizeof(short));
}

SocketStatus Socket::Write(int i) {
	assert(m_handle != INVALID_SOCKET);

	union {
		int value;
		char buffer[sizeof(int)];
	};

	value = i;

	return WriteBuffer(buffer, sizeof(int));
}

SocketStatus Socket::Write(const std::string & s) {
	assert(m_handle != INVALID_SOCKET);

	assert(s.length() <= std::numeric_limits<char>::max());

	auto length = static_cast<char>(s.length());

	auto status = Write(length);

	if (status == SocketStatus::Success) {
		status = WriteBuffer(s.c_str(), length);
	}

	return status;
}

// static
const char* Socket::StatusToString(SocketStatus status) {
	static const char* ErrorToString[] = {
		"Success",
		"Invalid address",
		"Socket creation failed",
		"Socket bind failed",
		"Socket listen failed",
		"Connection failed",
		"Connection refused"
	};

	return ErrorToString[static_cast<size_t>(status)];
}

SocketStatus Socket::ReadBuffer(char* buffer, int bufferLength) {
	int readLength;
	
	do {
		readLength = recv(m_handle, buffer, bufferLength, 0);

		if (readLength < 0) {
			return SocketStatus::ConnectionRefused;
		}

		buffer += readLength;
		bufferLength -= readLength;
	} while (bufferLength > 0);

	return SocketStatus::Success;
}

SocketStatus Socket::WriteBuffer(const char* buffer, int bufferLength) {
	int writtenLength;

	do {
		writtenLength = send(m_handle, buffer, bufferLength, 0);

		if (writtenLength < 0) {
			return SocketStatus::ConnectionRefused;
		}

		buffer += writtenLength;
		bufferLength -= writtenLength;
	} while (bufferLength > 0);

	return SocketStatus::Success;
}