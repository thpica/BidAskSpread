#pragma once

#include <string>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

#include "MemoryMapped.h"
#include "AtomicQueue.h"
#include "DataStructures.h"
#include "Callable.h"

//DEBUG
#include <crtdbg.h>

using namespace std;

class QiParser: public Callable
{
public:
	/*struct Line: qi::grammar<MemoryMapped::const_iterator, Observation()>{
		Line();

		qi::rule<MemoryMapped::const_iterator, string()> time;
		qi::rule<MemoryMapped::const_iterator, Observation()> line;
	};*/

	QiParser(string filepath, AtomicQueue<Observation>* outQueue);
	
	QiParser(const QiParser& other) = delete; //no copy
	QiParser(QiParser&& other);

	QiParser& operator=(const QiParser&) = delete;
	QiParser& operator=(QiParser&& other);

private:
	MemoryMapped m_mapped;
	AtomicQueue<Observation> *m_queue;
	virtual void run();
	void parse();

	template<typename T>
	T* newByMove(T&& org);
};
