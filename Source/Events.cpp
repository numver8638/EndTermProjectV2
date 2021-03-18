#include "Events.h"

// 소켓에 데이터를 쓰는 매크로. 오류 코드 반환시 바로 반환.
#define WRITE(value) \
	do { \
		if ((status = socket.Write((value))) != SocketStatus::Success) \
			return status; \
	} while (false)

// 소켓에서 데이터를 읽는 매크로. 오류 코드 반환시 바로 반환.
#define READ(value) \
	do { \
		if ((status = socket.Read((value))) != SocketStatus::Success) { \
			event = nullptr; \
			return status; \
		} \
	} while (false)

// CreateBlockEvent 직렬화 코드.
SocketStatus CreateBlockEvent::Serialize(Socket& socket) const {
	SocketStatus status;

	WRITE(static_cast<char>(GetEventID()));
	WRITE(static_cast<char>(m_kind));
	WRITE(static_cast<char>(m_direction));
	WRITE(m_pos.X);
	WRITE(m_pos.Y);

	return status;
}

// MoveBlockEvent 직렬화 코드.
SocketStatus MoveBlockEvent::Serialize(Socket& socket) const {
	SocketStatus status;

	WRITE(static_cast<char>(GetEventID()));
	WRITE(static_cast<char>(m_direction));
	WRITE(m_pos.X);
	WRITE(m_pos.Y);

	return status;
}

// KeyPressEvent 직렬화 코드.
SocketStatus KeyPressEvent::Serialize(Socket& socket) const {
	SocketStatus status;

	WRITE(static_cast<char>(GetEventID()));
	WRITE(static_cast<char>(m_key));

	return status;
}

// LoginEvent 직렬화 코드.
SocketStatus LoginEvent::Serialize(Socket& socket) const {
	SocketStatus status;

	WRITE(static_cast<char>(GetEventID()));
	WRITE(m_name);
	WRITE(m_speed);

	return status;
}

// GameInitEvent 직렬화 코드.
SocketStatus GameInitEvent::Serialize(Socket& socket) const {
	SocketStatus status;

	WRITE(static_cast<char>(GetEventID()));

	return status;
}

// GameStartEvent 직렬화 코드.
SocketStatus GameStartEvent::Serialize(Socket& socket) const {
	SocketStatus status;

	WRITE(static_cast<char>(GetEventID()));

	return status;
}

// GameOverEvent 직렬화 코드.
SocketStatus GameOverEvent::Serialize(Socket& socket) const {
	SocketStatus status;

	WRITE(static_cast<char>(GetEventID()));
	WRITE(m_cause);

	return status;
}

// ClientDisconnectEvent 직렬화 코드.
SocketStatus ClientDisconnectEvent::Serialize(Socket& socket) const {
	SocketStatus status;

	WRITE(static_cast<char>(GetEventID()));
	WRITE(m_cause);

	return status;
}

// ServerDisconnectEvent 직렬화 코드.
SocketStatus ServerDisconnectEvent::Serialize(Socket& socket) const {
	SocketStatus status;

	WRITE(static_cast<char>(GetEventID()));
	WRITE(m_cause);

	return status;
}

// CreateBlockEvent 역직렬화 코드.
// static
SocketStatus CreateBlockEvent::Deserialize(Socket& socket, Event* &event) {
	SocketStatus status;
	
	char kind, direction;
	int x, y;

	READ(kind);
	READ(direction);
	READ(x);
	READ(y);

	event = new CreateBlockEvent(static_cast<BlockKind>(kind), static_cast<BlockDirection>(direction), Position(x, y));

	return status;
}

// MoveBlockEvent 역직렬화 코드.
// static
SocketStatus MoveBlockEvent::Deserialize(Socket& socket, Event* &event) {
	SocketStatus status;

	char direction;
	int x, y;

	READ(direction);
	READ(x);
	READ(y);

	event = new MoveBlockEvent(static_cast<BlockDirection>(direction), Position(x, y));

	return status;
}

// KeyPressEvent 역직렬화 코드.
// static
SocketStatus KeyPressEvent::Deserialize(Socket& socket, Event* &event) {
	SocketStatus status;

	char key;

	READ(key);

	event = new KeyPressEvent(static_cast<Key>(key));

	return status;
}

// LoginEvent 역직렬화 코드.
// static
SocketStatus LoginEvent::Deserialize(Socket& socket, Event* &event) {
	SocketStatus status;

	std::string name;
	short speed;

	READ(name);
	READ(speed);

	event = new LoginEvent(std::move(name), speed);

	return status;
}

// GameInitEvent 역직렬화 코드.
// static
SocketStatus GameInitEvent::Deserialize(Socket& socket, Event* &event) {
	event = new GameInitEvent();

	return SocketStatus::Success;
}

// GameStartEvent 역직렬화 코드.
// static
SocketStatus GameStartEvent::Deserialize(Socket& socket, Event* &event) {
	event = new GameStartEvent();

	return SocketStatus::Success;
}

// GameOverEvent 역직렬화 코드.
// static
SocketStatus GameOverEvent::Deserialize(Socket& socket, Event* &event) {
	SocketStatus status;

	std::string cause;

	READ(cause);

	event = new GameOverEvent(std::move(cause));

	return status;
}

// ClientDisconnectEvent 역직렬화 코드.
// static
SocketStatus ClientDisconnectEvent::Deserialize(Socket& socket, Event* &event) {
	SocketStatus status;

	std::string cause;

	READ(cause);

	event = new ClientDisconnectEvent(std::move(cause));

	return status;
}

// ServerDisconnectEvent 역직렬화 코드.
// static
SocketStatus ServerDisconnectEvent::Deserialize(Socket& socket, Event* &event) {
	SocketStatus status;

	std::string cause;

	READ(cause);

	event = new ServerDisconnectEvent(std::move(cause));

	return status;
}