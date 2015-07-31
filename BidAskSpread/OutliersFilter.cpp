#include "OutliersFilter.h"


OutliersFilter::OutliersFilter(ObsQueue* in, ObsQueue* out, MsgQueue* msg):
	Filter(in, out, msg)
{}

void OutliersFilter::filter(){
	Counter counter;
	Observation obs;

	counter.registerCallback([&](uint64_t countPerSec){
		string msg = to_string(countPerSec) + " line/s";
		if(m_inputQueue->empty()) msg += "\tstarving!";
		if(m_outputQueue->full()) msg += "\tblocked!";
		m_msgQueue->enqueue(Message(OUTLIER_FILTER, msg));
	});
	counter.start();

	try{
		while(true){
			obs = m_inputQueue->dequeue();
			if(m_buffer.empty()){
				m_buffer.push_back(obs);
			} else if(*m_buffer.back().date == *obs.date){
				m_buffer.push_back(obs);
				//TODO: acquire only [window size] Obs and define state (better streaming capabilities)
			} else{
				removeDayOutliers(counter);
				//push buffer to output queue
				for(int i = 0; i < m_buffer.size(); i++){
					if(m_buffer[i].symbol != nullptr)
						m_outputQueue->enqueue(move(m_buffer.at(i)));
				}
				m_buffer.clear();
				m_buffer.push_back(obs);
			}
		}
	} catch(ObsQueue::QueueEndException&){
		if(!m_buffer.empty()){
			removeDayOutliers(counter);
			for(int i = 0; i < m_buffer.size(); i++){
				if(m_buffer[i].symbol != nullptr)
					m_outputQueue->enqueue(move(m_buffer.at(i)));
			}
		}
		counter.stop();
		m_msgQueue->enqueue(Message(OUTLIER_FILTER, "Finished !"));
		m_outputQueue->setQueueEnd();
	}
}

void OutliersFilter::removeDayOutliers(Counter& counter){
	const int windowSize = 1801;
	vector<size_t> deleteFlags;

	//filter
	for(size_t i = 0; i < m_buffer.size(); i++){
		Interval window;
		if(i <= (windowSize - 1) / 2)
			window.set(0, (windowSize - 1), i);
		else if(m_buffer.size() - i < (windowSize + 1) / 2)
			window.set(m_buffer.size() - ((windowSize + 1) / 2), m_buffer.size() - 1, i - (m_buffer.size() - ((windowSize + 1) / 2)));
		else
			window.set(i - ((windowSize - 1) / 2), i + ((windowSize - 1) / 2), ((windowSize - 1) / 2));

		Stats stats = computeContextStats(window);
		if(abs(*m_buffer[i].bid - stats.bid.trimmedMean) >(3 * stats.bid.sd + 0.5) ||
			abs(*m_buffer[i].offer - stats.offer.trimmedMean) > (3 * stats.offer.sd + 0.5))
		{
			deleteFlags.push_back(i);
		}
		counter.tick();
	}

	//clean buffer
	for(int i = 0; i < deleteFlags.size(); i++){
		m_buffer[deleteFlags[i]].deleteAll();
	}
	deleteFlags.clear();
}

OutliersFilter::Stats OutliersFilter::computeContextStats(Interval window){
	Stats result;

	vector<float> sortedContextBids;
	vector<float> sortedContextOffers;
	for(int i = 0; i < window.size() && i < m_buffer.size(); i++){
		if(i != window.current){
			sortedContextBids.push_back(*m_buffer.at(window.low + i).bid);
			sortedContextOffers.push_back(*m_buffer.at(window.low + i).offer);
		}
	}

	sort(sortedContextBids.begin(), sortedContextBids.end());
	sort(sortedContextOffers.begin(), sortedContextOffers.end());
	
	result.bid.mean = StatsUtils::mean(sortedContextBids);
	result.bid.trimmedMean = StatsUtils::trimmedMean(sortedContextBids, 0.1);
	result.bid.median = StatsUtils::median(sortedContextBids);
	result.bid.sd = StatsUtils::sampleSD(sortedContextBids, result.bid.mean);

	result.offer.mean = StatsUtils::mean(sortedContextOffers);
	result.offer.trimmedMean = StatsUtils::trimmedMean(sortedContextOffers, 0.1);
	result.offer.median = StatsUtils::median(sortedContextOffers);
	result.offer.sd = StatsUtils::sampleSD(sortedContextOffers, result.offer.mean);
	
	return result;
}