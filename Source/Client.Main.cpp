#include <iostream>

#include <WinSock2.h>

#include "Client.h"

// 클라이언트 메인 함수.
int main(int argc, char** argv) {
	WSADATA data;
	auto status = WSAStartup(WINSOCK_VERSION, &data);

	if (status != 0) {
		std::cout << "Fail to initialize socket." << std::endl;
	}
	else {
		Client client;
		status = client.Start();
		WSACleanup();
	}

	return status;
}