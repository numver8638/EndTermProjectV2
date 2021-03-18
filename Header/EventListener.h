#pragma once

#include "PacketStream.h"
#include "Thread.h"

class Client; // Defined in "Client.h"

// 서버에서 이벤트를 받아오는 스레드.
class EventListener : public Thread {
private:
	Client* m_client;
	PacketStream m_stream;

public:
	EventListener(Client* client);

	// 서버에서 이벤트를 받아 클라이언트 이벤트 큐에 삽입하는 메서드.
	int Run() override;
};