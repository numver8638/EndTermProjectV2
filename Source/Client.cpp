#include "Client.h"

#include <cassert>
#include <conio.h>

#include "ConsoleUtils.h"

#define KEY_UP		(38)
#define KEY_DOWN	(40)
#define KEY_LEFT	(37)
#define KEY_RIGHT	(39)

#define KEY_ESC		(27)
#define KEY_SPACE	(' ')

int Client::Start() {
	HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);

	// 시작할 때  먼저 화면 초기화.
	ClearScreen();

	// 메인 화면으로 전환.
	ChangeScene(0);

	while (!m_appStop) {
		// Scene이 변경되었을 때 전 Scene은 종료 이벤트, 다음 Scene은 초기화 이벤트 호출.
		if (m_sceneChanged) {
			m_sceneChanged = false;

			if (m_oldScene != nullptr) {
				m_oldScene->OnFinalize();
			}

			m_currentScene->OnInitialize();
		}

		// 키 입력이 있으면 분석해서 그에 따른 키 이벤트 호출.
		DWORD inputs;
		GetNumberOfConsoleInputEvents(inputHandle, &inputs);

		if (inputs > 0) {
			// 키보드 키 눌림 이벤트만 받음.
			INPUT_RECORD input;
			DWORD inputCount;

			ReadConsoleInputW(inputHandle, &input, 1, &inputCount);

			// 오류 방지를 위해 너무 많은 키 입력은 무시.
			FlushConsoleInputBuffer(inputHandle);

			if (input.EventType == KEY_EVENT && input.Event.KeyEvent.bKeyDown == TRUE) {
				const auto& event = input.Event.KeyEvent;

				switch (event.wVirtualKeyCode) {
				case VK_ESCAPE:
					m_currentScene->OnKeyPress(Key::ESC);
					break;

				case VK_SPACE:
					m_currentScene->OnKeyPress(Key::Space);
					break;

				case VK_LEFT:
					m_currentScene->OnKeyPress(Key::Left);
					break;

				case VK_RIGHT:
					m_currentScene->OnKeyPress(Key::Right);
					break;

				case VK_UP:
					m_currentScene->OnKeyPress(Key::Up);
					break;

				case VK_DOWN:
					m_currentScene->OnKeyPress(Key::Down);
					break;

				case VK_BACK:
					m_currentScene->OnKeyPress(Key::Backspace);
					break;

				case VK_RETURN:
					m_currentScene->OnKeyPress(Key::Enter);
					break;

				default:
					// 특수키가 아니고 문자 값이 0이 아니라면
					// 문자 입력 처리 이벤트 호출.
					if (event.uChar.UnicodeChar != 0) {
						m_currentScene->OnCharInput(event.uChar.UnicodeChar);
					}

					// 나머지는 무시.

					break;
				}
			}
		}

		auto event = GetEvent();

		// 서버에서 보낸 이벤트가 있다면 이벤트 호출.
		if (event != nullptr) {
			m_currentScene->OnEvent(event);

			delete event;
		}

		// 화면 렌더링.
		if (m_renderRequest) {
			m_renderRequest = false;
			ClearScreen();
			m_currentScene->OnRender();
		}
	}

	return 0;
}