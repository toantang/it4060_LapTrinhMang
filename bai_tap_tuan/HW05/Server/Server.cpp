//define in stdafx file 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "Server.h"
#include "stdafx.h"
#include "winsock2.h"
#include "windows.h"
#include "stdio.h"
#include "conio.h"
#include "ws2tcpip.h"
#include "winsock2.h"
#include "Resource.h"
#include "string.h"
#include "ws2tcpip.h"
#include "process.h"
#include "time.h"
#define WM_SOCKET WM_USER + 1
#define MAX_CLIENT 1024
#define SERVER_PORT 6600
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define MAX_SESSION 100
#define MAX_ACCOUNT 100
#define ENDING_DELIMITER "\r\n"
#pragma comment (lib, "Ws2_32.lib")
CRITICAL_SECTION critical;
char* getSubStr(char *, int, int);
char * getStrTimeNow();

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
	
	char * logs = (char *)malloc(sizeof(char) * 1000);
	char clientIP[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &session.clientAddr.sin_addr, clientIP, sizeof(clientIP));
	int clientPort = ntohs(session.clientAddr.sin_port);
	char * strNow = getStrTimeNow();
	sprintf(logs, "%s:%d [%s] $ %s $ %s\n", clientIP, clientPort, strNow, requestStr, result);
	EnterCriticalSection(&critical);
	FILE *fptr = fopen("log_20183989.txt", "a");
	if (fptr == NULL) {
		printf("Interact file logs error!");
		return;
	}
	fprintf(fptr, "%s", logs);
	//write file
	fclose(fptr);
	LeaveCriticalSection(&critical);
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
// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
HWND				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	windowProc(HWND, UINT, WPARAM, LPARAM);

SOCKET client[MAX_CLIENT];
SOCKET listenSock;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	init();
	database.index = 0;
	_beginthreadex(0, 0, optimizeSession, NULL, 0, 0);
	InitializeCriticalSection(&critical);
	MSG msg;
	HWND serverWindow;

	//Registering the Window Class
	MyRegisterClass(hInstance);

	//Create the window
	if ((serverWindow = InitInstance(hInstance, nCmdShow)) == NULL)
		return FALSE;

	//Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		MessageBox(serverWindow, L"Winsock 2.2 is not supported.", L"Error!", MB_OK);
		return 0;
	}

	//Construct socket	
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//requests Windows message-based notification of network events for listenSock
	WSAAsyncSelect(listenSock, serverWindow, WM_SOCKET, FD_ACCEPT | FD_CLOSE | FD_READ);

	//Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		MessageBox(serverWindow, L"Cannot associate a local address with server socket.", L"Error!", MB_OK);
	}

	//Listen request from client
	if (listen(listenSock, MAX_CLIENT)) {
		MessageBox(serverWindow, L"Cannot place server socket in state LISTEN.", L"Error!", MB_OK);
		return 0;
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//DeleteCriticalSection(&critical);
	return 0;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = windowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SERVER)); //
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"WindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	int i;
	for (i = 0; i <MAX_CLIENT; i++)
		client[i] = 0;
	hWnd = CreateWindow(L"WindowClass", L"WSAAsyncSelect TCP Server", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_SOCKET	- process the events on the sockets
//  WM_DESTROY	- post a quit message and return
//
//

LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SOCKET connSock;
	sockaddr_in clientAddr;
	int ret, clientAddrLen = sizeof(clientAddr), i;
	char rcvBuff[BUFF_SIZE], sendBuff[BUFF_SIZE];

	switch (message) {
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lParam)) {
			for (i = 0; i < MAX_CLIENT; i++)
				if (client[i] == (SOCKET)wParam) {
					closesocket(client[i]);
					client[i] = 0;
					continue;
				}
		}

		switch (WSAGETSELECTEVENT(lParam)) {
		case FD_ACCEPT:
		{
			connSock = accept((SOCKET)wParam, (sockaddr *)&clientAddr, &clientAddrLen);
			if (connSock == INVALID_SOCKET) {
				break;
			}
			for (i = 0; i < MAX_CLIENT; i++)
				if (client[i] == 0) {
					client[i] = connSock;
					//requests Windows message-based notification of network events for listenSock
					WSAAsyncSelect(client[i], hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
					break;
				}
			if (i == MAX_CLIENT)
				MessageBox(hWnd, L"Too many clients!", L"Notice", MB_OK);
		}
		break;

		case FD_READ:
		{
			for (i = 0; i < MAX_CLIENT; i++)
				if (client[i] == (SOCKET)wParam)
					break;

			ret = recv(client[i], rcvBuff, BUFF_SIZE, 0);
			if (ret > 0) {
				////echo to client
				//memcpy(sendBuff, rcvBuff, ret);
				//send(client[i], sendBuff, ret, 0);
				rcvBuff[ret] = 0;
				char allBuff[4096];

				//continue with old buff data
				strcpy(allBuff, buffStorage[client[i]]);
				int debugggg = strlen(allBuff);
				strcat(allBuff, rcvBuff);
				int idx = 0;
				debugggg = strlen(allBuff);
				for (int u = 0; u < strlen(allBuff); u++) {
					if (allBuff[u] == '\r' && allBuff[u + 1] == '\n') {
						char * tmp = getSubStr(allBuff, idx, u - idx);
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
		break;

		case FD_CLOSE:
		{
			for (i = 0; i < MAX_CLIENT; i++)
				if (client[i] == (SOCKET)wParam) {
					closesocket(client[i]);
					client[i] = 0;
					break;
				}
		}
		break;
		}
	}
	break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		shutdown(listenSock, SD_BOTH);
		closesocket(listenSock);
		WSACleanup();
		return 0;
	}
	break;

	case WM_CLOSE:
	{
		DestroyWindow(hWnd);
		shutdown(listenSock, SD_BOTH);
		closesocket(listenSock);
		WSACleanup();
		return 0;
	}
	break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
