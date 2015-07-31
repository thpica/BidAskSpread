#pragma once

#include <vector>
#include <map>

#include "DataStructures.h"
#include "Filter.h"
#include "Counter.h"

using namespace std;

class RelSpreadProcessor: public Filter
{
public:

	RelSpreadProcessor(ObsQueue* inQueue, ObsQueue* outQueue, MsgQueue* msg);
	
	//no copy
	RelSpreadProcessor(const RelSpreadProcessor&) = delete;
	RelSpreadProcessor& operator=(const RelSpreadProcessor&) = delete;

private:
	virtual void run();
	virtual void filter();
};