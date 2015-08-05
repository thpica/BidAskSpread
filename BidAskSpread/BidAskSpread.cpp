#include <iostream>
#include <Windows.h>
#include <iostream>
#include <string>
#include <numeric>
#include <vector>
#include <thread>
#include <cstring>

#include "AtomicQueue.h"
#include "concurrentqueue.h"
#include "DataStructures.h"
#include "TimestampMergeFilter.h"
#include "StreamParser.h"
#include "RelSpreadProcessor.h"
#include "DayRelSpreadProcessor.h"
#include "OutliersFilter.h"
#include "OutputWriter.h"
#include "MsgOutput.h"
//DEBUG


#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
	#ifndef DBG_NEW
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
		#define new DBG_NEW
	#endif // DBG_NEW
	#include <stdlib.h>
	#include <crtdbg.h>
#endif  // _DEBUG

using namespace std;

void processFile(char* filename){
	ConcurrentQueue<Message>* msgQueue = new ConcurrentQueue<Message>();
	AtomicQueue<Observation>* parsedQueue = new AtomicQueue<Observation>(30000);
	AtomicQueue<Observation>* timestampMergedQueue = new AtomicQueue<Observation>(30000);
	AtomicQueue<Observation>* outliersFilteredQueue = new AtomicQueue<Observation>(30000);
	AtomicQueue<Observation>* relSpreadQueue = new AtomicQueue<Observation>(30000);
	AtomicQueue<DaySpread>* dayRelSpreadQueue = new AtomicQueue<DaySpread>(30000);
	vector<thread> threads;

	MsgOutput* msgOutput = nullptr;

	thread msgOutputThread([&](){
		msgOutput = new MsgOutput(filename, msgQueue);
		msgOutput->operator()();
	});

	threads.push_back(thread([&](){
		StreamParser(filename, parsedQueue, msgQueue)();
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
		OutputWriter<DaySpread>(string(filename) + ".processed.csv", dayRelSpreadQueue, msgQueue)();
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
}

void pauseIfDebug(){
#ifdef _DEBUG
	cout << "Press any key to exit" << endl;
	getchar();
#endif //_DEBUG
}

int main(int argc, char* argv[]){
	const char helpMsg[] = "Usage: \n"\
		"BidAskSpread input_filename_1 [input_filename_2] [...]\n"\
		"Wildcards (*.csv or ?.csv) and will exclude *.processed.csv";
	if(argc < 2){
		cout << helpMsg << endl;
		pauseIfDebug();
		return 1;
	} else if(argc == 2 && (strcmp(argv[1], "/?") == 0 || strcmp(argv[1], "/help") == 0 ||
			  strcmp(argv[1], "help") == 0 || strcmp(argv[1], "-h") == 0)){
		cout << helpMsg << endl;
		pauseIfDebug();
		return 1;
	}

	for(int i = 1; i < argc; ++i){
		if(strstr(argv[i], "processed.csv") == nullptr)
			processFile(argv[i]);
	}

	pauseIfDebug();
	return 0;
}