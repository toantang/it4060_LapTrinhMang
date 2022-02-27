#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define ENDING_DELIMITER "\r\n"
#define SERVER_PORT 5000
#define SERVER_ADDR "127.0.0.1"

int main(int argc, char *argv[], char **envp) {
	int serverPort = atoi(argv[2]);
	char * addrServerStr = argv[1];
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	WSAStartup(wVersion, &wsaData);
	//do something with WinSock


	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	if (client == INVALID_SOCKET) {		printf("Not contruct!");	}	int tv = 10000; //Time-out interval: 10000ms

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	inet_pton(AF_INET, addrServerStr, &serverAddr.sin_addr);

	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO,
		(const char*)(&tv), sizeof(int));	if (connect(client, (sockaddr *)&serverAddr,
		sizeof(serverAddr))) {
		printf("Error! Cannot connect server.");
		return 0;
	}	//Step 5: Communicate with server
	int ret;
	bool isExit = false;

	do {
		char temp[1024];
		char buff[1024];
		printf("1: Login  \n2: Post Message \n3: Log out \nOther numbers: Exit!\n\nPlease select: ");
		//Send message with function type
		char func_no[10];
		gets_s(func_no, 10);
		switch (atoi(func_no)) {
		case 1:

			printf("Input username: ");
			gets_s(buff, 1024);
			strcpy(temp, "USER ");
			strcat(temp, buff);
			strcpy(buff, temp);
			strcat(buff, ENDING_DELIMITER);
			printf("Type request: Login \n");
			break;
		case 2:
			printf("Write Message: ");
			gets_s(buff);
			// add prefix "POST" 
			strcpy(temp, "POST ");
			strcat(temp, buff);
			strcpy(buff, temp);
			strcat(buff, ENDING_DELIMITER);
			printf("Type request: Post message \n");
			break;
		case 3:
			strcpy(buff, "QUIT");
			strcat(buff, ENDING_DELIMITER);
			printf("Type request: Logout \n");
			break;
		default:
			printf("Client stop !\n");
			isExit = true;
			break;
		}
		if (isExit) {
			continue;
		}

		ret = send(client, buff, strlen(buff), 0);
		if (ret == SOCKET_ERROR)
			printf("Error %d", WSAGetLastError());
		ret = recv(client, buff, strlen(buff), 0);
		if (ret == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT)
				printf("Time-out!");
			else printf("Error %d", WSAGetLastError());
		}
		else {
			buff[ret] = '\0';
			char result[1024];
			/// show result based on response code
			if (buff[0] == '-') {
				if (strcmp(&buff[1], "11") == 0) {
					strcpy(result, "Account not active!");
				}
				else if (strcmp(&buff[1], "12") == 0) {
					strcpy(result, "Application used by another account!");
				}
				else if (strcmp(&buff[1], "13") == 0) {
					strcpy(result, "Something went wrong!");
				}
				else if (strcmp(&buff[1], "21") == 0) {
					strcpy(result, "Not logged in!");
				}
				else if (strcmp(&buff[1], "31") == 0) {
					strcpy(result, "Not logged in!");
				}
				printf("\n=====>>  Response error: %s-%s", &buff[1], result);
			}
			else {
				if (strcmp(buff, "10") == 0) {
					strcpy(result, "Log in successed!");
				}
				else if (strcmp(buff, "20") == 0) {
					strcpy(result, "Post message successed!");
				}
				else if (strcmp(buff, "30") == 0) {
					strcpy(result, "Log out successed!");
				}
				printf("\n=====>>  Response: %s-%s", buff, result);
			}

		}
		printf("\n\n\t =================================\n\n");
	} while (!isExit);
	//end work with win sock
	printf("Bye!\n");
	closesocket(client);
	WSACleanup();
}