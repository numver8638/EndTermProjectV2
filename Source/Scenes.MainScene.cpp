#define _CRT_SECURE_NO_WARNINGS

#include "Scenes.h"

#include "ConsoleUtils.h"
#include "Client.h"
#include "StringUtils.h"

#define INDEX_IP		(0)
#define INDEX_PORT		(1)
#define INDEX_NAME		(2)
#define INDEX_TIMING	(3)

#define MAX_INDEX		(4)
#define MIN_INDEX		(0)

const wchar_t* ErrorMessages[] = {
	L"Success",
	L"Invalid ip address.",
	L"Socket creation failed.",
	L"Fail to bind.",
	L"Fail to listen.",
	L"Connection failed.",
	L"Connection refused."
};

void MainScene::OnKeyPress(Key key) {
	// 오류메세지 소거.
	m_errormsg.clear();

	// 위쪽 메뉴로 이동
	if (key == Key::Up && m_index > MIN_INDEX) {
		--m_index;
	}

	// 아래쪽 메뉴로 이동
	if (key == Key::Down && m_index < MAX_INDEX) {
		++m_index;
	}

	// 입력 지움
	if (key == Key::Backspace) {
		if (m_index == INDEX_IP && !m_ip.empty()) {
			m_ip.pop_back();
		}

		if (m_index == INDEX_PORT && !m_port.empty()) {
			m_port.pop_back();
		}

		if (m_index == INDEX_NAME && !m_username.empty()) {
			m_username.pop_back();
		}

		if (m_index == INDEX_TIMING && !m_speed.empty()) {
			m_speed.pop_back();
		}
	}

	// 스페이스 키 지원.
	// 스페이스 키는 문자 입력 이벤트와 분리되어있어
	// 여기서 스페이스 문자 처리를 해줘야 함.
	if (key == Key::Space) {
		if (m_index == INDEX_NAME && m_username.length() < 16) {
			m_username.append(1, L' ');
		}
		else {
			m_errormsg = L"Whitespace is not allowed.";
		}
	}

	// 게임 플레이 시작
	if (key == Key::Enter && m_index == 4) {
		// 입력된 값이 있으면 입력된 값을 이용하고, 비어있다면 기본값 사용.
		std::string user, ip;
		int speed, port;

		if (m_username.empty()) {
			user = "user";
		}
		else {
			user = WStringToString(m_username);
		}

		if (m_ip.empty()) {
			ip = "127.0.0.1";
		}
		else {
			ip = WStringToString(m_ip);
		}

		speed = m_speed.empty() ? 1000 : std::stoi(m_speed);

		port = m_port.empty() ? 50000 : std::stoi(m_port);

		GetClient()->SetName(user);
		GetClient()->SetSpeed(speed);
		
		// 서버와 연결
		auto status = GetClient()->GetSocket().Connect(ip.c_str(), static_cast<unsigned short>(port));
		
		if (status != SocketStatus::Success) {
			// 오류시 오류메세지 표출.
			m_errormsg = ErrorMessages[static_cast<size_t>(status)];
		}
		else {
			// 정상 연결. Game scene으로 전환.
			GetClient()->ChangeScene(1);
		}
	}

	// 변경사항이 있으므로 재 렌더링 요청.
	GetClient()->RequestRender();
}

void MainScene::OnCharInput(wchar_t ch) {
	// 오류메세지 소거.
	m_errormsg.clear();

	// 현재 입력중인 index에 따라 다르게 입력 처리.
	switch (m_index) {
	case INDEX_IP:
		// ip는 최대 15글자 까지 (xxx.xxx.xxx.xxx) = 15
		if (m_ip.length() < 15) {
			m_ip.append(1, ch);
		}
		break;

	case INDEX_PORT:
		// 포트번호는 숫자만 허용하고, 0~65535 범위 내인지 검증.
		if (ch < L'0' || L'9' < ch) {
			m_errormsg = L"Only numbers allowed.";
		}
		else {
			m_port.append(1, ch);
		}

		if (!m_port.empty()) {
			auto port = std::stoi(m_port);

			if (port > 65536) {
				m_port.pop_back();
				m_errormsg = L"Allowed range of port is 0 ~ 65535.";
			}
		}
		break;

	case INDEX_NAME:
		// 플레이어 이름은 최대 16글자까지
		if (m_username.length() < 16) {
			m_username.append(1, ch);
		}
		break;

	case INDEX_TIMING:
		// 속도는 숫자만 허용하고, 1 ~ 1000 범위 내인지 검증.
		if (ch < L'0' || L'9' < ch) {
			m_errormsg = L"Only numbers allowed.";
		}
		else {
			m_speed.append(1, ch);
		}

		if (!m_speed.empty()) {
			auto timing = std::stoi(m_speed);

			if (timing < 1 || timing > 1000) {
				m_speed.pop_back();
				m_errormsg = L"Allowed range of timing is 1 ~ 1000.";
			}
		}
		break;

	default:
		// Do nothing.
		break;
	}

	GetClient()->RequestRender();
}

void MainScene::OnRender() {
	// 렌더링 코드.
	// 정해진 위치에 정해진 문자열을 렌더링 하고
	// 현재 입력중인 index에 맞게 커서를 옮김.

	DrawString(L"Server address (default: localhost): ", Position(3, 3));
	DrawString(m_ip, Position(3 + 37, 3));
	DrawString(L"Server port (default: 50000): ", Position(3, 4));
	DrawString(m_port, Position(3 + 30, 4));
	DrawString(L"Name (default: user): ", Position(3, 5));
	DrawString(m_username, Position(3 + 22, 5));
	DrawString(L"Timing (default: 1000): ", Position(3, 6));
	DrawString(m_speed, Position(3 + 24, 6));
	DrawString(L"Start", Position(3, 7));

	if (!m_errormsg.empty()) {
		DrawString(m_errormsg, Position(3, 8));
	}

	if (m_index == 0) {
		ToggleCursor(true);
		MoveCursor(Position(3 + 37 + m_ip.length(), 3));
	}
	else if (m_index == 1) {
		ToggleCursor(true);
		MoveCursor(Position(3 + 30 + m_port.length(), 4));
	}
	else if (m_index == 2) {
		ToggleCursor(true);
		MoveCursor(Position(3 + 22 + m_username.length(), 5));
	}
	else if (m_index == 3) {
		ToggleCursor(true);
		MoveCursor(Position(3 + 24 + m_speed.length(), 6));
	}
	else {
		ToggleCursor(false);
		MoveCursor(Position(0, 0));
	}

	DrawChar(L'>', Position(1, 3 + m_index));
}