#pragma once
#include <atomic>
#include <mutex>
#include <thread>
#include <cstdint>
#include <functional>
#include <chrono>
using namespace std::chrono;
#include <vector>

using namespace std;

class Counter{
public:
	Counter();
	~Counter();
	void start();
	void stop();
	void tick(){ ticks(1); }
	void ticks(uint64_t count);
	uint64_t getTotalTickCount();
	uint64_t getTickPerSecCount();
	void registerCallback(function<void(uint64_t)> callback);
	void resetCounter();
private:
	thread* m_thread;
	atomic<bool> m_running;
	system_clock::time_point m_secStart;
	atomic<uint64_t> m_perSecTickCount;
	atomic<uint64_t> m_totalTickCount;
	mutex m_mutex;
	vector<function<void(uint64_t)>> m_callbacksList;
};