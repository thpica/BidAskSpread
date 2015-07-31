#include "Parser.h"

Parser::Parser(string filepath, ObsQueue* out, MsgQueue* msg):
	m_filepath(filepath),
	m_mappedHelper(),
	m_queue(out),
	m_msgQueue(msg)
{}

Parser::Parser(Parser&& other):
	m_mappedHelper(move(other.m_mappedHelper)),
	m_queue(move(other.m_queue))
{}

Parser& Parser::operator=(Parser&& other){
	m_mappedHelper = move(other.m_mappedHelper);
	m_queue = move(other.m_queue);
	return *this;
}

void Parser::run(){
	try{
		m_mappedHelper.open(m_filepath);
		m_counter.start();
		parse();
	} catch(exception& e){
		m_queue->setQueueEnd();
		throw e;
	}
}

void Parser::parse(){
	Line line;
	uint64_t lineNumber = 1;

	m_counter.registerCallback([&](uint64_t countPerSec){
		string msg = "Parsing at " + to_string(countPerSec) + " line/s";
		if(m_queue->full()) msg += "\tblocked!";
		m_msgQueue->enqueue(Message(PARSER, msg));
	});

	try{
		if(*m_mappedHelper.seek(30) == '\n') //skip first line (headers)
			m_mappedHelper.skip();
		
		while(true){
			//SYMBOL
			line.symbol.index = m_mappedHelper.getIndex();
			while(*m_mappedHelper.next() != ',')
				line.symbol.length++;
			line.symbol.length++;
			m_mappedHelper.skip(); //skip separator

			//DATE
			line.date.index = m_mappedHelper.getIndex();
			while(*m_mappedHelper.next() != ',')
				line.date.length++;
			line.date.length++;
			m_mappedHelper.skip(); //skip separator

			//TIME
			line.time.index = m_mappedHelper.getIndex();
			while(*m_mappedHelper.next() != ',')
				line.time.length++;
			line.time.length++;
			m_mappedHelper.skip(); //skip separator

			//BID
			line.bid.index = m_mappedHelper.getIndex();
			while(*m_mappedHelper.next() != ',')
				line.bid.length++;
			line.bid.length++;
			m_mappedHelper.skip(); //skip separator

			//OFR
			line.offer.index = m_mappedHelper.getIndex();
			while(*m_mappedHelper.next() != ',')
				line.offer.length++;
			line.offer.length++;
			m_mappedHelper.skip(); //skip separator

			//MODE
			line.mode.index = m_mappedHelper.getIndex();
			while(*m_mappedHelper.get() != '\r' && *m_mappedHelper.get() != '\n') { // line return
				m_mappedHelper.skip();
				line.mode.length++;
			}
			line.mode.length++;
			if(*m_mappedHelper.get() == '\r')
				m_mappedHelper.skip();
			m_mappedHelper.skip(); //skip new line

			if(line.isValid()){
				const float* bid = new float(atof(m_mappedHelper.at(line.bid.index, line.bid.length + 1)));
				const float* offer = new float(atof(m_mappedHelper.at(line.offer.index, line.offer.length + 1)));
				const unsigned short* mode = new const unsigned short(atoi(m_mappedHelper.at(line.mode.index, line.mode.length + 1)));
				if(*bid > 0.01 && *offer > 0.01 && *mode == 12){
					const string* symbol = new string(m_mappedHelper.at(line.symbol.index, line.symbol.length), line.symbol.length);
					const string* date = new string(m_mappedHelper.at(line.date.index, line.date.length), line.date.length);
					const string* time = new string(m_mappedHelper.at(line.time.index, line.time.length), line.time.length);
					if(!symbol->empty() && !date->empty() && !time->empty()){
						Observation obs(
							symbol,
							date,
							time,
							offer,
							bid,
							mode
							);
						m_queue->enqueue(move(obs));
					} else{
						delete symbol; delete date; delete time;
						symbol = nullptr; date = nullptr; time = nullptr;
						throw std::exception(("Parse error at line : " + to_string(lineNumber)).c_str());
					}
				} else{
					delete offer;
					delete bid;
					delete mode;
				}
			} else{
				throw std::exception(("Parse error at line : " + to_string(lineNumber)).c_str());
			}
			line.clear();
			lineNumber++;
			m_counter.tick();
		}
	} catch(std::out_of_range&){
		m_queue->setQueueEnd();
		m_msgQueue->enqueue(Message(PARSER, "Parser finished !"));
	}
}