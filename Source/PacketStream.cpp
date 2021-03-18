#include "PacketStream.h"

#include "Events.h"

// 이벤트 역 직렬화 함수 포인터 타입.
using Deserializer = SocketStatus(*)(Socket&, Event*&);

// 이벤트 역 직렬화 메서드 테이블
static const Deserializer DeserializeTable[] = {
	LoginEvent::Deserialize,
	GameInitEvent::Deserialize,
	GameStartEvent::Deserialize,
	GameOverEvent::Deserialize,
	CreateBlockEvent::Deserialize,
	MoveBlockEvent::Deserialize,
	KeyPressEvent::Deserialize,
	ClientDisconnectEvent::Deserialize,
	ServerDisconnectEvent::Deserialize
};

// 소켓에서 패킷을 읽어 이벤트로 반환하는 메서드.
SocketStatus PacketStream::ReceiveEvent(Event* &ret) {
	SocketStatus status;

	char id;

	status = m_socket.Read(id);

	if (status != SocketStatus::Success) {
		return status;
	}
	else if (0 <= id && id <= static_cast<size_t>(EventID::MaxEventID)) {
		const auto& handler = DeserializeTable[id];

		return handler(m_socket, ret);
	}
	else {
		// 알 수 없는 이벤트 타입. 연결 오류로 처리.
		std::printf("Unknown event id %d.\n", id);
		return SocketStatus::ConnectionRefused;
	}
}

// 이벤트를 패킷으로 만들어 전송하는 메서드.
SocketStatus PacketStream::SendEvent(const Event& event) {
	return event.Serialize(m_socket);
}