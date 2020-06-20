#include<iostream>
#include"EsayTcpServer.hpp"
using namespace std;

bool g_bRun = true;


void cmdThread()
{
	while (true) {
		char cmdBuf[256] = {};
		cin >> cmdBuf;
		if (0 == strcmp(cmdBuf, "exit")) {
			cout << "Exit!" << endl;
			g_bRun = false;
			return;
		}
		else {
			cout << "No such CMD!" << endl;
		}

	}

}

class MyServer: public EasyTcpServer   //can add new functions
{
public:
	virtual void Oneleave(ClientSocket* pClient) {

	}
};

int main() {

	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	server.Start(4);

	thread t1(cmdThread); //start my thread
	t1.detach();

	while (g_bRun){

		server.OnRun();
		//cout << "Be free!" << endl;
	}
	server.Close();	                                 
	cout << "To end." << endl;
	getchar();
	return 0;
}
