#pragma once

#include <vector>
#include <forward_list>
#include <future>
#include <atomic>
#include <thread>

#include "Filter.h"
#include "StatsUtils.h"

using namespace std;

class TimestampMergeFilter: public Filter{
public:
	TimestampMergeFilter(ObsQueue* in, ObsQueue* out, MsgQueue* msg);

	//no copy
	TimestampMergeFilter(const TimestampMergeFilter&) = delete;
	TimestampMergeFilter& operator=(const TimestampMergeFilter&) = delete;

private:
	forward_list<Observation> m_buffer;
	virtual void filter();
	Observation getMergedObs();
};