#include "DayRelSpreadProcessor.h"


DayRelSpreadProcessor::DayRelSpreadProcessor(ObsQueue* inputQueue, SprdQueue* outputQueue, MsgQueue* msgQueue):
m_inputQueue(inputQueue), m_outputQueue(outputQueue), m_msgQueue(msgQueue)
{}

void DayRelSpreadProcessor::pollDayObs(){
	Observation obs;
	while(!m_inputQueue->end()){
		if(m_inputQueue->empty()){
			Sleep(25);
		} else{
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
	while(!m_inputQueue->end()){
		pollDayObs();
		if(!m_buffer.empty()){
			m_outputQueue->enqueue(
				DaySpread(m_buffer[0].symbol,
						  m_buffer[0].date,
						  mean(m_buffer)
				         )
				);
			cleanBuffer();
		}
	}
	m_outputQueue->setQueueEnd();
}

