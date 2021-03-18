#pragma once

#include <cassert>
#include <string>

#include "Block.h"
#include "Key.h"
#include "Position.h"
#include "Socket.h"

// 이벤트 별 고유 번호.
enum class EventID : char {
	Login,
	GameInit,
	GameStart,
	GameOver,
	CreateBlock,
	MoveBlock,
	KeyPress,
	ClientDisconnect,
	ServerDisconnect,
	MaxEventID = ServerDisconnect
};

// 모든 이벤트들의 베이스 클래스.
class Event {
private:
	EventID m_id;

public:
	Event(EventID id)
		: m_id(id) {}

	EventID GetEventID() const { return m_id; }

	virtual ~Event() = default;

	virtual SocketStatus Serialize(Socket& socket) const = 0;
};

// 블록 생성 이벤트
class CreateBlockEvent : public Event {
private:
	BlockKind m_kind;
	BlockDirection m_direction;
	Position m_pos;

public:
	CreateBlockEvent(BlockKind kind, BlockDirection direction, Position pos)
		: Event(EventID::CreateBlock), m_kind(kind), m_direction(direction), m_pos(pos) {}

	// 이벤트 정보를 기반으로 Block 객체를 반환하는 메서드.
	Block GetBlock() const {
		return Block(m_kind, m_direction, m_pos);
	}

	SocketStatus Serialize(Socket& socket) const override;

	static SocketStatus Deserialize(Socket& socket, Event* &event);
};

// 블록 이동 이벤트
class MoveBlockEvent : public Event {
private:
	BlockDirection m_direction;
	Position m_pos;

public:
	MoveBlockEvent(BlockDirection direction, Position pos)
		: Event(EventID::MoveBlock), m_direction(direction), m_pos(pos) {}

	// 블록의 절대 위치.
	Position GetPosition() const { return m_pos; }

	// 블록의 방향.
	BlockDirection GetDirection() const { return m_direction; }

	SocketStatus Serialize(Socket& socket) const override;

	static SocketStatus Deserialize(Socket& socket, Event* &event);
};

// 키 입력 이벤트.
class KeyPressEvent : public Event {
private:
	Key m_key;

public:
	KeyPressEvent(Key key)
		: Event(EventID::KeyPress), m_key(key) {}

	// 플레이어가 입력한 키.
	Key GetKey() const { return m_key; }

	SocketStatus Serialize(Socket& socket) const override;

	static SocketStatus Deserialize(Socket& socket, Event* &event);
};

// 로그인 이벤트.
class LoginEvent : public Event {
private:
	std::string m_name;
	short m_speed;

public:
	LoginEvent(std::string name, short speed)
		: Event(EventID::Login), m_name(std::move(name)), m_speed(speed) {}

	// 플레이어 이름.
	const std::string& GetName() const { return m_name; }

	// 플레이 속도.
	short GetSpeed() const { return m_speed; }

	SocketStatus Serialize(Socket& socket) const override;

	static SocketStatus Deserialize(Socket& socket, Event* &event);
};

// 게임 초기화 이벤트.
class GameInitEvent : public Event {
public:
	GameInitEvent()
		: Event(EventID::GameInit) {}

	SocketStatus Serialize(Socket& socket) const override;

	static SocketStatus Deserialize(Socket& socket, Event* &event);
};

// 게임 시작 이벤트.
class GameStartEvent : public Event {
public:
	GameStartEvent()
		: Event(EventID::GameStart) {}

	SocketStatus Serialize(Socket& socket) const override;

	static SocketStatus Deserialize(Socket& socket, Event* &event);
};

// 게임 종료 이벤트.
class GameOverEvent : public Event {
private:
	std::string m_cause;

public:
	GameOverEvent(std::string cause)
		: Event(EventID::GameOver), m_cause(std::move(cause)) {}

	// 게임 종료 사유
	// 게임에서 이김, 블록이 천장까지 도달, 절대 성공할 수 없는 조건 등
	const std::string& GetCause() const { return m_cause; }

	SocketStatus Serialize(Socket& socket) const override;

	static SocketStatus Deserialize(Socket& socket, Event* &event);
};

// 클라이언트 연결 종료 이벤트. (클라이언트 -> 서버)
class ClientDisconnectEvent : public Event {
private:
	std::string m_cause;

public:
	ClientDisconnectEvent(std::string cause)
		: Event(EventID::ClientDisconnect), m_cause(std::move(cause)) {}

	// 클라이언트 종료 사유: 게임 종료 등
	const std::string& GetCuase() const { return m_cause; }

	SocketStatus Serialize(Socket& socket) const override;

	static SocketStatus Deserialize(Socket& socket, Event* &event);
};

// 서버 연결 종료 이벤트. (서버 -> 클라이언트)
class ServerDisconnectEvent : public Event {
private:
	std::string m_cause;

public:
	ServerDisconnectEvent(std::string cause)
		: Event(EventID::ServerDisconnect), m_cause(std::move(cause)) {}

	// 서버 종료 사유: 연결이 끊김, 서버가 종료됨 등
	const std::string& GetCause() const { return m_cause; }

	SocketStatus Serialize(Socket& socket) const override;

	static SocketStatus Deserialize(Socket& socket, Event* &event);
};