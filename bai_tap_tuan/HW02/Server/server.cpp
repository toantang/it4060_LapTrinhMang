#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<WinSock2.h>
#include <ws2tcpip.h>
#include <math.h>
#pragma comment(lib, "ws2_32.lib")

/*
* @function sumString : sum all number character in string, if string contains non number character will return -1
* @param str : a pointer to a string
*
* @return : sum all number character
			return -1 if not string has non number character
*/
int sumString(char* str) {
	int rs = 0;
	int lenStr = strlen(str);
	for (int i = 0; i < lenStr; i++) {
		if (isdigit(str[i])) {
			rs += str[i] - '0';
		}
		else {
			return -1;
		}
	}
	return rs;
}

int main(int argc, char *argv[], char **envp) {
	int port = atoi(argv[1]);
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	WSAStartup(wVersion, &wsaData);
	//do something with WinSock


	SOCKET server;
	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5000);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

	if (bind(server, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error! Cannot bind this address.");
		return 0;
	}
	//Step 4: Listen request from client
	if (listen(server, 100)) {
		printf("Error: ");
		return 0;
	}
	printf("Server started! \n");
	//Step 5: Communicate with client
	sockaddr_in clientAddr;
	char buff[1024], clientIP[INET_ADDRSTRLEN];
	int ret, clientAddrLen = sizeof(clientAddr), clientPort;
	SOCKET connSock;
	//accept request
	while (1) {
		connSock = accept(server, (sockaddr *)& clientAddr,
			&clientAddrLen);
		inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP,
			sizeof(clientIP));
		clientPort = ntohs(clientAddr.sin_port);
		while (1) {
			//receive message from client
			ret = recv(connSock, buff, 1024, 0);
			buff[ret] = 0;
			if (ret == SOCKET_ERROR) {
				printf("Error %d", WSAGetLastError());
				break;
			}
			else {
				printf("Receive from client[%s:%d] %s\n",
					clientIP, clientPort, buff);
				if (strlen(buff) == 0) {
					closesocket(connSock);
					break;
				}
				int rs = sumString(buff);
				//Send result back to client
				if (rs < 0) {
					char * stringError = "-String contains non number character ";
					ret = send(connSock, stringError, strlen(stringError), 0);
				}
				else {
					int number_digits = 1;
					if (rs > 0) {
						number_digits = (int)floor(log10(abs(rs))) + 1;
					}
					char * strSuccess = (char *)malloc((number_digits + 1) * sizeof(char));
					sprintf(strSuccess, "%d\n", rs);
					ret = send(connSock, strSuccess, number_digits, 0);
				}
				if (ret == SOCKET_ERROR) {
					printf("Error %d", WSAGetLastError());
					break;
				}
			}
		} //end communicating
		closesocket(connSock);
	}
	
	//Step 6: Close socket
	closesocket(server);
	//end work with win sock
	WSACleanup();
}