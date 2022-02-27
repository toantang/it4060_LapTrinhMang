#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <string.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

const int BUFF_SIZE = 1024;
WSADATA wsaData;
SOCKET listenSock;
sockaddr_in serverAddr;

int initWinsock() {
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void constructorSocket() {
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

int bindAddressSocket() {
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5500);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

	return bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr));
	/*if (bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		cout << "Error! Cannot bind this address" << endl;
		return 0;
	}
	return 1;*/
}

int listenRequestFromClient() {

	return listen(listenSock, 10);
	/*if (listen(listenSock, 10)) {
		cout << "Error when listen request from client" << endl;
		return 0;
	}
	return 1;*/
}

void communicateWithClient() {
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE];
	char clientIP[INET_ADDRSTRLEN];
	int ret;
	int clientAddrLen = sizeof(clientAddr);
	int clientPort;
	SOCKET connSock;

	//accept request
	connSock = accept(listenSock, (sockaddr *)&clientAddr, &clientAddrLen);
	inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
	clientPort = ntohs(clientAddr.sin_port);

	while (true) {
		//receive message from client
		ret = recv(connSock, buff, BUFF_SIZE, 0);
		buff[ret] = 0;

		if (ret == SOCKET_ERROR) {
			cout << "Error: " << WSAGetLastError() << endl;
			break;
		}
		else {
			
			cout << "Receive from client: " << endl;
			cout << "	clientIP: " << clientIP << endl;
			cout << "	clientPort: " << clientPort << endl;
			cout << "	size of buff: " << strlen(buff) << endl;
			cout << "	client message: " << buff << endl;


			//Echo to Client
			ret = send(connSock, buff, ret, 0);
			if (ret == SOCKET_ERROR) {
				cout << "Error when echo to client: " << WSAGetLastError() << endl;
				return;
			}

		}
	}
}


int main() {
	if (initWinsock() == 0) {
		constructorSocket();
		if (bindAddressSocket() == 0) {
			if (listenRequestFromClient() == 0) {
				communicateWithClient();
			}
		}
	}
	return 0;
}