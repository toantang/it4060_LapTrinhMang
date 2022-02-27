#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string.h>
#include <vector>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;
const int BUFF_SIZE = 2048;

WSADATA wsaData;
SOCKET client;
sockaddr_in serverAddr;

int initWinsock() {
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
}
void constructorSock() {
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int tv = 10000; // timeout interval 10000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(int));
}
void specifyServerAddress() {
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5500);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
}
int requestConnectToServer() {
	return connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr));
}

typedef struct RequestLogin {
	string username;
	string toStringReqLogin() {
		return username + "\n";
	}
} ReqLogin;
typedef struct RequestPost {
	string contentPost;
	string toStringReqPost() {
		return contentPost + "\n";
	}
} ReqPost;

typedef struct RequestLogout {
	string logout;
	string toStringLogout() {
		return "";
	}
} ReqLogout;
typedef struct RequestFromClient {
	string prefix;
	ReqLogin reqLogin;
	ReqPost reqPost;
	ReqLogout reqLogout;
	string toStringRequest() {
		string str = "";
		if (prefix == "USER") {
			str = prefix + reqLogin.toStringReqLogin();
		}
		else if (prefix == "POST") {
			str = prefix + reqPost.toStringReqPost();
		}
		else if (prefix == "QUIT") {
			str = prefix + reqLogout.toStringLogout();
		}
		else {
			str = "FAIL";
		}
		return str;
	}
} Request;

ReqLogin reqLogin;
ReqPost reqPost;
ReqLogout reqLogout;
Request request;
void requestLogin() {
	cout << "nhap username de dang nhap: ";
	char buff[BUFF_SIZE];
	gets_s(buff, BUFF_SIZE);
	reqLogin.username = buff;
}

void requestPost() {
	cout << "nhap noi dung bai viet: ";
	char buff[BUFF_SIZE];
	gets_s(buff, BUFF_SIZE);
	reqPost.contentPost = buff;
}

void requestLogout() {
	cout << "nhap 2 de log out" << endl;
}
void communicateWithServer() {
	char buff[BUFF_SIZE];
	int ret;
	int messageLen;
	bool isLogin = false;
	int mode = 0;
	while (true) {
		switch (mode) {
		case 0:
			request.prefix = "USER";
			requestLogin();		
			request.reqLogin = reqLogin;
			break;
		case 1:
			request.prefix = "POST";
			requestPost();
			request.reqPost = reqPost;
			break;
		case 2: 
			request.prefix = "QUIT";
			requestLogout();
			break;
		}
		
		
		string req = request.toStringRequest();
		messageLen = req.size();
		cout << "req is: " << req << endl;
		ret = send(client, req.c_str(), messageLen, 0);

		if (ret == SOCKET_ERROR) {

		}
		else {
			ret = recv(client, buff, BUFF_SIZE, 0);
			if (ret == SOCKET_ERROR) {

			}
			else if (strlen(buff) > 0) {
				buff[ret] = '\0';
				cout << "buff: " << buff << endl;
			}
			else {
				cout << "have any error" << endl;
			}
		}
	}
}

int main() {
	if (initWinsock() == 0) {
		constructorSock();
		specifyServerAddress();
		if (requestConnectToServer() == 0) {
			communicateWithServer();
		}
	}
	return 0;
}