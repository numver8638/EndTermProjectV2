#define _CRT_SECURE_NO_WARNINGS

#include "Scenes.h"

#include "Client.h"
#include "ConsoleUtils.h"
#include "Events.h"
#include "StringUtils.h"

static const std::wstring LoginString = L"Logging in...";		// 로그인 중일 때 출력할 문자열
static const std::wstring ErrorString = L"Connection Error: ";	// 연결 오류시 출력할 문자열
static const std::wstring GameOverString = L"Game Over";		// 게임 종료시 출력할 문자열

void GameScene::OnInitialize() {
	// 게임 Scene 초기화 코드. 매 번 전환될 때 실행.
	// 블럭 목록을 초기화하고 게임상태를 로그인 상태로 초기화 한 뒤
	// 서버에게 로그인 이벤트를 보내고 서버에서 보내는 이벤트를 받을 수 있도록
	// 이벤트 리스너 스레드를 시작함.
	assert(GetClient()->GetSocket().IsConnected());

	m_blocks.clear();
	m_state = ConnectionState::Login;

	GetClient()->GetStream().SendEvent(LoginEvent(GetClient()->GetName(), GetClient()->GetSpeed()));
	GetClient()->StartListener();
}

void GameScene::OnKeyPress(Key key) {
	if (key == Key::ESC) {
		if (m_state != ConnectionState::Disconnected || m_state != ConnectionState::Error) {
			// ESC키를 누를 시 서버에게 연결 종료를 보내고 메인화면으로 나감.
			GetClient()->GetStream().SendEvent(ClientDisconnectEvent("User exits game."));
			m_state = ConnectionState::Disconnected;
		}

		// 메인 화면으로 나감.
		GetClient()->ChangeScene(0);
	}
	else if (m_state == ConnectionState::GameStart) {
		if (key != Key::Up && key != Key::Backspace && key != Key::Enter) {
			// 위쪽 화살표 키와 백스페이스, 엔터를 제외한 나머지 키 입력은 서버로 전송.
			GetClient()->GetStream().SendEvent(KeyPressEvent(key));
		}
	}

	// 화면 전환이 있으므로 클라이언트에 렌더링 요청.
	GetClient()->RequestRender();
}

void GameScene::OnEvent(const Event* event) {
	// 이벤트 처리 코드. 이벤트 번호에 맞게 다시 캐스팅해서 각각의 메서드를 호출.
	switch (event->GetEventID()) {
	case EventID::CreateBlock:
		OnCreateBlock(static_cast<const CreateBlockEvent*>(event));
		break;

	case EventID::MoveBlock:
		OnMoveBlock(static_cast<const MoveBlockEvent*>(event));
		break;

	case EventID::GameInit:
		OnGameInit(static_cast<const GameInitEvent*>(event));
		break;

	case EventID::GameStart:
		OnGameStart(static_cast<const GameStartEvent*>(event));
		break;

	case EventID::GameOver:
		OnGameOver(static_cast<const GameOverEvent*>(event));
		break;

	case EventID::ServerDisconnect:
		OnServerDisconnect(static_cast<const ServerDisconnectEvent*>(event));
		break;

	default:
		// 클라이언트 전용 패킷 또는 알 수 없는 패킷 확인. 무시함.
		std::printf("[client] Not-to-be-sent packet %d is received from server. Ignore it.\n",
			static_cast<int>(event->GetEventID()));
		break;
	}

	// 변경 사항이 있으므로 클라이언트에게 렌더링 요청.
	GetClient()->RequestRender();
}

void GameScene::OnRender() {
	// 현재 상태에 맞게 블럭 또는 문자열을 렌더링.
	switch (m_state) {
	case ConnectionState::Login:
		DrawString(LoginString, Position(10 - (LoginString.length() / 2), 5));
		break;

	case ConnectionState::GameInit:
	case ConnectionState::GameStart:
		for (const auto& block : m_blocks) {
			for (const auto& pair : block.GetData().Data) {
				auto pos = block.GetPosition() + pair.first;
				
				// 위에서부터 블럭이 내려오면 일부 잘릴수도 있음.
				// Y가 음수이면 출력하지 않음.
				if (pos.Y >= 0) {
					// 특수부호가 2칸을 차치하므로 그 차이를 보정해서 출력함.
					pos.X *= 2;
					DrawChar(pair.second, pos);
				}
			}
		}
		break;

	case ConnectionState::GameOver:
		DrawString(GameOverString, Position(10 - (GameOverString.length() / 2), 5));
		DrawString(m_message, Position(10 - (m_message.length() / 2), 6));
		break;

	case ConnectionState::Error:
		DrawString(ErrorString, Position(10 - (ErrorString.length() / 2), 5));
		DrawString(m_message, Position(10 - (m_message.length() / 2), 6));
		break;
	}
}

void GameScene::OnFinalize() {
	// 이벤트 리스너 스레드 정지.
	GetClient()->StopListener();
}

void GameScene::OnCreateBlock(const CreateBlockEvent* event) {
	// 새로운 블럭 생성. 블럭 목록에 추가.
	m_blocks.emplace_back(event->GetBlock());
}

void GameScene::OnMoveBlock(const MoveBlockEvent* event) {
	assert(!m_blocks.empty() && "MoveBlockEvent must be raised after CreateBlockEvent.");

	// 블럭 목록의 마지막 블럭은 항상 현재 움직이고 있는 블럭을 나타냄.
	auto& block = m_blocks.back();

	block.SetAbsolutePosition(event->GetPosition());
	block.SetDirection(event->GetDirection());
}

void GameScene::OnGameInit(const GameInitEvent* event) {
	// 게임 초기화 이벤트. 현재 상태를 게임 초기화 상태로 전환.
	m_state = ConnectionState::GameInit;
}

void GameScene::OnGameStart(const GameStartEvent* event) {
	// 게임 시작 이벤트. 현재 상태를 게임 시작 상태로 전환.
	m_state = ConnectionState::GameStart;
}

void GameScene::OnGameOver(const GameOverEvent* event) {
	// 게임 종료 이벤트. 현재 상태를 게임 종료 상태로 전환하고
	// 사용자에게 게임 종료 사유를 출력(출력은 OnRender()에서 담당. 출력용 데이터만 보관).
	m_state = ConnectionState::GameOver;
	m_message = StringToWString(event->GetCause());
}

void GameScene::OnServerDisconnect(const ServerDisconnectEvent* event) {
	// 서버 연결 종료 이벤트. 서버에서 연결을 종료하는 경우는 서버에서
	// 오류가 발생한 경우이므로 현재 상태를 오류 상태로 전환하고
	// 사용자에게 게임 종료 사유를 출력(출력은 OnRender()에서 담당. 출력용 데이터만 보관).
	m_state = ConnectionState::Error;
	m_message = StringToWString(event->GetCause());
}