#include <iostream>
#include <Windows.h>
#include <iostream>
#include <string>
#include <numeric>
#include <vector>
#include <thread>

#include "AtomicQueue.h"
#include "concurrentqueue.h"
#include "DataStructures.h"
#include "TimestampMergeFilter.h"
#include "Parser.h"
#include "RelSpreadProcessor.h"
#include "DayRelSpreadProcessor.h"
#include "OutliersFilter.h"
#include "OutputWriter.h"
#include "MsgOutput.h"
//DEBUG

#define _CRTDBG_MAP_ALLOC
#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif
	#include <stdlib.h>
	#include <crtdbg.h>
#endif  // _DEBUG

using namespace std;

int main(int argc, char* argv[]){
	if(argc != 2){
		cout << "Usage: BidAskSpread [input_filename]" << endl;
		return 1;
	}

	ConcurrentQueue<Message>* msgQueue = new ConcurrentQueue<Message>();

	AtomicQueue<Observation>* parsedQueue = new AtomicQueue<Observation>();
	AtomicQueue<Observation>* timestampMergedQueue = new AtomicQueue<Observation>();
	AtomicQueue<Observation>* outliersFilteredQueue = new AtomicQueue<Observation>();
	AtomicQueue<Observation>* relSpreadQueue = new AtomicQueue<Observation>();
	AtomicQueue<DaySpread>* dayRelSpreadQueue = new AtomicQueue<DaySpread>();

	vector<thread> threads;

	MsgOutput* msgOutput = nullptr;

	thread msgOutputThread([&](){
		msgOutput = new MsgOutput(argv[1], msgQueue);
		msgOutput->operator()();
	});

	threads.push_back(thread([&](){
		Parser(string(argv[1]), parsedQueue, msgQueue)();
	}));

	threads.push_back(thread([&](){
		TimestampMergeFilter(parsedQueue, timestampMergedQueue, msgQueue)();
	}));

	threads.push_back(thread([&](){
		OutliersFilter(timestampMergedQueue, outliersFilteredQueue, msgQueue)();
	}));

	threads.push_back(thread([&](){
		RelSpreadProcessor(outliersFilteredQueue, relSpreadQueue, msgQueue)();
	}));

	threads.push_back(thread([&](){
		DayRelSpreadProcessor(relSpreadQueue, dayRelSpreadQueue, msgQueue)();
	}));

	threads.push_back(thread([&](){
		OutputWriter<DaySpread>(string(argv[1]) + ".processed.csv", dayRelSpreadQueue, msgQueue)();
	}));

	for_each(threads.begin(), threads.end(), [](thread& t){
		if(t.joinable())
			t.join();
	});

	msgOutput->shutdown();
	msgOutputThread.join();

	delete parsedQueue;	delete timestampMergedQueue; 
	delete outliersFilteredQueue; delete relSpreadQueue;
	delete dayRelSpreadQueue; delete msgQueue;

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
	cout << "Press any key to exit" << endl;
	getchar();
#endif //_DEBUG
	return 0;
}