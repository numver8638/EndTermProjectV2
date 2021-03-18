#include "Server.h"

#include <algorithm>
#include <cassert>
#include <ctime>
#include <cstdlib>
#include <iostream>

#include "ConnectionState.h"
#include "PacketStream.h"
#include "Socket.h"
#include "Thread.h"

#define MIN_WIDTH	(0)
#define MAX_WIDTH	(10)
#define MIN_HEIGHT	(0)
#define MAX_HEIGHT	(10)

#define BUFFER_HEIGHT (4)

// 클라이언트와 통신하는 스레드. 클라이언트 당 하나씩 생성됨.
class ClientThread : public Thread {
private:
	Socket m_socket;
	PacketStream m_stream;

	std::string m_username;
	ConnectionState m_state = ConnectionState::Login;
	bool m_connected = true;

	std::vector<Block> m_blocks;
	Block m_currentBlock;
	int m_maxHeight;

	// 타이머 정보
	DWORD m_ticks;
	DWORD m_tickCount;
	DWORD m_tickBase;

	void Disconnect() {
		m_connected = false;
		m_socket.Close();
	}

	// 현재 블럭 상태에서 주어진 위치로 블럭을 이동 할 수 있는지 판단하는 메서드.
	bool CanMove(const Position& pos, const BlockData& data) {
		std::vector<bool> plane(MAX_WIDTH * MAX_HEIGHT);

		// 현재 쌓인 블록을 기준으로 bool vector 생성.
		auto it = m_blocks.begin();
		const auto end = --m_blocks.end(); // 마지막 블럭은 현재 블럭(움직이고 있는 블럭)으로 취급함.
		
		while (it != end) {
			auto& b = *(it++);
			const auto& pos = b.GetPosition();

			for (const auto& p : b.GetData().Data) {
				if ((pos.Y + p.first.Y) >= 0) {
					plane[(pos.Y + p.first.Y) * MAX_WIDTH + (pos.X + p.first.X)] = true;
				}
			}
		}

		// 위에서 생성한 bool vector와 현재 블럭이 겹치는지 판단.
		auto collide = false;

		for (const auto& p : data.Data) {
			// 블럭이 위에서 내려오는 경우 일부가 잘릴수도 있음.
			// 그런 예외 상황을 고려해서 Y가 0 이상인 부분만 판단.
			if ((pos.Y + p.first.Y) >= 0) {
				collide |= plane[(pos.Y + p.first.Y) * MAX_WIDTH + (pos.X + p.first.X)];
			}
		}

		// 충돌하지 않는다면 옮길 수 있고, 아니라면 옮길 수 없음.
		return !collide;
	}

	void SendCreateBlock() {
		// Send random block
		auto block = Block::GenerateRandomBlock();

		// 블럭이 X축 밖으로 넘어가지 않는 범위에서 랜덤으로 X 위치를 결정함.
		auto pos = Position(rand() % (MAX_WIDTH - block.GetData().Width), 0);

		// 블럭의 위치를 설정.
		block.SetAbsolutePosition(pos);

		// 클라이언트에게 전송.
		m_stream.SendEvent(CreateBlockEvent(block.GetKind(), block.GetDirection(), block.GetPosition()));

		// 서버에 저장.
		m_blocks.push_back(block);
	}

	void CheckBlocks() {
		auto overflow = false;
		std::vector<bool> plane(MAX_WIDTH * MAX_HEIGHT);

		// 현재 쌓인 블록을 기준으로 bool vector 생성.
		auto it = m_blocks.begin();
		const auto end = --m_blocks.end(); // 마지막 블럭은 현재 블럭(움직이고 있는 블럭)으로 취급함.

		while (it != end) {
			auto& b = *(it++);
			const auto& pos = b.GetPosition();
			
			for (const auto& p : b.GetData().Data) {
				// 블럭이 XY 범위를 넘어간다면 게임 실패 조건 성립.
				// Y == 0인 경우 블럭이 천장에 닿은 경우이므로 이 경우도 게임 실패.
				if ((pos.Y + p.first.Y) > 0) {
					plane[(pos.Y + p.first.Y) * MAX_WIDTH + (pos.X + p.first.X)] = true;
				}
				else {
					overflow = true;
				}
			}
		}

		// 실패 조건 판별
		// 블럭이 최고 높이에 도달하거나 초과함.
		if (overflow) {
			m_state = ConnectionState::GameOver;
			m_stream.SendEvent(GameOverEvent("Block overflow."));
		}

		// 성공 조건 판별
		// 구성면이 모두 블럭으로 참.
		auto succeed = true;
		for (auto i = (MAX_WIDTH * (MAX_HEIGHT - m_maxHeight)); i < (MAX_WIDTH * MAX_HEIGHT); i++) {
			succeed &= plane[i];
		}

		if (succeed) {
			m_state = ConnectionState::GameOver;
			m_stream.SendEvent(GameOverEvent("You won!"));
		}
	}

	void MoveBlock(Key key) {
		auto& block = m_blocks.back(); // 복사본 하나 생성.

		auto pos = block.GetPosition();
		auto delta = Position();
		auto direction = block.GetDirection();

		// 사용자 키 입력에 따라 블럭 변화 처리.
		switch (key) {
		case Key::Right:
			delta.X = 1;
			break;

		case Key::Left:
			delta.X = -1;
			break;

		case Key::Down:
			delta.Y = 1;
			break;

		case Key::Space:
			direction = (direction == BlockDirection::Horizontal) ? BlockDirection::Vertical : BlockDirection::Horizontal;
			break;

		default:
			return;
		}

		// 위에서 위치 데이터 반영한 새로운 블록 데이터 가져옴.
		auto& data = Block::GetData(block.GetKind(), direction);

		pos += delta;

		// 블록이 X 범위 밖으로 나가는 경우 위치 보정.
		if (pos.X < 0) {
			pos.X = 0;
		}

		if ((pos.X + data.Width) > MAX_WIDTH) {
			pos.X = MAX_WIDTH - data.Width;
		}

		if (pos.Y >= MAX_HEIGHT) {
			pos.Y = MAX_HEIGHT - 1;
		}

		// 옮길 위치가 이동 가능할 경우 현재 데이터에 반영하고 이벤트 전송.
		if (CanMove(pos, data)) {
			block.SetAbsolutePosition(pos);
			block.SetDirection(direction);

			MoveBlockEvent e(direction, pos);

			m_stream.SendEvent(e);

			std::printf("[thread %d] User pressed %d key. Move block to (%d,%d).\n",
						GetThreadID(), static_cast<int>(key), pos.X, pos.Y);
		}
		else if (delta.Y > 0) {
			// 움직일 수 없는 상태서 y 변위가 있을경우 바닥에 도달한 것으로 간주.
			// 새로운 블럭을 생성함.
			std::printf("[thread %d] Reached to the base(%d,%d). Create next block.\n",
						GetThreadID(), pos.X, pos.Y);
			SendCreateBlock();
		}
		else {
			// 블럭을 움직 일 수 없음(디버깅용 메세지)
			std::printf("[thread %d] Cannot move block to (%d, %d). Ignore it.\n",
						GetThreadID(), pos.X, pos.Y);
		}

		// 현재 블록 상태 점검.
		CheckBlocks();
	}

	// 타이머 이벤트. 사용자가 지정한 주기가 되면 이벤트가 발생함.
	void OnTimer() {
		// 현재 게임 상태에 따라 다르게 행동.
		switch (m_state) {
			// 게임 초기화 단계
		case ConnectionState::GameInit: {
			int width = 0;
			int height = 0;

			// 현재까지의 블럭들을 순회하면서 최고 높이와 현재 폭을 계산함.
			for (const auto& block : m_blocks) {
				const auto& data = block.GetData();

				width += data.Width;
				height = max(height, data.Height);
			}

			if (width == MAX_WIDTH) {
				// Building initial state complete.
				// Switch to GameStart state.
				m_maxHeight = height;

				m_state = ConnectionState::GameStart;
				m_stream.SendEvent(GameStartEvent());

				SendCreateBlock();
			}
			else {
				// 적합한 블럭을 생성할 때 까지 반복.
				while (true) {
					auto block = Block::GenerateRandomBlock(true);

					// 현재 남은 공간에 적합한 블럭인지 확인하고
					// 적합한 경우 블럭 위치를 설정하고 블럭 생성 이벤트를 전송.
					if (block.GetData().Width <= (MAX_WIDTH - width)) {
						block.SetAbsolutePosition(Position(width, MAX_HEIGHT - 1));

						m_blocks.push_back(block);
						m_stream.SendEvent(CreateBlockEvent(block.GetKind(), block.GetDirection(), block.GetPosition()));
						
						break;
					}
				}
			}

			break;
		}

		// 게임 시작 단계.
		case ConnectionState::GameStart:
			MoveBlock(Key::Down);
			break;

		default:
			// Ignore it.
			break;
		}
	}

	void OnKeyPress(const KeyPressEvent* event) {
		// 게임 시작 단계에서만 사용자 입력을 처리함.
		if (m_state != ConnectionState::GameStart) {
			std::printf("[thread %d] User pressed key but not in GameStart state. Ignore it.\n", GetThreadID());
		}
		else {
			MoveBlock(event->GetKey());
		}
	}

	void OnLogin(const LoginEvent* event) {
		// 로그인 상태서 로그인 이벤트 발생시 연결 해제(오류로 취급).
		if (m_state != ConnectionState::Login) {
			std::printf("[thread %d] User sent LoginEvent while logged in.\n", GetThreadID());
			ServerDisconnectEvent event("you're already logged in.");

			m_stream.SendEvent(event);

			Disconnect();
		}
		else {
			// 로그인 성공. 게임 초기화 단계로 넘어 감.
			std::printf("[thread %d] User %s logged in. (speed: %d)\n", GetThreadID(), event->GetName().c_str(), event->GetSpeed());

			m_username = event->GetName();
			m_tickBase = event->GetSpeed();

			m_state = ConnectionState::GameInit;

			GameInitEvent event;

			m_stream.SendEvent(event);
		}
	}

	void OnClientDisconnect(const ClientDisconnectEvent* event) {
		// 클라이언트에서 연결 종료. 서버도 연결 종료함.
		std::printf("[thread %d] User disconnected. cause: %s\n", GetThreadID(), event->GetCuase().c_str());
		Disconnect();
	}

public:
	ClientThread(Server& server, Socket socket)
	: m_socket(std::move(socket)), m_stream(m_socket) {}

	int Run() {
		assert(m_socket.IsConnected());

		// Set random seed.
		srand(time(NULL));

		m_tickCount = GetTickCount();
		DWORD deltaSum = 0;

		while (m_connected) {
			auto current = GetTickCount();

			auto delta = (current - m_tickCount);
			m_ticks += delta;
			deltaSum += delta;
			m_tickCount = current;

			// 정해진 주기에 도달하면 타이머 이벤트 호출.
			if (deltaSum > m_tickBase) {
				deltaSum = 0;
				OnTimer();
			}

			// 이벤트가 들어 온 경우 이벤트 처리.
			if (m_socket.HasAcceptRequest()) {
				Event* event = nullptr;
				auto status = m_stream.ReceiveEvent(event);

				if (status == SocketStatus::Success) {
					// 클라이언트가 보낸 이벤트의 수신에 성공했다면
					// 수신된 이벤트에 따라 이벤트 처리 코드 호출. 
					switch (event->GetEventID()) {
					case EventID::KeyPress:
						OnKeyPress(static_cast<const KeyPressEvent*>(event));
						break;

					case EventID::Login:
						OnLogin(static_cast<const LoginEvent*>(event));
						break;

					case EventID::ClientDisconnect:
						OnClientDisconnect(static_cast<const ClientDisconnectEvent*>(event));
						break;

					default:
						std::printf("[thread %d] user %s: Not-to-be-sent packet %d is received from client. Ignore it.\n",
							GetThreadID(), m_username.c_str(), static_cast<int>(event->GetEventID()));
						break;
					}

					delete event;
				}
				else {
					// 수신 실패. 연결 종료.
					std::printf("[thread %d] Connection error: %s\n", GetThreadID(), Socket::StatusToString(status));
					Disconnect();
				}
			}
		}

		return 0;
	}
};

int Server::Start() {
	Socket listenSocket;

	std::string ip;
	unsigned short port;
	char buffer[16 + 1];

	std::cout << "Server IP (default: localhost): ";
	std::cin.getline(buffer, sizeof(buffer));

	if (buffer[0] != '\0') {
		ip.assign(buffer);
	}
	else {
		ip = "127.0.0.1";
	}

	std::cout << "Server Port (default: 50000): ";
	std::cin.getline(buffer, sizeof(buffer));

	if (buffer[0] != '\0') {
		port = std::atoi(buffer);
	}
	else {
		port = 50000;
	}

	auto status = listenSocket.Bind(ip.c_str(), port);

	if (status != SocketStatus::Success) {
		std::cout << "Socket error: " << Socket::StatusToString(status) << std::endl;

		return 0;
	}

	std::cout << "Created server on " << ip << ":" << port << std::endl;

	const auto searchFunction = [this](Thread* thread) -> bool { return !thread->IsRunning(); };

	while (true) {
		// 클라이언트 접속 요청이 있으면 수락.
		if (listenSocket.HasAcceptRequest()) {
			auto thread = new ClientThread(*this, listenSocket.Accept());
			m_threads.emplace(thread);

			thread->Start();
		}

		// 종료된 스레드가 있으면 자원 정리.
		auto it = std::find_if(m_threads.begin(), m_threads.end(), searchFunction);

		while (it != m_threads.end()) {
			Thread* thread = *it;

			std::cout << "Removing terminated thread " << thread->GetThreadID() << std::endl;

			thread->Wait();
			delete thread;
			
			it = m_threads.erase(it);
			it = std::find_if(it, m_threads.end(), searchFunction);
		}
	}

	return 0;
}