#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

const int BUFF_SIZE = 2048;

using namespace std;

WSADATA wsaData;
SOCKET server;
sockaddr_in serverAddr;

int initWinsock() {
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	return iResult;
}

void constructorSocket() {
	server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

int bindAddressToSocket() {
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5500);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	if (bind(server, (sockaddr *)&serverAddr, sizeof(serverAddr)) != 0) {
		cout << "Error! Cannot bind this address" << endl;
		return 0;
	}
	else {
		cout << "server started" << endl;
		return 1;
	}
}

void echoToClient(char buff[BUFF_SIZE], int ret, sockaddr_in clientAddr) {
	ret = sendto(server, buff, ret, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
	if (ret == SOCKET_ERROR) {
		cout << "Error when echo to client: " << WSAGetLastError() << endl;
	}
}

void communicateClient() {
	sockaddr_in clientAddr; //sock address cua client
	char buff[BUFF_SIZE]; // chuoi ki tu nhan duoc tu client
	char clientIP[INET_ADDRSTRLEN];
	int ret; // mã code nhận được
	int clientAddrLen = sizeof(clientAddr); // do dai dia chi client
	int clientPort; // so hieu cong cua may client
	while (true) {
		//Receive message from client
		ret = recvfrom(server, buff, BUFF_SIZE, 0, (sockaddr *)&clientAddr, &clientAddrLen);
		if (ret == SOCKET_ERROR) {
			cout << "Error: " << WSAGetLastError() << endl;
		}
		else {
			cout << "ret is " << ret << endl;
			buff[ret] = 0; // nếu ret = 100 => có 99 byte vì mảng chạy từ 0
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			cout << "Receive from client: " << endl;
			cout << "	ip address: " << clientIP << endl;
			cout << "	client port: " << clientPort << endl;
			cout << "	content: " << buff << endl;

			echoToClient(buff, ret, clientAddr);
		}
	}
}



int main(int argc, char* argv[]) {
	if (initWinsock() == 0) {
		constructorSocket();
		if (bindAddressToSocket() == 1) {
			communicateClient();
		}
		
	}
	return 0;
}