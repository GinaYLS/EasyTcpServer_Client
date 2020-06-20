#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_


#define FD_SETSIZE  2506

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#include<Windows.h>

#pragma comment(lib,"ws2_32.lib")

#include<iostream>
#include<vector>
#include<thread>
#include<mutex>
#include<atomic>
#include<memory>
//#include<functional>  mem_fun

#include"MessageHeader_s.hpp"
#include"CELLTimestamp.hpp"
#include"CellTask.hpp"
using namespace std;

static const int RecvBufSize = 10240;
static const int SendBufSize = 10240;

class ClientSocket
{
	SOCKET _sockfd;
	char szMsgBuf[RecvBufSize] = {}; //the second much bigger recive buff
	int lastPose = 0;

	char szSendBuf[SendBufSize] = {}; //the second much bigger send buff
	int lastSendPose = 0;
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET) {
		_sockfd = sockfd;
		memset(szMsgBuf, 0, RecvBufSize);
		lastPose = 0;

		memset(szSendBuf, 0, SendBufSize);
		lastSendPose = 0;
	}

	SOCKET Get_sockfd() {
		return _sockfd;
	}

	char* GetMsgBuf() {
		return szMsgBuf;
	}

	int GetlastPose() {
		return lastPose;
	}

	void SetlastPose(int pose) {
		lastPose = pose;
	}
	int SendData(DataHeader* header) {
		int ret = SOCKET_ERROR;
		int sendlen = header->DataLength;
		const char* pSend = (const char*)header;
		while (lastSendPose + sendlen >= SendBufSize) {
			int cplen = SendBufSize - lastSendPose;
			memcpy(szSendBuf + lastSendPose, pSend, cplen);
			pSend += cplen;
			sendlen -= cplen;
			ret = send(_sockfd, szSendBuf, SendBufSize, 0);
			if (ret == SOCKET_ERROR) return ret;
			lastSendPose = 0;
		}

		memcpy(szSendBuf + lastSendPose, pSend, sendlen);
		lastSendPose += sendlen;

		return ret;
	}
};

class INetEvent {   //internet event
public:
	virtual void Oneleave(ClientSocket* pClient) = 0; //pure virtual  a client leave
	//virtual void ProcessData(CellServer* pcellserver, SOCKET _cSock, DataHeader* header) = 0; //process a massage
};
class CellSendMsg :public CellTask
{
	ClientSocket* pClient;
	DataHeader* pHeader;
public:
	CellSendMsg(ClientSocket* Client, DataHeader* Header) {
		pClient = Client;
		pHeader = Header;
	}
	virtual void DoTask() {
		pClient->SendData(pHeader);
		delete pHeader;
	}

};
typedef shared_ptr<CellSendMsg> CellSendMsgPtr;
class CellServer
{
	SOCKET _sock;
	vector<ClientSocket*> g_clients;
	vector<ClientSocket*> clientsBuff;
	mutex _mutex;
	thread* pthread;
	INetEvent* pNetEvent; //main server
	CellTaskServer taskServer;

	fd_set fd_copy;
	bool clients_change;
	SOCKET maxsock;
public:
	atomic_int _recvCount;
	atomic_int _sendCount;
public:
	CellServer(SOCKET sock = INVALID_SOCKET) {
		_sock = sock;
		pthread = nullptr;
		_recvCount = 0;
		_sendCount = 0;
		pNetEvent = nullptr;
	}

	void SetEvent(INetEvent* event) {
		pNetEvent = event;
	}

	~CellServer() {
		delete pthread;
		Close();
		_sock = INVALID_SOCKET;
	}
	bool IsRun() {
		return _sock != INVALID_SOCKET;
	}
	void Close() {
		if (_sock != INVALID_SOCKET) {
			for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
				closesocket(g_clients[n]->Get_sockfd());
				delete g_clients[n];
			}
			closesocket(_sock);                                      //step7:close the socket;
			_sock = INVALID_SOCKET;
			g_clients.clear();
		}

	}

	bool OnRun() {
		clients_change = true;
		while (IsRun()) {
			if (clientsBuff.size() > 0) {
				lock_guard<mutex> lock(_mutex);
				for (auto client : clientsBuff) {
					g_clients.push_back(client);
				}
				clientsBuff.clear();
				clients_change = true;
			}
			if (g_clients.empty()) {
				chrono::milliseconds t(1);
				this_thread::sleep_for(t);
				continue;
			}

			fd_set fdRead;  //the union of socket
			FD_ZERO(&fdRead);
			maxsock = g_clients[0]->Get_sockfd();
			if (clients_change) {
				clients_change = false;
				maxsock = g_clients[0]->Get_sockfd();
				for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
					FD_SET(g_clients[n]->Get_sockfd(), &fdRead);
					if (maxsock < g_clients[n]->Get_sockfd()) {
						maxsock = g_clients[n]->Get_sockfd();
					}
				}
				memcpy(&fd_copy, &fdRead, sizeof(fd_set));
			}
			else {
				memcpy(&fdRead, &fd_copy, sizeof(fd_set));
			}

			int ret = select(maxsock + 1, &fdRead, nullptr, nullptr, nullptr);
			if (ret < 0) {
				cout << "The End;" << endl;
				Close();
				return false;
			}

			for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
				if (FD_ISSET(g_clients[n]->Get_sockfd(), &fdRead)) {
					if (-1 == RecvData(g_clients[n])) {
						auto iter = g_clients.begin() + n;
						if (iter != g_clients.end()) {
							clients_change = false;
							if (pNetEvent) {
								pNetEvent->Oneleave(g_clients[n]);
							}
							delete g_clients[n];
							g_clients.erase(iter);
						}
					}
				}
			}
			//return true;
		}
		//return false;
	}
	void Start() {
		pthread = new thread(&CellServer::OnRun, this);
		taskServer.Start();
	}
	int RecvData(ClientSocket* pSock) {
		char* szRecv = pSock->GetMsgBuf() + pSock->GetlastPose();
		int nlen = recv(pSock->Get_sockfd(), szRecv, RecvBufSize - pSock->GetlastPose(), 0);            //step6:recieve a massage from client;
		if (nlen <= 0) {
			//cout << "The client has gone." << endl;
			return -1;
		}
		pSock->SetlastPose(pSock->GetlastPose() + nlen);
		while (pSock->GetlastPose() > sizeof(DataHeader)) {
			DataHeader* header = (DataHeader*)pSock->GetMsgBuf();
			if (pSock->GetlastPose() >= header->DataLength) {
				int remain_size = pSock->GetlastPose() - header->DataLength;
				ProcessData(pSock, header);
				memcpy(pSock->GetMsgBuf(), pSock->GetMsgBuf() + header->DataLength, remain_size);
				pSock->SetlastPose(remain_size);
			}
			else break;
		}
		return 1;
	}
	void addSendTask(ClientSocket* pClient, DataHeader* Header) {
		//CellSendMsg* task = new CellSendMsg(pClient, Header);
		CellSendMsgPtr task = make_shared<CellSendMsg>(pClient, Header);
		taskServer.addTask(task);
	}
	void ProcessData(ClientSocket* pSock, DataHeader* header) {

		_recvCount++;

		switch (header->cmd)                                          //step7:process the request from the client
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			//cout << "Recieve a CMD:" << "LOGIN" << ' ' << "DataLength:" << login->DataLength << endl;
			LoginResult* ret = new LoginResult();
			addSendTask(pSock, ret);
			_sendCount++;
		}
		break;

		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			//cout << "Recieve a CMD:" << "LOGOUT" << ' ' << "DataLength:" << logout->DataLength << endl;
			//LogoutResult ret;
			//pSock->SendData(&ret);
		}
		break;
		default:
			DataHeader header;
			pSock->SendData(&header);
			break;
		}

	}

	void AddClient(ClientSocket* psock) {
		lock_guard<mutex> lock(_mutex);
		//_mutex.lock();
		clientsBuff.push_back(psock);
		//_mutex.unlock();
	}
	size_t GetCcount() {
		return g_clients.size() + clientsBuff.size();
	}


};


class EasyTcpServer :public INetEvent
{
	SOCKET _sock;
	vector< CellServer*> cellServers;
	CELLTimestamp _tTime;
public:
	EasyTcpServer() {
		_sock = INVALID_SOCKET;
	}
	virtual ~EasyTcpServer() {
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
	}

	int Bind(const char* ip, unsigned short port) {
		if (_sock == INVALID_SOCKET) {
			InitSocket();
		}
		sockaddr_in  _sin = {};                                  //step2:bind a port to recieve massage from client
		_sin.sin_family = AF_INET;//ipv4
		_sin.sin_port = htons(port);//host to net unsigned short
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip); //inet_addr("127.0.0.1"),this computer
		}
		else {
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}

		int ret = bind(_sock, (sockaddr*)& _sin, sizeof(_sin));

		if (SOCKET_ERROR == ret) {
			cout << "bind error!" << endl;
		}
		else {
			cout << "bind success!" << endl;
			return ret;
		}
	}

	int Listen(int n) {
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			cout << "listen error!" << endl;
			return false;
		}
		else cout << "listen success!" << endl;
		return ret;
	}

	SOCKET Accept() {
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;  //client's socket
		cSock = accept(_sock, (sockaddr*)& clientAddr, &nAddrLen);
		if (cSock == INVALID_SOCKET) {
			cout << "INVALID_SOCKET" << endl;
		}

		//NewUserJoin userJoin;
		//SendDataToAll(&userJoin);

		AddClient(new ClientSocket(cSock));
		//cout << "A new client:" << (int)cSock << endl;
		return cSock;
	}

	void AddClient(ClientSocket* pClient) {
		auto MinServer = cellServers[0];
		for (auto s : cellServers) {
			if (MinServer->GetCcount() > s->GetCcount())
				MinServer = s;
		}
		MinServer->AddClient(pClient);
	}

	void Start(int CELLSERVERS_COUNT) {
		for (int i = 0; i < CELLSERVERS_COUNT; i++) {
			auto ser = new CellServer(_sock);
			cellServers.push_back(ser);
			ser->SetEvent(this);
			ser->Start();
		}
	}

	void Close() {
		if (_sock != INVALID_SOCKET) {
			closesocket(_sock);                                      //step7:close the socket;
			WSACleanup();
			_sock = INVALID_SOCKET;
		}

	}

	bool IsRun() {
		return _sock != INVALID_SOCKET;
	}

	bool OnRun() {
		if (IsRun()) {
			Time4Msg();
			fd_set fdRead;
			//fd_set fdWrite;
			//fd_set fdExp;

			FD_ZERO(&fdRead);
			//FD_ZERO(&fdWrite);
			//FD_ZERO(&fdExp);

			FD_SET(_sock, &fdRead);
			//FD_SET(_sock, &fdWrite);
			//FD_SET(_sock, &fdExp);

			timeval t = { 0,10 };  //if not set this, the select will be block when their is no clients.

			int ret = select(_sock + 1, &fdRead, 0, 0, &t);
			if (ret < 0) {
				cout << "The End;" << endl;
				Close();
				return false;
			}

			if (FD_ISSET(_sock, &fdRead)) {
				FD_CLR(_sock, &fdRead);
				Accept();
			}

			return true;
		}
		return false;
	}


	void Time4Msg() {

		auto t = _tTime.getElapsedSecond();
		if (t >= 1.0) {
			int rCount = 0, rCCount = 0, sCount = 0;
			for (auto ser : cellServers) {
				rCount += ser->_recvCount;
				sCount += ser->_sendCount;
				rCCount += ser->GetCcount();
				ser->_recvCount = 0;
				ser->_sendCount = 0;
			}
			cout << "Time:" << t << "****" << "Socket:" << rCCount << "****" << "RECVCOUNT:" << (int)(rCount / t) << "****" << "SEND:" << (int)(sCount / t) << endl;
			_tTime.updata();
			rCount = 0;
		}

	}



	virtual void Oneleave(ClientSocket* pClient) {

	}

};

#endif 
