#include "EventListener.h"

#include "Client.h"

EventListener::EventListener(Client* client)
	: m_client(client), m_stream(client->GetSocket()) {}

int EventListener::Run() {
	assert(m_stream.IsConnected());

	SocketStatus status;

	do {
		Event* event = nullptr;
		status = m_stream.ReceiveEvent(event);

		if (status != SocketStatus::Success) {
			// 별도의 로직 분리 없이 이벤트로 서버와 연결 끊김을 나타냄.
			// 연결 끊김 이후 이벤트 리스너 종료.
			event = new ServerDisconnectEvent("Connection error.");
		}

		m_client->PushEvent(event);
	} while (status == SocketStatus::Success);

	return 0;
}