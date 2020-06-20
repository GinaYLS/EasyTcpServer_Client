#ifndef _CELL_TASK_
#define _CELL_TASK_

#include<iostream>
#include<thread>
#include<mutex>
#include<list>
using namespace std;

class CellTask {
public:
	CellTask() {};
	virtual ~CellTask() {};
	virtual void DoTask() {};
};
class CellTaskServer {
	list<shared_ptr<CellTask>> Tasks;
	list<shared_ptr<CellTask>> Tasks_Buf;
	mutex _mutex;
public:
	CellTaskServer() {

	}
	~CellTaskServer() {

	}
	void Start() {
		thread t(&CellTaskServer::OnRun, this);
		t.detach();
	}
	void addTask(shared_ptr<CellTask> task) {
		lock_guard<mutex> lock(_mutex);
		Tasks_Buf.push_back(task);
	}
	void OnRun() {
		while (1) {
			if (!Tasks_Buf.empty()) {
				lock_guard<mutex> lock(_mutex);
				for (auto t : Tasks_Buf) {
					Tasks.push_back(t);
				}
				Tasks_Buf.clear();
			}
			if (Tasks.empty()) {
				chrono::milliseconds t(1);
				this_thread::sleep_for(t);
				continue;
			}
			for (auto t : Tasks) {
				t->DoTask();
				//delete t;
			}
			Tasks.clear();

		}

	}
};

#endif // !_CELL_TASK_
