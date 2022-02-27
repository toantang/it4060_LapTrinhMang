#include <iostream>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;
const int BUFF_SIZE = 2048;
WSADATA wsaData;
SOCKET client;
sockaddr_in serverAddr;

int initWinsock() {
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void constructorSock() {
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int tv = 10000; // timeout interval 10000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(int));
}

void specifyServerAddress() {
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5500);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
}

int requestConnectToServer() {
	return connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr));
}
void communicateWithServer() {
	char buff[BUFF_SIZE];
	int ret;
	int messageLen;
	while (true) {
		cout << "Send to server: ";
		gets_s(buff, BUFF_SIZE);

		ret = send(client, buff, strlen(buff), 0);

		if (ret == SOCKET_ERROR) {
			cout << "Error: " << WSAGetLastError() << endl;
		}
		else {
			ret = recv(client, buff, BUFF_SIZE, 0);
			if (ret == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					cout << "Time out" << endl;
				}
				else {
					cout << "Error: " << WSAGetLastError() << endl;
				}
			}
			else {
				buff[ret] = 0;
				cout << "Echo from server: " << buff << endl;
			}
		}
	}
}
int main() {
	if (initWinsock() == 0) {
		constructorSock();
		specifyServerAddress();
		if (requestConnectToServer() == 0) {
			communicateWithServer();
		}
	}
	return 0;
}