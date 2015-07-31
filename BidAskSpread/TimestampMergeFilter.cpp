#include "TimestampMergeFilter.h"

TimestampMergeFilter::TimestampMergeFilter(ObsQueue* in, ObsQueue* out, MsgQueue* msg):
Filter(in, out, msg)
{}

void TimestampMergeFilter::filter(){
	Counter counter;
	counter.registerCallback([&](uint64_t countPerSec){
		string msg = to_string(countPerSec) + " line/s";
		if(m_inputQueue->empty()) msg += "\tstarving!";
		if(m_outputQueue->full()) msg += "\tblocked!";
		m_msgQueue->enqueue(Message(TIMESTAMP_MERGE_FILTER, msg));
	});
	counter.start();

	try{
		while(true){
			Observation obs = m_inputQueue->dequeue();
			if(m_buffer.empty()){
				m_buffer.push_front(obs);
			} else{
				if(*obs.date == *m_buffer.front().date && *obs.time == *m_buffer.front().time){
					m_buffer.push_front(obs);
				} else{
					m_outputQueue->enqueue(getMergedObs());
					m_buffer.clear();
					counter.tick();
					m_buffer.push_front(obs);
				}
			}
		}
	} catch(ObsQueue::QueueEndException&){
		if(!m_buffer.empty()){
			m_outputQueue->enqueue(getMergedObs());
			counter.stop();
			m_buffer.clear();
		}
		m_outputQueue->setQueueEnd();
		m_msgQueue->enqueue(Message(TIMESTAMP_MERGE_FILTER, "Finished !"));
	}
}

Observation TimestampMergeFilter::getMergedObs(){
	Observation result;

	vector<float> sortedBids;
	vector<float> sortedOffers;

	if(distance(m_buffer.begin(), m_buffer.end()) > 1){
		for(forward_list<Observation>::iterator it = m_buffer.begin(); it != m_buffer.end(); it++){
			sortedBids.push_back(*it->bid);
			sortedOffers.push_back(*it->offer);
		}

		//sort buffers
		sort(sortedBids.begin(), sortedBids.end());
		sort(sortedOffers.begin(), sortedOffers.end());

		float* medOffer = new float(StatsUtils::median(sortedOffers));
		float* medBid = new float(StatsUtils::median(sortedBids));

		result = Observation(
			m_buffer.front().symbol,
			m_buffer.front().date,
			m_buffer.front().time,
			medOffer,
			medBid,
			m_buffer.front().mode
			);

		//cleanup
		delete m_buffer.front().bid;
		delete m_buffer.front().offer;
		for(forward_list<Observation>::iterator it = ++m_buffer.begin(); it != m_buffer.end(); it++)
			it->deleteAll();
	} else{
		result = m_buffer.front();
	}
	return result;
}