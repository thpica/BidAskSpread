#include "DayRelSpreadProcessor.h"


DayRelSpreadProcessor::DayRelSpreadProcessor(ObsQueue* inputQueue, SprdQueue* outputQueue, MsgQueue* msgQueue):
m_inputQueue(inputQueue), m_outputQueue(outputQueue), m_msgQueue(msgQueue)
{}

void DayRelSpreadProcessor::pollDayObs(){ //throws QueueEndException
	Observation obs;

	while(true){
		if(m_buffer.empty()){
			obs = m_inputQueue->dequeue();
			m_buffer.push_back(obs);
		} else if(*m_buffer.back().date == *m_inputQueue->peek()->date){
			obs = m_inputQueue->dequeue();
			m_buffer.push_back(obs);
		} else{
			break;
		}
	}
}

void DayRelSpreadProcessor::cleanBuffer(){
	delete m_buffer[0].bid; delete m_buffer[0].offer;
	delete m_buffer[0].time; delete m_buffer[0].mode;
	delete m_buffer[0].relSpread;
	for(size_t i = 1; i < m_buffer.size(); i++)
		m_buffer[i].deleteAll();
	m_buffer.clear();
}

void DayRelSpreadProcessor::process(){
	Counter counter;

	counter.registerCallback([&](uint64_t countPerSec){
		string msg = to_string(countPerSec) + " line/s";
		if(m_inputQueue->empty()) msg += "\tstarving!";
		if(m_outputQueue->full()) msg += "\tblocked!";
		m_msgQueue->enqueue(Message(DAY_RELSPREAD_PROCESSOR, msg));
	});
	counter.start();

	try{
		while(true){
			pollDayObs();
			if(!m_buffer.empty()){
				m_outputQueue->enqueue(
					DaySpread(m_buffer[0].symbol,
					m_buffer[0].date,
					mean(m_buffer)
					)
					);
				counter.tick();
				cleanBuffer();
			}
		}
	} catch(ObsQueue::QueueEndException&){
		if(!m_buffer.empty()){
			m_outputQueue->enqueue(DaySpread(m_buffer[0].symbol, m_buffer[0].date,	mean(m_buffer)));
			counter.tick();
			cleanBuffer();
		}
		m_outputQueue->setQueueEnd();
		counter.stop();
		m_msgQueue->enqueue(Message(DAY_RELSPREAD_PROCESSOR, "Finished !"));
	}
}

