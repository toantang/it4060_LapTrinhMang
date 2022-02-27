#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <string.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable : 4996)
using namespace std;

const int BUFF_SIZE = 2048;
WSADATA wsaData;
SOCKET server;
sockaddr_in serverAddr;

typedef struct ResponseSplitString {
	string num;
	string alphabet;
	string toStringAnswer() {
		string str = num + alphabet + "\n";
		return str;
	}
} ResSplitString;

typedef struct ResponseError {
	string idError;
	string nameError;
	string toStringError() {
		string str = "";
		str = str + "Error code: " + idError + ", error: " + nameError + "\n";
		return str;
	}
} ResError;

typedef struct ResponseTCP {
	string prefix;
	ResSplitString resSplitString;
	ResError resError;
	string toStringResponse() {
		string str = "";
		if (prefix == "ANSWER") {
			str = prefix + resSplitString.toStringAnswer();
		}
		else {
			str = prefix + resError.toStringError();
		}
		return str;
	}
} Response;

ResSplitString resSplitString;
ResError resError;
Response response;

int initWinsock() {
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void constructorSock() {
	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

int bindAddressServer() {
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5500);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	return bind(server, (sockaddr*)&serverAddr, sizeof(serverAddr));
}

int listenRequestFromClient() {
	return listen(server, 10);
}

void inforMessageFromClient(char clientIP[], int clientPort, char buff[]) {
	cout << "Receive from client: " << endl;
	cout << "	clientIP: " << clientIP << endl;
	cout << "	clientPort: " << clientPort << endl;
	cout << "	size of buff: " << strlen(buff) << endl;
	cout << "	client message: " << buff << endl;
}

bool checkSpecialCharacters(string s) {
	int len = s.size();
	for (int i = 0; i < len; i++) {	
		if ((
			(s[i] >= 48 && s[i] <= 57) || 
			(s[i] >= 65 && s[i] <= 90) || 
			(s[i] >= 97 && s[i] <= 122)) 
			== false) {
			return false;
		}
	}
	return true;
}

void processData(string data) {
	string numbers = "";
	string characters = "";
	for (int i = 0; i < data.size(); i++) {
		if (data[i] >= 48 && data[i] <= 57) {
			numbers += data[i];
		}
		else {
			characters += data[i];
		}
	}

	cout << "numbers: " << numbers << ", characters: " << characters << endl;
	resSplitString.num = numbers;
	resSplitString.alphabet = characters;
}
void communicateWithClient() {
	cout << "Server started:" << endl;
	sockaddr_in clientAddr;

	char buff[BUFF_SIZE];
	int ret;
	int clientPort;
	char clientIP[INET_ADDRSTRLEN];
	int clientAddrLen = sizeof(clientAddr);
	SOCKET connSock;
	connSock = accept(server, (sockaddr*)&clientAddr, &clientAddrLen);
	inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
	clientPort = ntohs(clientAddr.sin_port);

	while (true) {
		//receive message from client
		ret = recv(connSock, buff, BUFF_SIZE, 0);
		buff[ret] = 0;

		if (ret == SOCKET_ERROR) {
			cout << "Error: " << WSAGetLastError() << endl;
			response.prefix = "ERROR";
			resError.idError = "001";
			resError.nameError = WSAGetLastError();
			response.resError = resError;
		}
		else {
			
			inforMessageFromClient(clientIP, clientPort, buff);

			if (checkSpecialCharacters(buff)) {
				cout << "vao ham process daa" << endl;
				processData(buff);
				response.prefix = "ANSWER";
				response.resSplitString = resSplitString;
			}
			else {
				resError.idError = "002";
				resError.nameError = "Characters is not correct";
				response.resError = resError;
				response.prefix = "ERROR";
			}
			cout << "infor response: " << response.toStringResponse() << endl;
			char res[BUFF_SIZE];
			//string ans = resSplitString.toStringAnswer();
			string ans = response.toStringResponse();
			strcpy(res, ans.c_str());

			cout << "ans = " << ans << endl;
			ret = send(connSock, res, strlen(res), 0);

			if (ret == SOCKET_ERROR) {
				cout << "Error: " << WSAGetLastError() << endl;
			}
		}
	}
}

int main() {
	cout << "vao ham main" << endl;
	if (initWinsock() == 0) {
		constructorSock();
		if (bindAddressServer() == 0) {
			if (listenRequestFromClient() == 0) {
				communicateWithClient();
			}
		}
	}
	return 0;
}