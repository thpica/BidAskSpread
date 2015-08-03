#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
using namespace std::chrono;
#include "Callable.h"
#include "DataStructures.h"
#include "Counter.h"

using namespace std;

template<typename T>
class OutputWriter: public Callable{
public:

	OutputWriter(string path, AtomicQueue<T>* in, MsgQueue* msgQueue):
		m_filepath(path),
		m_inputQueue(in),
		m_msgQueue(msgQueue)
	{}

	virtual ~OutputWriter(){ m_ofs.close(); }

	//no copy
	OutputWriter(const OutputWriter&) = delete;
	OutputWriter& operator=(const OutputWriter&) = delete;

private:
	const string m_filepath;
	AtomicQueue<T>* m_inputQueue;
	MsgQueue* m_msgQueue;
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
	Counter counter;

	counter.registerCallback([&](uint64_t countPerSec){
		string msg = to_string(countPerSec) + " line/s";
		if(m_inputQueue->empty()) msg += "\tstarving!";
		m_msgQueue->enqueue(Message(OUTPUT_WRITER, msg));
	});
	counter.start();

	try{
		while(true){
			DaySpread ds = m_inputQueue->dequeue();
			m_ofs << *ds.symbol << "," << *ds.date << "," << *ds.relSpread << endl;
			ds.deleteAll();
			counter.tick();
			this_thread::sleep_for(milliseconds(250));
		}
	} catch(AtomicQueue<DaySpread>::QueueEndException&){
		counter.stop();
		m_msgQueue->enqueue(Message(OUTPUT_WRITER, "Finished !"));
	}
}

template<>
void OutputWriter<Observation>::save(){
	Counter counter;

	counter.registerCallback([&](uint64_t countPerSec){
		string msg = to_string(countPerSec) + " line/s";
		if(m_inputQueue->empty()) msg += "\tstarving!";
		m_msgQueue->enqueue(Message(OUTPUT_WRITER, msg));
	});
	counter.start();

	try{
		while(true){
			Observation obs = m_inputQueue->dequeue();
			m_ofs << *obs.symbol << "," << *obs.date << "," << *obs.time << ","
				<< *obs.bid << "," << *obs.offer << "," << *obs.mode;
			if(obs.relSpread != nullptr)
				m_ofs << "," << *obs.relSpread;
			m_ofs << endl;
			obs.deleteAll();
			counter.tick();
			this_thread::sleep_for(milliseconds(250));
		}
	} catch(AtomicQueue<DaySpread>::QueueEndException&){
		counter.stop();
		m_msgQueue->enqueue(Message(OUTPUT_WRITER, "Finished !"));
	}
}