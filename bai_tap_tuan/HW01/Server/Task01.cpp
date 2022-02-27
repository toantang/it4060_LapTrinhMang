#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

/*
* @function domainToIps : get ip addresses of domain
* @param domain : a pointer to a string domain
*
* @return :  string includes ip address split by ","
			 return "" if not have infomation
*/

char * domainToIps(char* domain) {
	char result[1000] = "";
	addrinfo * addrif;
	int rc;
	sockaddr_in *address;
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;

	rc = getaddrinfo(domain, NULL, &hints, &addrif);
	char ipStr[INET_ADDRSTRLEN];
	if (rc == 0) {
		while (true) {
			address = (sockaddr_in *)addrif->ai_addr;
			inet_ntop(AF_INET, &address->sin_addr, ipStr, sizeof(ipStr));

			strcat_s(result, sizeof result, ipStr);
			strcat_s(result, sizeof result, ",");

			if (addrif->ai_next != NULL) {
				addrif = addrif->ai_next;
			}
			else {
				break;
			}
		}
		return result;
	}
	return "";
}

/*
* @function ipToDomains : reverse ip address to domain name
* @param ipStr : a pointer to a string ip address, such as "8.8.8.8"
*
* @return :  string include hostname of ip
			 return "" if not have infomation
*/

char * ipToDomains(char* ipStr) {
	struct sockaddr_in addr;
	int ret;
	char hostname[NI_MAXHOST];
	char servInfo[NI_MAXSERV];
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, ipStr, &addr.sin_addr);
	ret = getnameinfo((struct sockaddr *) &addr,
		sizeof(struct sockaddr),
		hostname, NI_MAXHOST,
		servInfo, NI_MAXSERV, NI_NUMERICSERV);
	if (ret != 0) {
		printf("Failed.Error code : # %ld\n", WSAGetLastError());
		return "";
	}
	else
		return hostname;
	
}


int main(int argc, char *argv[], char **envp) {
	int port = atoi(argv[1]);

	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	WSAStartup(wVersion, &wsaData);
	//do something with WinSock
	SOCKET server;
	server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

	if (bind(server, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error! Cannot bind this address.");
		return 0;
	}
	printf("Server started! \n");
	sockaddr_in clientAddr;
	char buff[1000], clientIP[INET_ADDRSTRLEN];
	int ret, clientAddrLen = sizeof(clientAddr), clientPort;
	while (1) {
		//Receive message
		ret = recvfrom(server, buff, sizeof(buff), 0,
			(sockaddr *)&clientAddr, &clientAddrLen);
		inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP,
			sizeof(clientIP));
		if (ret == SOCKET_ERROR)
			printf("Error : %d", WSAGetLastError());
		else {
			buff[ret] = 0;
			clientPort = ntohs(clientAddr.sin_port);
			printf("Receive from client %s:%d %s\n",
				clientIP, clientPort, buff);
			bool isGetSuccessInfo = true;
			char* result;
			// consider type request
			switch (buff[0])
			{
			case 'F':
				//Echo to client
				//strcpy(result, domainToIps(&buff[1]));
				result = domainToIps(&buff[1]);
				if (strcmp(result, "") == 0) {
					isGetSuccessInfo = false;
					break;
				}
				printf("Ips: %s  \n", result);
				ret = sendto(server, result, strlen(result), 0,
					(SOCKADDR *)&clientAddr, sizeof(clientAddr));
				if (ret == SOCKET_ERROR)
					printf("Error: %d", WSAGetLastError());
				break;
			case 'R':
				//Echo to client
				//strcpy(result, domainToIps(&buff[1]));
				result = ipToDomains(&buff[1]);
				if (strcmp(result, &buff[1]) == 0 || strcmp(result, "204.204.204.204") == 0) {
					isGetSuccessInfo = false;
					break;
				}
				printf("Hostname: %s \n", result);
				ret = sendto(server, result, strlen(result), 0,
					(SOCKADDR *)&clientAddr, sizeof(clientAddr));
				if (ret == SOCKET_ERROR)
					printf("Error: %d", WSAGetLastError());
				break;
			}

			if (isGetSuccessInfo == false) {
				result = "-No Infomations!";
				ret = sendto(server, result, strlen(result), 0,
					(SOCKADDR *)&clientAddr, sizeof(clientAddr));
				if (ret == SOCKET_ERROR)
					printf("Error: %d", WSAGetLastError());
			}
		}
	}

	//end work with win sock
	WSACleanup();
}