#include "Counter.h"

Counter::Counter():
m_totalTickCount(0),
m_perSecTickCount(0),
m_running(false)
{}

Counter::~Counter(){
	stop();
	if(m_thread->joinable()){
		m_thread->join();
		delete m_thread;
	} else
		m_thread->detach();
}

void Counter::start(){
	m_running = true;
	m_thread = new thread([&](){
		while(m_running.load()){
			{
				lock_guard<mutex> lock(m_mutex);
				for(function<void(uint64_t)> callback : m_callbacksList)
					callback(m_perSecTickCount);
			}
			m_perSecTickCount = 0;
			this_thread::sleep_for(seconds(1));
		}
	});
}

void Counter::stop(){
	m_running = false;
}

void Counter::tick(uint64_t tickCount){
	m_perSecTickCount += tickCount;
	m_totalTickCount += tickCount;
}

uint64_t Counter::getTotalTickCount(){
	return m_totalTickCount.load();
}

uint64_t Counter::getTickPerSecCount(){
	return m_perSecTickCount.load();
}

void Counter::registerCallback(function<void(uint64_t)> callback){
	lock_guard<mutex> lock(m_mutex);
	m_callbacksList.push_back(callback);
}

void Counter::resetCounter(){
	m_totalTickCount = 0;
	m_perSecTickCount = 0;
}
