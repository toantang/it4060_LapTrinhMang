#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

const int BUFF_SIZE = 2048;

using namespace std;

WSAData wsaData;
SOCKET client;
sockaddr_in serverAddr; // dia chi socket cua server;

int initWinsock() {
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void constructorSocket() {
	client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int tv = 10000; // time out 10000ms = 10s
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));
}

void specifyServerAddress() {
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5500);
	string serverAddress = "127.0.0.1"; // dia chi cua server, neu server co dia chi 
	// 192.168.1.1 thi thay 120.0.0.1 thanh 192.168.1.1
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
}

void receiveEchoServer(char buff[BUFF_SIZE], int ret, int serverAddrLen) {
	ret = recvfrom(client, buff, BUFF_SIZE, 0, (sockaddr*)&serverAddr, &serverAddrLen);
	if (ret == SOCKET_ERROR) {
		cout << "da xay ra loi" << endl;
		if (WSAGetLastError() == WSAETIMEDOUT) {
			cout << "time out" << endl;
		}
		else {
			cout << "cannot receive from server" << endl;
		}
	}
	else {
		buff[ret] = '\0';
		cout << "message reiceve from server: " << buff << endl;
	}
	_strupr_s(buff, BUFF_SIZE);
}

void communicateWithServer() {
	char buff[BUFF_SIZE]; // data gui len server
	int ret;
	int serverAddrLen = sizeof(serverAddr);
	do {
		// Send message
		cout << "Send to server: ";
		gets_s(buff, BUFF_SIZE);
		cout << "du lieu nhap tu ban phim: " << buff << endl;
		ret = sendto(client, buff, strlen(buff), 0, (sockaddr *)&serverAddr, serverAddrLen);
		if (ret == SOCKET_ERROR) {
			cout << "Cannot send message" << endl;
		}
		else {
			cout << "data sended server: " << buff << endl;
			receiveEchoServer(buff, ret, serverAddrLen);
		}
	} while (strcmp(buff, "BYE") != 0);
}

int main(int argc, char* argv[]) {
	if (initWinsock() == 0) {
		constructorSocket();
		specifyServerAddress();
		communicateWithServer();
	}	
	return 0;
}