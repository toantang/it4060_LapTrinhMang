#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <string.h>
#include <vector>
#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable : 4996)
using namespace std;

const int BUFF_SIZE = 2048;

WSADATA wsaData;
sockaddr_in serverAddr;
SOCKET server;

typedef struct responseInformationIP {
	string officalIP;
	vector<string> aliasIP;
	string toStringInforIP() {
		string str = "";
		str += "OfficalIP: " + officalIP + "\n";
		str = str + "AliasIP:" + "\n";
		if (aliasIP.size() == 0) {
			str = str + "Not found alias ip" + "\n";
		}
		for (int i = 0; i < aliasIP.size(); i++) {
			str = str + aliasIP[i] + "\n";
		}
		return str;
	}
} ResInforIP;

typedef struct responseInformationDomainName {
	string officalName;
	vector<string> aliasName;
	string toStringInforDomainName() {
		string str = "";
		str += "Offical Name: " + officalName + "\n";
		str = str + "Alias name: " + "\n";
		if (aliasName.size() == 0) {
			str = str + "Not found alias name" + "\n";
		}
		for (int i = 0; i < aliasName.size(); i++) {
			str = str + aliasName[i] + "\n";
		}
		return str;
	}
} ResInforDomainName;

typedef struct responseInformationError {
	string idError;
	string nameError;

	/*string toStringError(ResError resError) {
		string str = "";
		str = str + "Error code: " + resError.idError + ", error: " + resError.nameError;
		return str;
	}*/
	string toStringError() {
		string str = "";
		str = str + "Error code: " + idError + ", error: " + nameError;
		return str;
	}
} ResError;

typedef struct response {
	string responseString = "";
} Response ;

ResInforDomainName resInforDomainName;
ResInforIP resInforIP;
ResError resError;
Response response;

int initWinsock() {
	return WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void constructorSock() {
	server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

int bindAddressSock() {
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5500);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	return bind(server, (sockaddr*)&serverAddr, sizeof(serverAddr));
}

void informationFromClient(char clientIP[], int clientPort, char buff[]) {
	cout << "Receive from client: " << endl;
	cout << "	ip address: " << clientIP << endl;
	cout << "	client port: " << clientPort << endl;
	cout << "	content: " << buff << endl;
}

void getIPByDomainName(char hostname[]) {
	addrinfo* result = NULL;
	addrinfo* hints = NULL;
	int rc = getaddrinfo(hostname, NULL, hints, &result);
	sockaddr_in* addr;
	char ipStr[INET_ADDRSTRLEN];

	if (rc == 0) {
		bool IPAddrIsOfficial = true;
		while (result != NULL) {
			addr = (struct sockaddr_in*)result->ai_addr;
			inet_ntop(AF_INET, &addr->sin_addr, ipStr, sizeof(ipStr));

			if (IPAddrIsOfficial) {
				resInforIP.officalIP = ipStr;
				IPAddrIsOfficial = false;
			}
			else {
				resInforIP.aliasIP.push_back(ipStr);
			}
			result = result->ai_next;
		}
	}
	else {
		resError.idError = "001";
		resError.nameError = "Not found information.\ngetaddrinfo(). error: " + WSAGetLastError();
		response.responseString = resError.toStringError();
	}
}

void getHostByIP(char ip[]) {
	//cout << "Request is IP address.\n";
	char* addrStr;
	inet_pton(AF_INET, ip, &addrStr);
	hostent *host = gethostbyaddr((char*)&addrStr, sizeof(addrStr), AF_INET);
	
	if (host != nullptr) {
		resInforDomainName.officalName = host->h_name;
		resInforDomainName.aliasName.push_back(host->h_name);
		char** alias = host->h_aliases;
		int len = sizeof(alias);
		for (int i = 0; alias[i] != NULL; i++) {
			cout << "alias[" << i << ": " << alias[i] << endl;
			resInforDomainName.aliasName.push_back(alias[i]);
		}
	}
	else {
		resError.idError = "001";
		resError.nameError = "Not found information.\ngethostbyaddr(). error: " + WSAGetLastError(); 
		response.responseString = resError.toStringError();
	}
}

void processMessageFromClient(char buff[]) {
	cout << "process message from client " << endl;

	int cntDot = 0;
	int len = strlen(buff);
	for (int i = 0; i < len; i++) {
		if (buff[i] == '.') {
			cntDot++;
		}
	}

	cout << "cntDot = " << cntDot << endl;
	if (cntDot == 3) {
		cout << "request is IP" << endl;
		getHostByIP(buff);
		response.responseString = resInforDomainName.toStringInforDomainName();
	}
	else {
		cout << "request is domain name" << endl;
		getIPByDomainName(buff);
		response.responseString = resInforIP.toStringInforIP();
	}
}

void echoToClient(int ret, char res[], sockaddr_in clientAddr, int clientAddrLen) {
	cout << "res: " << res << endl;
	ret = sendto(server, res, BUFF_SIZE, 0, (sockaddr*)&clientAddr, clientAddrLen);
	if (ret == SOCKET_ERROR) {
		cout << "Error: " << WSAGetLastError() << endl;
	}
	else {
		cout << "Echo to client is successful" << endl;
	}
}

void communicateWithClient() {
	cout << "Server started: " << endl;
	sockaddr_in clientAddr;
	int ret;
	char buff[BUFF_SIZE];
	int clientAddrLen = sizeof(clientAddr);
	int clientPort;
	char clientIP[INET_ADDRSTRLEN];

	while (true) {
		ret = recvfrom(server, buff, BUFF_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrLen);

		if (ret == SOCKET_ERROR) {
			cout << "Error: " << WSAGetLastError() << endl;
		}
		else {
			buff[ret] = 0;
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP)); 
			clientPort = ntohs(clientAddr.sin_port);
			informationFromClient(clientIP, clientPort, buff);

			processMessageFromClient(buff);

			string responseString = response.responseString;

			char res[BUFF_SIZE];
			strcpy(res, responseString.c_str());
			echoToClient(ret, res, clientAddr, clientAddrLen);
		}
	}
}
int main() {
	if (initWinsock() == 0) {
		constructorSock();
		if (bindAddressSock() == 0) {
			communicateWithClient();
		}
	}
	return 0;
}