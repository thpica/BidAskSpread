#pragma once

#include <string>
#include <cstdint>
#include <chrono>
using namespace std::chrono;

#include "MemoryMappedHelper.h"
#include "DataStructures.h"
#include "Callable.h"
#include "Counter.h"

using namespace std;

class Parser: public Callable
{
public:

	Parser(string filepath, ObsQueue* out, MsgQueue* msg);
	
	Parser(const Parser& other) = delete; //no copy
	Parser(Parser&& other);

	Parser& operator=(const Parser&) = delete;
	Parser& operator=(Parser&& other);

private:
	struct Cell{
		Cell():index(0), length(0){}
		void clear(){ index = 0; length = 0; }
		size_t index;
		size_t length;
	};

	struct Line{
		Cell symbol;
		Cell date;
		Cell time;
		Cell bid;
		Cell offer;
		Cell mode;
		bool isValid(){
			return symbol.length >= 3 && date.length == 8 &&
				time.length >= 7 && bid.length > 0 &&
				offer.length > 0 && mode.length > 0;
		}
		void clear(){
			symbol.clear(); date.clear(); time.clear(); bid.clear(); offer.clear(); mode.clear();
		}
	};

	MemoryMappedHelper m_mappedHelper;
	ObsQueue* m_queue;
	MsgQueue* m_msgQueue;
	const string m_filepath;
	virtual void run();
	void parse();
};
