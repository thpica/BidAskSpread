#include "RelSpreadProcessor.h"


RelSpreadProcessor::RelSpreadProcessor(ObsQueue* inQueue, ObsQueue* outQueue, MsgQueue* msg):
Filter(inQueue, outQueue, msg)
{}

inline void RelSpreadProcessor::run(){
	filter();
}

inline void RelSpreadProcessor::filter(){
	Counter counter;
	counter.registerCallback([&](uint64_t countPerSec){
		string msg = to_string(countPerSec) + " line/s";
		if(m_inputQueue->empty()) msg += "\tstarving!";
		if(m_outputQueue->full()) msg += "\tblocked!";
		m_msgQueue->enqueue(Message(RELSPREAD_PROCESSOR, msg));
	});
	counter.start();

	try{
		while(true){
			double* relSpread = nullptr;
			Observation obs = m_inputQueue->dequeue(); //poll obs

			relSpread = new double((2 * ((double)*obs.offer - (double)*obs.bid) / ((double)*obs.offer + (double)*obs.bid)));
			if(*relSpread >= 0){
				obs.relSpread = relSpread;
				m_outputQueue->enqueue(move(obs));
				counter.tick();
			} else{
				obs.deleteAll();
				delete relSpread; relSpread = nullptr;
			}
		}
	} catch(ObsQueue::QueueEndException&){
		counter.stop();
		m_outputQueue->setQueueEnd();
		m_msgQueue->enqueue(Message(RELSPREAD_PROCESSOR, "Finished !"));
	}
}