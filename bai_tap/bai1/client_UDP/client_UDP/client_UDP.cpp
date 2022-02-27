#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib");

#define SERVER_ADDR "127.0.0.1"

using namespace std;

const int BUFF_SIZE = 2048;
WSADATA wsaData;
SOCKET client;
sockaddr_in serverAddr;

int initWinsock() {
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void constructorSock() {
	client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

void specifyServerAddr() {
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5500);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
}

void receiveDataFrommServer(int ret, char buff[], int serverAddrLen) {
	ret = recvfrom(client, buff, BUFF_SIZE, 0, (sockaddr*)&serverAddr, &serverAddrLen);
	if (ret == SOCKET_ERROR) {
		cout << "Error: " << WSAGetLastError() << endl;
	}
	else {
		buff[ret] = '\0';
		cout << "message reiceve from server: " << buff << endl;
	}
	_strupr_s(buff, BUFF_SIZE);
}

void communicateWithServer() {
	char buff[BUFF_SIZE];
	int ret;
	int serverAddrLen = sizeof(serverAddr);

	do {
		cout << "Send to server" << endl;
		cout << "type ip address or domain name: ";
		gets_s(buff, BUFF_SIZE);
		ret = sendto(client, buff, strlen(buff), 0, (sockaddr*)&serverAddr, serverAddrLen);
		if (ret == SOCKET_ERROR) {
			cout << "Error: " << WSAGetLastError() << endl;
		}
		else {
			cout << "Data from server: " << endl;
			receiveDataFrommServer(ret, buff, serverAddrLen);
		}
	} while (strcmp(buff, " ") != 0);
}
int main() {
	if (initWinsock() == 0) {
		constructorSock();
		specifyServerAddr();
		communicateWithServer();
	}
	return 0;
}