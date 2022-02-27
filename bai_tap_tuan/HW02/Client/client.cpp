#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")



int main(int argc, char *argv[], char **envp) {
	
	char* addrServerStr = argv[1];
	int portServer = atoi(argv[2]);
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	WSAStartup(wVersion, &wsaData);
	//do something with WinSock


	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	if (client == INVALID_SOCKET) {		printf("Not contruct!");	}	int tv = 10000; //Time-out interval: 10000ms

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5000);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO,
		(const char*)(&tv), sizeof(int));	if (connect(client, (sockaddr *)&serverAddr,
		sizeof(serverAddr))) {
		printf("Error! Cannot connect server.");
		return 0;
	}	//Step 5: Communicate with server
	
	int ret, messageLen;
	//Send message
	while (1) {
		char buff[1024];
		//Send message
		printf("Send to server: ");
		gets_s(buff, 1024);
		messageLen = strlen(buff);
		if (messageLen == 0) break;
		ret = send(client, buff, messageLen, 0);
		if (ret == SOCKET_ERROR)
			printf("Error %d", WSAGetLastError());
		ret = recv(client, buff, 1024, 0);
		if (ret == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT)
				printf("Time-out!");
			else printf("Error %d", WSAGetLastError());
		}
		else if (strlen(buff) > 0) {
			buff[ret] = '\0';
			if (buff[0] == '-') {
				printf("Server send error : %s\n", &buff[1]);
			}
			else {
				printf("Server send success: %s\n", buff);
			}
			
		}
	}
	//end work with win sock
	printf("Bye!\n");
	closesocket(client);
	WSACleanup();
}