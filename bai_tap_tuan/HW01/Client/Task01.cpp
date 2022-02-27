#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
int main(int argc, char *argv[], char **envp) {
	char* addrServerStr = argv[1];
	int portServer = atoi(argv[2]);
	// Initiates Winsock v2.2
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	WSAStartup(wVersion, &wsaData);
	//do something with WinSock
	printf("Client started!\n");
	//Step 2: Construct socket
	SOCKET client;
	client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//(optional) Set time-out for receiving
	int tv = 10000; //Time-out interval: 10000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO,
		(const char*)(&tv), sizeof(int));
	//Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(portServer);
	inet_pton(AF_INET, addrServerStr, &serverAddr.sin_addr);
	char buff[1000];
	int ret, serverAddrLen = sizeof(serverAddr);
	bool isExit = false;
	char temp[1000];
	do {
		printf("1: Find Ips from domain  \n2: Find domains from Ip \nOther number: Exit program \n\nPlease select: ");
		//Send message
		int func_no;
		scanf("%d", &func_no);
		switch (func_no) {
		case 1:
			printf("Write domain: ");
			scanf("%s", &buff);
			// add prefix "F"
			strcpy(temp, "F");
			strcat(temp, buff);
			strcpy(buff, temp);
			printf("Sending to server request parse domain \n");
			break;
		case 2:
			printf("Write Ip: ");
			scanf("%s", &buff);
			// add prefix "R" for reverse
			strcpy(temp, "R");
			strcat(temp, buff);
			strcpy(buff, temp);
			printf("Sending to server request reverse Ip \n");
			break;
		default:
			printf("Client stop !\n");
			isExit = true;
			continue;
		}
		ret = sendto(client, buff, strlen(buff), 0,
			(sockaddr *)&serverAddr, serverAddrLen);
		if (ret == SOCKET_ERROR)
			printf("Error! Cannot send mesage.");
		//Receive echo message
		ret = recvfrom(client, buff, 1024, 0,
			(sockaddr *)&serverAddr, &serverAddrLen);
		if (ret == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT)
				printf("Time-out!");
			else printf("Error! Cannot receive message.\n");
		}
		else {
			buff[ret] = '\0';
			if (buff[0] == '-') {
				printf("Server send error : %s\n", &buff[1]);
			}
			else if (strstr(buff, ",")) {
				int num_rs = 0;
				for (int i = 0; i < strlen(buff); i++) {
					if (buff[i] == ',') {
						num_rs++;
						buff[i] = '\t';
					}
				}
				printf("Receive from server %d results : %s\n", num_rs, buff);
			}
			else {
				printf("Receive from server 1 result : %s\n", buff);
			}

		}
		printf("\n\t ====================\n\n");
	} while (!isExit);

	closesocket(client);
	//Terminates use of the WinSock
	WSACleanup();
}