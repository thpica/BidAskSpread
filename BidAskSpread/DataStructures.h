#pragma once

#include <string>
using std::string;

#include "AtomicQueue.h"
#include "concurrentqueue.h"
using namespace moodycamel;

struct Observation{
	Observation(const string* _symbol = nullptr,const string* _date = nullptr,
				const string* _time = nullptr,	const float* _offer = nullptr,
				const float* _bid = nullptr, const unsigned char* _mode = nullptr,
				double* _relspread = nullptr):
		symbol(_symbol), date(_date), time(_time),
		offer(_offer), bid(_bid), mode(_mode), relSpread(_relspread)
	{}

	void deleteAll(){
		delete symbol;
		symbol = nullptr;
		delete date;
		date = nullptr;
		delete time;
		time = nullptr;
		delete offer;
		offer = nullptr;
		delete bid;
		bid = nullptr;
		delete mode;
		mode = nullptr;
		delete relSpread;
	}

	const string* symbol;
	const string* date;
	const string* time;
	const float* offer;
	const float* bid;
	const unsigned char* mode;
	double* relSpread;
};

struct DaySpread{
	DaySpread():
	symbol(nullptr), date(nullptr), relSpread(nullptr)
	{}

	DaySpread(const string* smb, const string* dt, const double* sprd):
	symbol(smb),
	date(dt),
	relSpread(sprd)
	{}

	void deleteAll(){
		delete symbol; delete date; delete relSpread;
		symbol = nullptr; date = nullptr; relSpread = nullptr;
	}

	const string* symbol;
	const string* date;
	const double* relSpread;
};

enum Sender{
	UNDEFINED,
	PARSER,
	TIMESTAMP_MERGE_FILTER,
	OUTLIER_FILTER,
	RELSPREAD_PROCESSOR,
	DAY_RELSPREAD_PROCESSOR,
	OUTPUT_WRITER
};
static const char* senderString[] = {
	"Undefined",
	"Parser",
	"Timestamp Merge Filter",
	"Outliers Filter",
	"RelSpread Processor",
	"Day RelSpread Processor",
	"Output writer"
};

struct Message{
	Message(): sender(UNDEFINED){}

	Message(Sender _sender, string _text):
	sender(_sender), text(_text){}
	
	Message(Message&& other):
	sender(std::move(other.sender)), text(std::move(other.text)){}
	
	Message& operator=(Message&& other){
		sender = std::move(other.sender);
		text = std::move(other.text);
		return *this;
	}

	Sender sender;
	string text;
};

typedef AtomicQueue<Observation> ObsQueue;
typedef ConcurrentQueue<Message> MsgQueue;
typedef AtomicQueue<DaySpread> SprdQueue;