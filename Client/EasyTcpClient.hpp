#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_


#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#include<Windows.h>

#pragma comment(lib,"ws2_32.lib")

#include<iostream>
#include<thread>
#include"MessageHeader_c.hpp"
using namespace std;

static const int RecvBufSize = 10240;

class EasyTcpClient
{
private:
	SOCKET _sock;

	char szRecv[RecvBufSize] = {};
	char szMsgBuf[RecvBufSize * 10] = {}; //the second much bigger buff
	int lastPose = 0;

public:
	EasyTcpClient() {
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpClient(){
		Close();
	}

	void InitSocket() {
		if (_sock != INVALID_SOCKET) {
			Close();
		}
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);          //step1:creat a socket
		//return 1;
	}

	int Connect(const char* ip, unsigned short port,int i) {
		if (_sock == INVALID_SOCKET) {
			InitSocket();
		}
		sockaddr_in  _sin = {};                                  //step2:connect the server
		_sin.sin_family = AF_INET;//ipv4
		_sin.sin_port = htons(port);//host to net unsigned short
		_sin.sin_addr.S_un.S_addr =inet_addr(ip);//server's socket
		int ret = connect(_sock, (sockaddr*)& _sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret) {
			cout << "connect error!" << endl;
		}
		else cout<<i << "connect success!" << endl;

		return ret;

	}
	void Close() {
		if (_sock != INVALID_SOCKET) {
			closesocket(_sock);                                      //step7:close the socket;
			WSACleanup();
			_sock = INVALID_SOCKET;
		}
	}

	bool IsRun() {
		return _sock!= INVALID_SOCKET;
	}

	bool OnRun() {
		if (IsRun()) {
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 0,0 };
			int res = select(_sock, &fdReads, 0, 0, &t);
			if (res < 0) {
				cout << "1:The End!" << endl;
				return false;
			}
			if (FD_ISSET(_sock, &fdReads)) {
				FD_CLR(_sock, &fdReads);

				if (-1 == RecvData()) {
					cout << "2:The End!" << endl;
					return false;
				}

			}
			return true;
		}

		return false;

	}

	int RecvData() {

		int nlen = recv(_sock, (char*)& szRecv, RecvBufSize, 0);            //step6:recieve a massage from server;
		if (nlen <= 0) {
			cout << "Lose the connection with the server." << endl;
			return -1;
		}
		memcpy(szMsgBuf+lastPose, szRecv, nlen);
		lastPose += nlen;
		while (lastPose >= sizeof(DataHeader)) {
			DataHeader* header = (DataHeader*)szMsgBuf;
			if (lastPose >= header->DataLength) {
				//cout << header->cmd << header->DataLength << endl;
				int remain_size = lastPose - header->DataLength;
				ProcessData(header);
				memcpy(szMsgBuf, szMsgBuf + header->DataLength, remain_size);
				lastPose = remain_size;
			}
			else break;
		}


		return 1;

	}

	void ProcessData(DataHeader* header) {
		switch (header->cmd)                                          //step7:process the request from the server
		{
		case CMD_LOGIN_RESULT:
		{
			//a method of recieve data!!!
			LoginResult* loginret = (LoginResult*)header;
			//cout << loginret->DataLength << endl;
			cout << "Recieve the LoginResult from the server:" << loginret->result << ' ' << "DataLength:" << loginret->DataLength << endl;
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LogoutResult* logoutret = (LogoutResult*)header;
			cout << "Recieve the LogoutResult from the server:" << logoutret->result << ' ' << "DataLength:" << logoutret->DataLength << endl;
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* newuserjoin = (NewUserJoin*)header;
			cout << "A New User!" << endl;
		}
		break;
		}
	}

	int SendData(DataHeader* header) {
		if (IsRun() && header) {
			return send(_sock, (const char*)header, header->DataLength, 0);
		}
		return SOCKET_ERROR;
	}
};


#endif // !_EasyTcpClient_hpp_

