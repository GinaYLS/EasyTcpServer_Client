#ifndef _CELL_TIMESTAMP_
#define _CELL_TIMESTAMP_

#include<chrono>
using namespace std::chrono;

class CELLTimestamp {
	time_point<high_resolution_clock> _begin;
public:
	CELLTimestamp() {
		updata();
	}
	~CELLTimestamp() {};

	void updata() {
		_begin = high_resolution_clock::now();  //get the second
	}

	double getElapsedSecond() {
		return this->getElapsedTimeInMicrosec() * 0.000001;
	}
	double getElapsedMilliSec() {
		return this->getElapsedTimeInMicrosec() * 0.001;
	}
	long long getElapsedTimeInMicrosec() {
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	}

};

#endif // !_CELL_TIMESTAMP_

