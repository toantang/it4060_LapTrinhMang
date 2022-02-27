#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "stdio.h"
#include "conio.h"
#include "string.h"
#include "ws2tcpip.h"
#include "winsock2.h"
#include "process.h"
#include "utils.h"
#include "time.h"
#define SERVER_PORT 5000
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define MAX_SESSION 100
#define MAX_ACCOUNT 100
#define ENDING_DELIMITER "\r\n"
#pragma comment (lib, "Ws2_32.lib")
struct Account {
	char username[1024];
	int status;
};

struct Session {
	SOCKET socket;
	sockaddr_in clientAddr;
	char username[1024];
	int status;
};
struct Database {
	int index;
	struct Session listSession[MAX_SESSION];
};

struct DataThread {
	SOCKET socket;
	char buff[2048];
};


struct Account listAccount[MAX_ACCOUNT];
struct Database database;




/*
* @function init : read account status and save it to a list object 'Account'
*
* @return :  void
*/
void init() {
	FILE *fptr = fopen("account.txt", "r");
	if (fptr == NULL) {
		printf("Open file account error!");
		return;
	}
	char buff[1024];
	int i = 0;
	int idx = 0;
	while (fscanf(fptr, " %1023s", buff) == 1) {
		if (i % 2 == 0) {
			strcpy(listAccount[idx].username, buff);

		}
		else {
			listAccount[idx].status = atoi(buff);
			idx++;
		}
		i++;
	}
	fclose(fptr);
}




/*
* @function writeLogs : write logs to file
* @param session : a Session object contains infomations session request
* @param requestStr : raw string request
* @param result : response code string
*
* @return :  void
*/
void writeLogs(Session session, char * requestStr, char * result) {
	FILE *fptr = fopen("log_20183989.txt", "a");
	if (fptr == NULL) {
		printf("Interact file logs error!");
		return;
	}
	char * logs = (char *)malloc(sizeof(char) * 2000);
	char clientIP[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &session.clientAddr.sin_addr, clientIP, sizeof(clientIP));
	int clientPort = ntohs(session.clientAddr.sin_port);
	char * strNow = getStrTimeNow();
	sprintf(logs, "%s:%d [%s] $ %s $ %s\n", clientIP, clientPort, strNow, requestStr, result);
	fprintf(fptr, "%s", logs);
	//write file
	fclose(fptr);
}

/*
* @function logout : logout
* @param socket : a SOCKET object contains infomations socket
*
* @return :  0		 if logout success
1		 if not logged before
*/
int logout(SOCKET socket) {
	for (int i = 0; i < MAX_SESSION; i++) {
		if (database.listSession[i].socket == socket) {
			if (strlen(database.listSession[i].username) > 0) {
				strcpy(database.listSession[i].username, "");
				database.listSession[i].status = 0;
				return 0;
			}
			else {
				return 1;
			}

		}
	}
	return 1;
}

/*
* @function isAccountActive : check account active when read file account
* @param username : a user string to check
*
*
* @return :  true     if active
false    if not active
*/
bool isAccountActive(char * username) {
	for (int i = 0; i < MAX_ACCOUNT; i++) {
		if (strcmp(listAccount[i].username, username) == 0) {
			if (listAccount[i].status == 1) {
				return true;
			}
		}
	}
	return false;
}

/*
* @function login : login account
* @param socket : a SOCKET object contains infomations socket request
* @param username : username to login
*
* @return :  0		if login success
1		if account not active
2		if socket in used by another account
3		if can't get infomation
*/
int login(SOCKET socket, char * username) {
	if (!isAccountActive(username)) {
		printf("%s not active !\n", username);
		return 1;
	}
	for (int i = 0; i < MAX_SESSION; i++) {
		if (database.listSession[i].socket == socket) {
			if (strcmp(database.listSession[i].username, "") == 0) {
				//login
				strcpy(database.listSession[i].username, username);
				database.listSession[i].status = 1;
				printf("%s login success !\n", username);
				return 0;
			}
			else {   // denied
				printf("%s login error, socket in used by another account !\n", username);
				return 2;
			}

		}
	}
	sockaddr_in clientAddr;
	int sizeAddr = 5000;
	int rs = getpeername(socket, (sockaddr *)&clientAddr, &sizeAddr);
	if (rs == 0) {
		//update database
		struct Session session;
		session.clientAddr = clientAddr;
		session.socket = socket;
		strcpy(session.username, username);
		session.status = 1;
		database.listSession[database.index] = session;
		database.index += 1;
		return 0;
	}
	return 3;
}

/*
* @function isLogin : check has account using socket
* @param socket : a SOCKET object contains infomations socket
*
* @return :  true		if socket has account in used
fales		if socket free
*/
bool isLogin(SOCKET socket) {
	for (int i = 0; i < MAX_SESSION; i++) {
		if (database.listSession[i].socket == socket) {
			if (strlen(database.listSession[i].username) > 0) {
				return true;
			}
		}
	}
	return false;
}


/* The recv() wrapper function */
int Receive(SOCKET s, char *buff, int size, int flags) {
	int n;

	n = recv(s, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error: %d", WSAGetLastError());
	buff[n] = 0;
	return n;
}



/* optimizeSession - Thread to optimize database Session object by replace old session*/
unsigned __stdcall optimizeSession(void* param) {
	while (1) {
		if (database.index == MAX_SESSION - 1) {
			database.index = 0;
		}
	}
	return 0;
}



char buffStorage[20000][2048];
/* handleMessage - read and handle buff - socket connected*/
void handleMessage(SOCKET connectedSocket, char * buff) {
	sockaddr_in clientAddr;
	int sizeAddr = 5000;
	int rs = getpeername(connectedSocket, (sockaddr *)&clientAddr, &sizeAddr);
	int rsss = WSAGetLastError();
	struct Session session;
	session.clientAddr = clientAddr;
	session.socket = connectedSocket;
	char * responseCode = (char *)malloc(sizeof(char) * 2);
	bool isSuccess = false;
	char  * typeReq = (char *)malloc(sizeof(char) * 4);
	typeReq = getSubStr(buff, 0, 4);
	if (strcmp(typeReq, "USER") == 0) {
		int loginCode = login(connectedSocket, &buff[5]);
		switch (loginCode) {
		case 0:
			responseCode = "10";
			isSuccess = true;
			break;
		case 1:
			responseCode = "11";
			break;
		case 2:
			responseCode = "12";
			break;
		case 3:
			responseCode = "13";
			break;
		}
	}
	else if (strcmp(typeReq, "POST") == 0) {
		if (isLogin(connectedSocket)) {
			responseCode = "20";
			isSuccess = true;
		}
		else {
			responseCode = "21";
		}
	}
	else if (strcmp(typeReq, "QUIT") == 0) {
		int logoutCode = logout(connectedSocket);
		switch (logoutCode) {
		case 0:
			responseCode = "30";
			isSuccess = true;
			break;
		case 1:
			responseCode = "31";
			break;
		}
		printf("Client disconnects.\n");
	}
	else {
		logout(connectedSocket);
		printf("unknow method  ---> logout.\n");
	}
	writeLogs(session, buff, responseCode);
	//Send response to client
	int ret = 0;
	if (!isSuccess) {
		char * temp = (char *)malloc(sizeof(char) * 3);
		sprintf(temp, "-%s", responseCode);
		ret = send(connectedSocket, temp, strlen(temp), 0);
	}
	else {
		ret = send(connectedSocket, responseCode, strlen(responseCode), 0);
	}

	if (ret == SOCKET_ERROR)
		printf("Error %d: Cannot send data.\n", WSAGetLastError());
	else
		printf("Server receive: %s\n", buff);
}

/* handleSocket - Thread to receive the message from client and handle*/
unsigned __stdcall handleSocket(void *param) {
	DataThread * data = (DataThread *)param;
	SOCKET connectedSocket = data->socket;
	char * buff = data->buff;
	handleMessage(connectedSocket, buff);
	return 0;
}

int main(int argc, char *argv[], char **envp) {
	int serverPort = atoi(argv[1]);
	init();
	database.index = 0;
	_beginthreadex(0, 0, optimizeSession, NULL, 0, 0);

	//Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported\n");

	//Step 2: Construct socket	
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error! Cannot bind this address.");
		_getch();
		return 0;
	}

	//Step 4: Listen request from client
	if (listen(listenSock, 10)) {
		printf("Error! Cannot listen.");
		_getch();
		return 0;
	}

	printf("Server started!\n");


	SOCKET client[FD_SETSIZE], connSock;
	fd_set readfds, initfds; //use initfds to initiate readfds at the begining of every loop step
	sockaddr_in clientAddr;
	int ret, nEvents, clientAddrLen;
	char rcvBuff[BUFF_SIZE];

	for (int i = 0; i < FD_SETSIZE; i++)
		client[i] = 0;	// 0 indicates available entry

	FD_ZERO(&initfds);
	FD_SET(listenSock, &initfds);

	//Step 5: Communicate with clients
	while (1) {
		readfds = initfds;		/* structure assignment */
		nEvents = select(0, &readfds, 0, 0, 0);
		if (nEvents < 0) {
			printf("\nError! Cannot poll sockets: %d", WSAGetLastError());
			break;
		}

		//new client connection
		if (FD_ISSET(listenSock, &readfds)) {
			clientAddrLen = sizeof(clientAddr);
			if ((connSock = accept(listenSock, (sockaddr *)&clientAddr, &clientAddrLen)) < 0) {
				printf("\nError! Cannot accept new connection: %d", WSAGetLastError());
				break;
			}
			else {
				printf("New connection from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

				int i;
				for (i = 0; i < FD_SETSIZE; i++)
					if (client[i] == 0) {
						client[i] = connSock;
						FD_SET(client[i], &initfds);
						break;
					}

				if (i == FD_SETSIZE) {
					printf("\nToo many clients.");
					closesocket(connSock);
				}

				if (--nEvents == 0)
					continue; //no more event
			}
		}

		//receive data from clients
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (client[i] == 0)
				continue;

			if (FD_ISSET(client[i], &readfds)) {
				ret = Receive(client[i], rcvBuff, BUFF_SIZE, 0);
				if (ret <= 0) {
					FD_CLR(client[i], &initfds);
					closesocket(client[i]);
					strcpy(buffStorage[client[i]], "");
					client[i] = 0;
				}
				else if (ret > 0) {
					// handle tcp buffer
					char allBuff[4096];

					//continue with old buff data
					strcpy(allBuff, buffStorage[client[i]]);
					strcat(allBuff, rcvBuff);
					int idx = 0;
					for (int u = 0; u < strlen(allBuff); u++) {
						if (allBuff[u] == '\r' && allBuff[u + 1] == '\n') {
							char * tmp = getSubStr(allBuff,idx, u -idx);
							idx = u + 2;
							//handleMessage(client[i], tmp);
							struct DataThread data;
							data.socket = client[i];
							strcpy(data.buff, tmp);
							_beginthreadex(0, 0, handleSocket, (void *)&data, 0, 0);
							
						}
					}

					//save remain data
					char * remainData = getSubStr(allBuff, idx, strlen(allBuff) - idx);
					strcpy(buffStorage[client[i]], remainData);
					
				}
			}

			if (--nEvents <= 0)
				continue; //no more event
		}

	}

	closesocket(listenSock);
	WSACleanup();
	return 0;
}


