#pragma once

#include <algorithm>
#include "DataStructures.h"
#include "Callable.h"

class Filter: public Callable {
public:
	Filter(ObsQueue* input, ObsQueue* output, MsgQueue* msg):
		m_inputQueue(input),
		m_outputQueue(output),
		m_msgQueue(msg)
	{}

	virtual ~Filter(){};

protected:
	ObsQueue* m_inputQueue;
	ObsQueue* m_outputQueue;
	MsgQueue* m_msgQueue;

	virtual void run(){ 
		try{
			filter();
		} catch(std::exception& e){
			m_outputQueue->setQueueEnd();
			throw e;
		}
	}

	virtual void filter() = 0;
};

