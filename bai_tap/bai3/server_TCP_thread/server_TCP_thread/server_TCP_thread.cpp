#include <iostream>
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <fstream>
#include <vector>
#include <string.h>
#include <string>
#include <process.h> // khai bao thu vien su dung da luong thread
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable : 4996)
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

const int BUFF_SIZE = 2048;
WSADATA wsaData;
SOCKET server;
sockaddr_in serverAddr;

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

typedef struct informationUser {
	string username;
	string status;
    string toStringInfoUser() {
        string str = "";
        str = str + username + " " + status + "\n";
        return str;
    }
} User;

typedef struct informationPost {
	string content;
    string toStringContent() {
        return content;
    }
} Post;

typedef struct responseInforError {
    string idError;
    string nameError;
    string toStringError() {
        string str = "";
        str = str + "Error" + idError + ", error: " + nameError + "\n";
        return str;
    }
} ResError;

typedef struct ResponseInformation {
    string prefix;
    ResError resError;
    Post post;
    User user;
    string toStringResponse() {
        if (prefix == "OUSER") {
            return prefix + user.toStringInfoUser() + "\n";
        }
        if (prefix == "OPOST") {
            return prefix + post.toStringContent() + "\n";
        }
        else if (prefix == "ERROR") {
            return prefix + ": " + resError.toStringError() + "\n";
        }
        else {
            return "unknow\n";
        }
    }
} Response;

typedef struct informationLog {
    /*string dmy; // ngày tháng theo định dạng day/mon/year
    string hms; // giờ theo định dạng hour/minute/second
    string idResult;
    string toStringLog() {
        string str = "";
        str = str + dmy + " " + hms + " " + idResult + "\n";
        return str;
    }*/
    string requestString;
    string responseString;
    string toStringLog() {
        string str = "";
        str = str + requestString + " " + responseString + " ";
        return str;
    }
} CustomLog;

ResError resError;
Response response;
Post post;
CustomLog customLog;
User user;

User convertToUser(string line) {
    User user;
    int len = line.size();
    string username;
    string status;
    bool sign = true;
    for (int i = 0; i < len; i++) {
        if (line[i] == 32) {
            sign = false;
            continue;
        }
        else {
            if (sign == true) {
                username += line[i];
            }
            else {
                status += line[i];
            }
        }
    }
    cout << "username: " << username << ", status: " << status << endl;
    user.username = username;
    user.status = status;
    return user;
}

vector<string> readFile() {
    string filename("toantang.txt");
    // Khai báo vector để lưu các dòng đọc được
    vector<string> lines;
    string line;

    //Mở file bằng ifstream
    ifstream input_file(filename);
    //Kiểm tra file đã mở thành công chưa
    if (!input_file.is_open()) {
        cerr << "Could not open the file - '"
            << filename << "'" << endl;
        return lines;
    }

    //Đọc từng dòng trong
    while (getline(input_file, line)) {
        lines.push_back(line);
    }
    
    input_file.close();
    return lines;
}

void writeFile(string log) {
    ofstream ofs("log_20183998.txt");
    if (!ofs) {
        cerr << "Error: file not opened." << endl;
        return;
    }
    ofs << log << endl;
    ofs.close();
}

string getHeader(string request) {
    cout << "header is: " << request.substr(0, 4) << endl;
    return request.substr(0, 4);
}

string getBody(string request) {
    cout << "body is: " << request.substr(4) << endl;
    string str = request.substr(4);
    str.erase(str.size() - 1);
    return str;
}

int login(string body) {
    vector<string> listUser = readFile();
    //cout << "size list user: " << listUser.size() << endl;
    for (int i = 0; i < listUser.size(); i++) {
        cout << "lines[" << i << "]: " << listUser[i] << endl;
        User userInList = convertToUser(listUser[i]);
        //cout << "user.name = " << userInList.username << ", body: " << body << endl;
        if (userInList.username == body) {
            //cout << "username is correct" << endl;
            if (userInList.status == "0") {
                //cout << "status is correct" << endl;
                user = userInList;
                return 0;
            }
            else {
                return 2;// tai khoan da dc dang nhap
            }
        }
    };

    return 1; // tai khoan ko co trong file
}

int postOneStatus(string content) {
    if (content.size() > 0) {
        post.content = content;
        return 0;
    }
    return 1;
}

int logout() {
    return 0;
}
unsigned __stdcall handleSocket(void* param) {
    char buff[BUFF_SIZE];
    int ret;
    SOCKET connectedSocket = (SOCKET)param;
    sockaddr_in clientAddr;
    int clientAdrrLen = sizeof(clientAddr);
    
    while (true) {
        //receive message from client
        ret = recv(connectedSocket, buff, BUFF_SIZE, 0);

        string reiceveMessage;
        string errorCode;
        string prefix;

        if (ret == SOCKET_ERROR) {
            reiceveMessage = WSAGetLastError();
            errorCode = "01";
            break;
        }
        else if (ret == 0) {
            reiceveMessage = "client disconnect";
            errorCode = "02";
            break;
        }
        else if (strlen(buff) > 0) {
            buff[ret] = 0;

            string headerIP = getHeader(buff);
            //const char* typeRequest = headerIP.c_str();
            string body = getBody(buff);
            reiceveMessage = body;

            if (headerIP == "USER") {
                cout << "thuc hien switch user" << endl;
                int loginCode = login(body);
                cout << "logincode: " << loginCode << endl;
                switch (loginCode) {
                case 0:
                    errorCode = "10";
                    prefix = "OUSER";
                    break;
                case 1:
                    errorCode = "11";
                    prefix = "ERROR";
                    break;
                case 2:
                    errorCode = "12";
                    prefix = "ERROR";
                    break;
                default:
                    errorCode = "19"; 
                    prefix = "ERROR";
                    break;
                }
            }
            else if (headerIP == "POST") {
                int postCode = postOneStatus(body);
                switch (postCode) {
                case 0:
                    errorCode = "20";
                    prefix = "OPOST";
                    break;
                case 1:
                    errorCode = "21";
                    prefix = "ERROR";
                    break;
                default:
                    errorCode = "29";
                    prefix = "ERROR";
                    break;
                }
            }
            else if (headerIP == "QUIT") {
                int logoutCode = logout();
                switch (logoutCode) {
                case 0:
                    errorCode = "30";
                    prefix = "ERROR";
                    break;
                default:
                    errorCode = "39";
                    prefix = "ERROR";
                    break;
                }
            }
            else {
                cout << "unknow method" << endl;
                errorCode = "41";
                prefix = "ERROR";
            }
        }
        else {
            cout << "unknow method" << endl;
            reiceveMessage = "unknow method";
            prefix = "ERROR";
        }
        
        customLog.requestString = reiceveMessage;
        customLog.responseString = errorCode;
        resError.idError = errorCode;
        writeFile(customLog.toStringLog());

        response.prefix = prefix;
        response.resError = resError;
        response.user = user;

        string res = response.toStringResponse();
        cout << "res = " << res << endl;
        ret = send(connectedSocket, res.c_str(), res.size(), 0);
    }
}

void communicateWithClient() {
    cout << "Server started" << endl;
    SOCKET connSock;
    sockaddr_in clientAddr;
    char buff[BUFF_SIZE];
    char clientIP[INET_ADDRSTRLEN];
    int ret;
    int clientPort;
    int clientAddrLen = sizeof(clientAddr);
    while (true) {
        connSock = accept(server, (sockaddr*)&clientAddr, &clientAddrLen);
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
        _beginthreadex(0, 0, handleSocket, (void*)connSock, 0, 0);
    }
}

int main() {
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