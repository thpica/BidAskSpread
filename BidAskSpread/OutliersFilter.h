#pragma once

#include <vector>
#include <numeric>
#include <utility>
#include <iostream>

using namespace std;

#include "Filter.h"
#include "StatsUtils.h"

class OutliersFilter: public Filter{
public:

	struct Interval{
		size_t low;
		size_t high;
		size_t current;
		Interval(){}
		Interval(size_t l, size_t h, size_t c): low(l), high(h), current(c){}
		void set(size_t l, size_t h, size_t c){ low = l; high = h; current = c; }
		size_t size(){ return (high - low) + 1; }
	};

	struct Stats{
		struct Bid{
			long double mean;
			long double trimmedMean;
			long double median;
			long double sd;
			Bid(){}
		} bid;
		struct Offer{
			long double mean;
			long double trimmedMean;
			long double median;
			long double sd;
			Offer(){}
		} offer;
		Stats(): bid(), offer(){}
	};

	OutliersFilter(ObsQueue* in, ObsQueue* out, MsgQueue* msg);

	//no copy
	OutliersFilter(const OutliersFilter&) = delete;
	OutliersFilter& operator=(const OutliersFilter&) = delete;

private:
	vector<Observation> m_buffer;
	void removeDayOutliers();
	virtual void filter();
	Stats computeContextStats(Interval window);
};