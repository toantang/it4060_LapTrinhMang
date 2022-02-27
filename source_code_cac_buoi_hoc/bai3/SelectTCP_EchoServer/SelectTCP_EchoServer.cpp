#include <iostream>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

const int BUFF_SIZE = 1024;
WSADATA wsaData;
SOCKET listenSock;
sockaddr_in serverAddr;

int initWinsock() {
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
}
void constructorSock() {
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

int bindAddressSock() {
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5500);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	return bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr));
}

int listenRequestFromClient() {
	return listen(listenSock, 10);
}

int receiveFromClient(SOCKET s, char* receiveBuff, int size, int flags) {
	int ret;
	ret = recv(s, receiveBuff, size, flags);
	receiveBuff[ret] = 0;
	if (ret == SOCKET_ERROR) {
		cout << WSAGetLastError() << endl;
	}
	
	return ret;
}

void processData(char* receiveBuff, char* sendBuff) {
	//sendBuff = receiveBuff;
	memcpy(sendBuff, receiveBuff, BUFF_SIZE);
}

int sendDataToClient(SOCKET s, char* sendBuff, int size, int flags) {
	int ret = send(s, sendBuff, size, flags);
	if (ret == SOCKET_ERROR) {
		cout << WSAGetLastError() << endl;
	}
	return ret;
}
void communicateWithClient() {
	cout << "Server started: " << endl;

	SOCKET client[FD_SETSIZE];
	SOCKET connSock;
	fd_set readfds; // tap cac fd_set thăm dò
	fd_set initfds; // khoi tao mot fd_set
	int ret;
	int nEvents; // số các socket đã xử lý xong
	sockaddr_in clientAddr;
	int clientAddrLen;
	char rcvBuff[BUFF_SIZE];
	char sendBuff[BUFF_SIZE];
	int clientPort;
	char clientIP[INET_ADDRSTRLEN];

	for (int i = 0; i < FD_SETSIZE; i++) {
		client[i] = 0;
	}

	FD_ZERO(&initfds);
	FD_SET(listenSock, &initfds); // truyền listenSock vào initfds

	while (true) {
		readfds = initfds; // gan tập readfds = initfds;
		nEvents = select(0, &readfds, 0, 0, 0); // so luong cac socket tham do
		if (nEvents < 0) {
			break;
		}
		if (FD_ISSET(listenSock, &readfds)) {
			clientAddrLen = sizeof(clientAddr);
			connSock = accept(listenSock, (sockaddr*)&clientAddr, &clientAddrLen);

			if (connSock < 0) {
				cout << "Error" << endl;
			}
			else {
				int i;
				for (i = 0; i < FD_SETSIZE; i++) {
					if (client[i] == 0) {
						client[i] = connSock;
						FD_SET(client[i], &initfds);
						break;
					}
				}

				if (i == FD_SETSIZE) {
					closesocket(connSock);
				}
				if (--nEvents == 0) {
					continue;
				}
			}
		}
		

		//receive data from client;
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (client[i] == 0) {
				continue;
			}
			if (FD_ISSET(client[i], &readfds)) {
				ret = receiveFromClient(client[i], rcvBuff, BUFF_SIZE, 0);

				if (ret <= 0) {
					FD_CLR(client[i], &initfds);
					closesocket(client[i]);
					client[i] = 0;
				}
				else {
					inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
					clientPort = ntohs(clientAddr.sin_port);
					cout << "Receive from client: " << endl;
					cout << "	clientIP: " << clientIP << endl;
					cout << "	clientPort: " << clientPort << endl;
					cout << "	size of buff: " << strlen(rcvBuff) << endl;
					cout << "	client message: " << rcvBuff << endl;

					processData(rcvBuff, sendBuff);
					sendDataToClient(client[i], sendBuff, ret, 0);
				}

			}
		}
	}
}
int main() {
	if (initWinsock() == 0) {
		constructorSock();
		if (bindAddressSock() == 0) {
			if (listenRequestFromClient() == 0) {
				communicateWithClient();
			}
		}
	}
	return 0;
}