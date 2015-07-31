#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include "Callable.h"
#include "DataStructures.h"

using namespace std;

template<typename T>
class OutputWriter: public Callable{
public:

	OutputWriter(string path, AtomicQueue<T>* in):
		m_filepath(path),
		m_inputQueue(in)
	{}

	virtual ~OutputWriter(){ m_ofs.close(); }

	//no copy
	OutputWriter(const OutputWriter&) = delete;
	OutputWriter& operator=(const OutputWriter&) = delete;

private:
	const string m_filepath;
	AtomicQueue<T>* m_inputQueue;
	MsgQueue m_msgQueue;
	ofstream m_ofs;

	virtual void run(){
		m_ofs.open(m_filepath, ios::trunc);
		if(!m_ofs.is_open())
			throw ios::failure("Unable to open output file");
		save();
	}

	void save(){
		static_assert(false, "OutputWriter Requires an AtomicQueue of Observation or DaySpread");
	}

};

//template specializations
template<>
void OutputWriter<DaySpread>::save(){
	while(!m_inputQueue->end()){
		if(m_inputQueue->empty()){
			Sleep(25);
		} else{
			DaySpread ds = m_inputQueue->dequeue();
			m_ofs << *ds.symbol << "," << *ds.date << "," << *ds.relSpread << endl;
			ds.deleteAll();
		}
	}
}

template<>
void OutputWriter<Observation>::save(){
	while(!m_inputQueue->end()){
		if(m_inputQueue->empty()){
			Sleep(25);
		} else{
			Observation obs = m_inputQueue->dequeue();
			m_ofs << *obs.symbol << "," << *obs.date << "," << *obs.time << ","
				<< *obs.bid << "," << *obs.offer << "," << *obs.mode;
			if(obs.relSpread != nullptr)
				m_ofs << "," << *obs.relSpread;
			m_ofs << endl;
			obs.deleteAll();
		}
	}
}