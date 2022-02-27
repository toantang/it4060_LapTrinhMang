#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;
const int MAX_CLIENT = 100;
const int BUFF_SIZE = 2048;

WSADATA wsaData;
SOCKET listenSock;
sockaddr_in serverAddr;

int initWinsock() {
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

int constructorSock() {
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	unsigned long ul = 1;
	/*if (ioctlsocket(listenSock, FIONBIO, (unsigned long*)&ul)) {
		cout << "Can not change to non-blocking mode" << endl;
	}*/
	return ioctlsocket(listenSock, FIONBIO, (unsigned long*)&ul);
}
int bindAddressSocket() {
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5500);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	return bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr));
}

int listenRequestFromClient() {
	return listen(listenSock, 10);
}

void communicateWithClient() {
	cout << "Server started: " << endl;
	SOCKET client[MAX_CLIENT]; // mảng socket lưu trữ các socket của nhiều client
	int numClient = 0; // so luong client hien tai
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	int ret;
	char buff[BUFF_SIZE];
	char clientIP[INET_ADDRSTRLEN];
	int clientPort;

	for (int i = 0; i < MAX_CLIENT; i++) {
		client[i] = 0;
	}

	/*for (int i = 0; i < MAX_CLIENT; i++) {
		cout << "client[" << i << "] = " << client[i] << endl;
	}*/
	while (true) {
		SOCKET connSock;
		
		connSock = accept(listenSock, (sockaddr*)&clientAddr, &clientAddrLen);
		
		if (connSock != SOCKET_ERROR) {
			if (numClient == MAX_CLIENT) {
				cout << "Error: Cannot response more client" << endl;
				closesocket(connSock);
			}
			else {
				int i = 0;
				while (i < MAX_CLIENT) {
					//cout << "client[" << i << "] = " << client[i] << endl;
					if (client[i] == 0) {
						client[i] = connSock;
						numClient++;
						cout << "vao ham nay" << endl;
						break;
					}
					i++;
				}
			}						
		}

		/*cout << "numClient: " << numClient << endl;
		for (int i = 0; i < numClient; i++) {
			cout << "client[" << i << "] = " << client[i] << endl;
		}*/
		for (int i = 0; i < numClient; i++) {
			if (client[i] == 0) continue;
			ret = recv(client[i], buff, BUFF_SIZE, 0);

			if (ret == SOCKET_ERROR) {
				if (WSAGetLastError() != WSAEWOULDBLOCK) {
					closesocket(client[i]);
				}
			}
			else if (ret == 0) {
				printf("Client disconnects");
				closesocket(client[i]);
				client[i] = 0;
			}
			else {
				buff[ret] = 0;
				inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
				clientPort = ntohs(clientAddr.sin_port);

				cout << "Receive from client: " << endl;
				cout << "	clientIP: " << clientIP << endl;
				cout << "	clientPort: " << clientPort << endl;
				cout << "	size of buff: " << strlen(buff) << endl;
				cout << "	client message: " << buff << endl;

				//send response to client
				ret = send(client[i], buff, strlen(buff), 0);
				if (ret == SOCKET_ERROR) {
					int errorCode = WSAGetLastError();
					if (errorCode != WSAEWOULDBLOCK) {
						closesocket(client[i]);
					}
				}
			}
		}
	}
}
int main() {
	if (initWinsock() == 0) {
		if (constructorSock() == 0) {
			if (bindAddressSocket() == 0) {
				if (listenRequestFromClient() == 0) {
					communicateWithClient();
				}
			}
		}
	}
	return 0;
}