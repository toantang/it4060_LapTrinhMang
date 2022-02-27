#include <iostream>
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <string.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

const int BUFF_SIZE = 1024;
WSAData wsaData;
SOCKET client;
sockaddr_in serverAddr;

int initWinsock() {
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void constructorSocket() {
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int tv = 10000; // timeout interval 10000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(int));
}

void specifyServerAddress() {
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5500);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
}

int requestToConnectServer() {
	return connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr));
}

void communicateWithServer() {
	char buff[BUFF_SIZE];
	int ret;
	int messageLen;
	while (true) {
		cout << "send to server: ";
		gets_s(buff, BUFF_SIZE);
		messageLen = strlen(buff);
		cout << "message to server: " << buff << ", len = " << messageLen << endl;
		if (messageLen == 0) {
			break;
		}
		ret = send(client, buff, messageLen, 0);

		if (ret == SOCKET_ERROR) {
			cout << "Error when send message to server: " << WSAGetLastError() << endl;
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
			else if (strlen(buff) > 0) {
				buff[ret] = '\0';
				cout << "Receive from server: " << buff << endl;
			}
			else {
				cout << "Error!!!" << endl;
			}
		}

	}
}
int main() {
	if (initWinsock() == 0) {
		constructorSocket();
		specifyServerAddress();
		if (requestToConnectServer() == 0) {
			communicateWithServer();
		}
	}
	else {
		cout << "Error: " << WSAGetLastError() << endl;
	}
	return 0;
}