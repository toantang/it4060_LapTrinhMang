#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

WSADATA wsaData;
int iResult;
void initSocket() {
	iResult = WSAStartup(MAKEWORD(2, 2, ), &wsaData);
	if (iResult == 0) {
		cout << "init socket is successful" << endl;
		SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		cout << "value of socket is " << s << endl;
	}
	else {
		cout << "init socket is failed" << endl;
		cout << "value of iResult is " << iResult << endl;
	}
}

void splitDomainName() {
	addrinfo *result = NULL;
	addrinfo *hints = NULL;
	int rc = getaddrinfo("soict.hust.edu.vn", NULL, hints, &result);
	sockaddr_in* addr;
	char ipStr[INET_ADDRSTRLEN];
	if (rc == 0) {
		while (result != NULL) {
			addr = (struct sockaddr_in*)result->ai_addr;
			inet_ntop(AF_INET, &addr->sin_addr, ipStr, sizeof(ipStr));
			cout << "IPv4 address is " << ipStr << endl;
			cout << "ipproto_xxx: " << result->ai_protocol << endl;
			result = result->ai_next;
		}
	}
	else {
		cout << "Failed. Error code is " << WSAGetLastError() << endl;
	}
}
int main() {
	initSocket();
	splitDomainName();
	return 0;
}