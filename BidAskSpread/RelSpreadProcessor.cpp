#include "RelSpreadProcessor.h"


RelSpreadProcessor::RelSpreadProcessor(ObsQueue* inQueue, ObsQueue* outQueue, MsgQueue* msg):
Filter(inQueue, outQueue, msg)
{}

inline void RelSpreadProcessor::run(){
	filter();
}

inline void RelSpreadProcessor::filter(){
	while(!m_inputQueue->end()){
		if(m_inputQueue->empty()){
			Sleep(25);
		} else{
			long double* relSpread = nullptr;
			Observation obs = m_inputQueue->dequeue(); //poll obs

			relSpread = new long double((2 * ((long double)*obs.offer - (long double)*obs.bid) / ((long double)*obs.offer + (long double)*obs.bid)));
			if(*relSpread >= 0){
				obs.relSpread = relSpread;
				m_outputQueue->enqueue(move(obs));
			} else{
				obs.deleteAll();
				delete relSpread; relSpread = nullptr;
			}
		}
	}
	m_outputQueue->setQueueEnd();
}