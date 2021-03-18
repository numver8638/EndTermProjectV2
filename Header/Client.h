#pragma once

#include <cassert>
#include <queue>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Events.h"
#include "EventListener.h"
#include "PacketStream.h"
#include "Scenes.h"
#include "Socket.h"

class Client {
private:
	std::vector<Scene*> m_scenes;	// 현재 장면(스테이지)를 저장하는 벡터
	std::queue<Event*> m_events;	// 현재 서버로부터 들어온 이벤트를 저장하는 큐

	Socket m_socket;
	PacketStream m_stream;
	EventListener m_listener;

	// 렌더링 스레드 (본 스레드)와 이벤트 리스너 스레드간 변수 공유시 사용하는
	// 스레드 제어용 변수. - 이걸 우리말로 뭐라고 하는지..?
	CRITICAL_SECTION m_eventCriticalSection; 

	std::string m_name;	// 플레이어 이름
	short m_speed;		// 플레이어가 지정한 속도 (밀리세컨드 단위)

	Scene* m_oldScene = nullptr;		// 전 Scene. OnFinalize() 이벤트 호출을 위해 저장.
	Scene* m_currentScene = nullptr;	// 현재 Scene.
	bool m_sceneChanged = false;		// Scene이 변경 되었는지 나타내는 변수.

	bool m_appStop;
	bool m_renderRequest = true;	// 렌더링 요청 확인용 변수. 깜빡임을 방지하기 위해
									// Scene에서 요청이 있을 때만 렌더링을 함.

public:
	Client()
		: m_stream(m_socket), m_listener(this) {
		m_scenes.assign({ new MainScene(this), new GameScene(this) });

		InitializeCriticalSection(&m_eventCriticalSection);
	}

	~Client() {
		m_socket.Close();

		for (auto scene : m_scenes) {
			delete scene;
		}

		while (!m_events.empty()) {
			auto event = m_events.front();
			m_events.pop();

			delete event;
		}
	}

	// Scene을 변경하는 메서드.
	void ChangeScene(int sceneNo) {
		assert(0 <= sceneNo && sceneNo <= m_scenes.size());

		m_oldScene = m_currentScene;
		m_currentScene = m_scenes[sceneNo];
		m_sceneChanged = true;
	}

	// 이벤트 큐에 삽입하는 메서드.
	void PushEvent(Event* event) {
		EnterCriticalSection(&m_eventCriticalSection);

		m_events.push(event);

		LeaveCriticalSection(&m_eventCriticalSection);
	}

	// 서버에서 보낸 이벤트 큐에서 가져오는 메서드.
	Event* GetEvent() {
		Event* event = nullptr;

		EnterCriticalSection(&m_eventCriticalSection);

		if (!m_events.empty()) {
			event = m_events.front();
			m_events.pop();
		}

		LeaveCriticalSection(&m_eventCriticalSection);
	
		return event;
	}

	// 렌더링 요청 메서드.
	void RequestRender() { m_renderRequest = true; }

	// 클라이언트 종료 메서드.
	void StopApp() {
		m_appStop = true;
		m_socket.Close();
	}

	// 서버와 연결된 소켓을 가져오는 메서드.
	Socket& GetSocket() { return m_socket; }

	// 서버와 연결된 Stream을 가져오는 메서드
	PacketStream& GetStream() { return m_stream; }

	// 플레이어의 이름을 가져오는 메서드
	const std::string& GetName() const { return m_name; }

	// 플레이어의 이름을 저장하는 메서드
	void SetName(std::string name) { m_name = std::move(name); }

	// 플레이어의 현재 속도를 가져오는 메서드
	short GetSpeed() const { return m_speed; }

	// 플레이어의 속도를 저장하는 메서드
	void SetSpeed(short speed) { m_speed = speed; }

	// 이벤트 리스너 스레드를 시작하는 메서드
	void StartListener() { m_listener.Start(); }

	// 이벤트 리스너 스레드를 종료하는 메서드
	void StopListener() {
		m_listener.Stop();
		m_socket.Close();
	}

	// 클라이언트 시작 메서드.
	int Start();
};