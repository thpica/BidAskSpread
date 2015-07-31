#pragma once

#include <vector>
#include <algorithm>

#include "DataStructures.h"

namespace StatsUtils{
	using namespace std;

	static long double* mean(const vector<Observation>& values){
		long double* sum = new long double(0.0); //KahanSum<Observation>()(values.begin(), values.end());
		for(size_t i = 0; i < values.size(); i++)
			*sum += *values[i].relSpread;
		*sum /= values.size();
		return sum;
	}

	static long double mean(const vector<float>& values){
		long double sum = 0.0; //KahanSum<float>()(values);
		for(size_t i = 0; i < values.size(); i++)
			sum += values[i];
		return sum / values.size();
	}

	static long double trimmedMean(const vector<float>& values, const float trimmFactor){
		int skip = static_cast<int>(round((values.size() - 1) * trimmFactor));
		long double sum = 0.0; //KahanSum<float>()(values.begin() + skip, values.end() - skip);
		for(size_t i = skip; i < (values.size() - skip); i++)
			sum += values[i];
		return sum / (values.size() - 2 * skip);
	}

	static long double sampleSD(const vector<float>& values, const long double& mean){
		long double variance = 0.0;
		for(size_t i = 0; i < values.size(); i++)
			variance += ((values[i] - mean) * (values[i] - mean));
		variance /= values.size() - 1;
		return sqrt(variance);
	}

	static float median(const vector<float>& sortedValues){
		return (sortedValues.size() % 2 == 0) ?
			(sortedValues[(sortedValues.size() / 2) - 1] + sortedValues[(sortedValues.size() / 2)]) / 2 :
			sortedValues[sortedValues.size() / 2];
	}

	static const float* median(const vector<const float*>& sortedValues){
		return new float((sortedValues.size() % 2 == 0) ?
			(*sortedValues[(sortedValues.size() / 2) - 1] + *sortedValues[(sortedValues.size() / 2)]) / 2 :
			*sortedValues[sortedValues.size() / 2]);
	}
}