#include"EasyTcpClient.hpp"

bool g_bRun = true;
const int cCount = 10000;
const int tCount = 4;
EasyTcpClient* client[cCount];
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
void sendThread(int id) {
	int begin = (id - 1) * (cCount / tCount);
	int end = id * (cCount / tCount);
	for (int i = begin; i < end; i++) {
		client[i] = new EasyTcpClient();
		client[i]->Connect("127.0.0.1", 4567, i);
	}
	chrono::milliseconds ts(5000);
	this_thread::sleep_for(ts);

	Login login;
	strcpy(login.UserName, "Gina");
	strcpy(login.PassWord, "iamgina");


	while (g_bRun) {
		//client1.OnRun();
		for (int i = begin; i < end; i++) {
			client[i]->SendData(&login);
			//client[i]->OnRun();       //send data only
		}
	}

	for (int i = begin; i < end; i++) {
		client[i]->Close();
	}
}

int main() {

	thread t(cmdThread); //start my thread
	t.detach();

	for (int i = 0; i < tCount; i++) {
		thread t1(sendThread, i + 1);
		t1.detach();
	}



	cout << "To end." << endl;
	getchar();
	return 0;
}
