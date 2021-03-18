#include <iostream>

#include <WinSock2.h>

#include "Server.h"

// 서버 메인 함수.
int main(int argc, char** argv) {
	WSADATA data;
	auto status = WSAStartup(WINSOCK_VERSION, &data);

	if (status != 0) {
		std::cout << "Fail to initialize socket." << std::endl;
	}
	else {
		Server server;
		status = server.Start();
		WSACleanup();
	}

	return status;
}