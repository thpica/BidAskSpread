#pragma once

#include <istream>
#include <string>
#include <cstdint>
#include <cstring>
#include <chrono>
using namespace std::chrono;

#include "MemoryMappedStreamBuf.h"
#include "DataStructures.h"
#include "Callable.h"
#include "Counter.h"

using namespace std;

class StreamParser: public Callable
{
public:

	StreamParser(string filepath, ObsQueue* out, MsgQueue* msg);

private:
	MemoryMappedStreamBuf m_streambuf;
	ObsQueue* m_queue;
	MsgQueue* m_msgQueue;
	const string m_filepath;
	virtual void run();
	void parse();
	unsigned char* parseUChar(const char* ptr) const;
	float* parseFloat(const char* ptr) const;
};
