#pragma once

#include "Events.h"
#include "Socket.h"

// 소켓에서 패킷을 읽고 쓰게 하는 클래스.
class PacketStream {
private:
	Socket& m_socket;

public:
	PacketStream(Socket& socket)
		: m_socket(socket) {}

	// 소켓에서 패킷을 읽어 이벤트로 반환하는 메서드.
	SocketStatus ReceiveEvent(Event* &ret);

	// 이벤트를 패킷으로 만들어 전송하는 메서드.
	SocketStatus SendEvent(const Event& event);

	// 현재 연결되어있는지 판단하는 메서드. Socket::IsConnected() 참고.
	bool IsConnected() const { return m_socket.IsConnected(); }
};