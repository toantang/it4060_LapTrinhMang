#include <iostream>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <process.h> // khai bao thu vien su dung da luong thread
#include <string.h>
#include <vector>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;
const int BUFF_SIZE = 2048;
WSADATA wsaData;
SOCKET listenSock;
sockaddr_in serverAddr;

int initWinsock() {
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void constructorSock() {
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

typedef struct Room {
	vector<SOCKET> clients;
	string roomId;
} RoomChat;

RoomChat roomChat;

string getHeader(string request) {
	cout << "header is: " << request.substr(0, 4) << endl;
	return request.substr(0, 4);
}

string getBody(string request) {
	cout << "body is: " << request.substr(4) << endl;
	string str = request.substr(4);
	//str.erase(str.size() - 1);
	return str;
}

int login(SOCKET client, string body) {
	cout << "body.size() = " << body.size();
	if (body == "10") {
		roomChat.clients.push_back(client);
		return 0;
	}
	return 1;
}

int postMessage(SOCKET client, string body) {
	return 0;
}
unsigned __stdcall echoThread(void* param) {
	//cout << "xin chao echoThread" << endl;
	char buff[BUFF_SIZE];
	int ret;
	SOCKET connectedSocket = (SOCKET)param;
	ret = recv(connectedSocket, buff, BUFF_SIZE, 0);
	if (ret == SOCKET_ERROR) {

	}
	else if (strlen(buff) > 0) {
		buff[ret] = 0;
		cout << "Receive from client: " << endl;
		cout << "	client message: " << buff << endl;
		//cout << "	clientIP: " << clientIP << endl;
		//cout << "	clientPort: " << clientPort << endl;
		cout << "	size of buff: " << strlen(buff) << endl;

		string header = getHeader(buff);
		string body = getBody(buff);
		string messageToClient;

		if (header == "USER") {
			int x = login(connectedSocket, body);
			switch (x) {
			case 0: 
				messageToClient = "00";
				break;
			case 1:
				messageToClient = "01";
				break;
			}
		}
		else if (header == "POST") {
			int x = postMessage(connectedSocket, body);
			switch (x) {
			case 0: 
				messageToClient = body;
				break;
			}
		}
		else {
			messageToClient = "39";
		}

		cout << "message to client: " << messageToClient << endl;
		cout << roomChat.clients.size() << endl;
		//ret = send(connectedSocket, buff, strlen(buff), 0);
		for (int i = 0; i < roomChat.clients.size(); i++) {
			//ret = send(connectedSocket, buff, strlen(buff), 0);
			ret = send(roomChat.clients.at(i), messageToClient.c_str(), messageToClient.size(), 0);
			if (ret == SOCKET_ERROR) {
				cout << "error" << endl;
			}
		}
		//ret = send(connectedSocket, buff, strlen(buff), 0);
		if (ret == SOCKET_ERROR) {

		}
	}
	closesocket(connectedSocket);
	return 0;
}

void communicateWithClient() {
	cout << "Server strarted" << endl;
	SOCKET connSock;
	sockaddr_in clientAddr;
	char clientIP[INET_ADDRSTRLEN];
	int clientAddrLen = sizeof(clientAddr);
	int clientPort;
	char buff[BUFF_SIZE];
	int ret;
	while (true) {
		connSock = accept(listenSock, (sockaddr*)&clientAddr, &clientAddrLen);

		if (connSock == SOCKET_ERROR) {
			cout << "Error % d: Cannot permit incoming connection.\n" << WSAGetLastError() << endl;
		}
		else {
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			cout << "Accept incoming connection from " << clientIP << ", " << clientPort << endl;
			_beginthreadex(0, 0, echoThread, (void*)connSock, 0, 0);
		}
	}
}
int main() {
	if (initWinsock() == 0) {
		constructorSock();
		if (bindAddressSocket() == 0) {
			if (listenRequestFromClient() == 0) {
				communicateWithClient();
			}
		}
	}
	return 0;
}