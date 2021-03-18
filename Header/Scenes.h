#pragma once

#include <vector>

#include "ConnectionState.h"
#include "Block.h"
#include "Events.h"
#include "Key.h"

class Client;

// 게임 장면(혹은 스테이지)를 나타내는 클래스.
// Unity의 Scene에서 아이디어를 가져옴.
class Scene {
private:
	Client* const m_client;

public:
	Scene(Client* client)
		: m_client(client) {}

	virtual ~Scene() = default;

	virtual void OnInitialize() {}

	virtual void OnKeyPress(Key key) {}

	virtual void OnCharInput(wchar_t ch) {}

	virtual void OnEvent(const Event* event) {}

	virtual void OnRender() {}

	virtual void OnFinalize() {}

	Client* GetClient() const { return m_client; }
};

// 메인 화면을 나타내는 클래스.
class MainScene : public Scene {
private:
	std::wstring m_username;	// 플레이어 이름
	std::wstring m_speed;		// 속도
	std::wstring m_errormsg;	// 오류메세지
	std::wstring m_ip;			// 서버 IP
	std::wstring m_port;		// 서버 Port

	int m_index = 0;			// 현재 입력중인 Cursor

public:
	MainScene(Client* client)
		: Scene(client) {}

	void OnKeyPress(Key key) override;

	void OnCharInput(wchar_t ch) override;

	void OnRender() override;
};

// 게임 중 화면을 나타내는 클래스.
class GameScene : public Scene {
private:
	ConnectionState m_state = ConnectionState::Login;

	// 렌더링 전용. 모든 처리는 서버에서 함.
	std::vector<Block> m_blocks;
	Block m_currentBlock;

	std::wstring m_message;

	void OnCreateBlock(const CreateBlockEvent* event);

	void OnMoveBlock(const MoveBlockEvent* event);

	void OnGameInit(const GameInitEvent* event);

	void OnGameStart(const GameStartEvent* event);

	void OnGameOver(const GameOverEvent* event);

	void OnServerDisconnect(const ServerDisconnectEvent* event);

public:
	GameScene(Client* client)
		: Scene(client) {}

	void OnInitialize() override;

	void OnKeyPress(Key key) override;

	void OnEvent(const Event* event) override;

	void OnRender() override;

	void OnFinalize() override;
};