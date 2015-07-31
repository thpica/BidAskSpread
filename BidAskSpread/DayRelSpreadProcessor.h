#pragma once

#include "Callable.h"
#include "DataStructures.h"
#include "StatsUtils.h"
using namespace StatsUtils;
#include "Counter.h"

class DayRelSpreadProcessor: public Callable
{
public:
	DayRelSpreadProcessor(ObsQueue* inputQueue, SprdQueue* outputQueue, MsgQueue* msgQueue);

	//no copy
	DayRelSpreadProcessor(const DayRelSpreadProcessor&) = delete;
	DayRelSpreadProcessor& operator=(const DayRelSpreadProcessor&) = delete;
	
private:
	vector<Observation> m_buffer;
	ObsQueue *m_inputQueue;
	SprdQueue *m_outputQueue;
	MsgQueue *m_msgQueue;
	virtual void run(){ process(); }
	void pollDayObs();
	void cleanBuffer();
	void process();
};