#pragma once

#include <exception>
#include <atomic>
#include <cstdint>
#include <chrono>
using namespace std::chrono;
#include "readerwriterqueue.h"
using namespace moodycamel;

using namespace std;

template<typename T>
class AtomicQueue
{
public:
	AtomicQueue(uint64_t queue_size = 25000): //25K queue max size
		m_atEnd(false),
		m_full(false),
		m_queue(queue_size){}

	class QueueException: public exception{
	public:
		virtual const char* what() const throw(){ return "Undefined Queue Exception."; }
	};

	class QueueEndException: public QueueException{
	public:
		virtual const char* what() const throw(){ return "Queue is at end."; }
	};

	class QueueEmptyException: public QueueException{
	public:
		virtual const char* what() const throw(){ return "Queue is empty."; }
	};

	class QueueFullException: public QueueException{
	public:
		virtual const char* what() const throw(){ return "Queue is full."; }
	};
	
	void setQueueEnd(){ m_atEnd.store(true); }

	bool empty(){ return m_empty.load(); }

	bool full(){ return m_full.load(); }

	bool end(){ return m_atEnd.load() && empty(); }

	T* peek(){
		T* ptr = nullptr;
		while((ptr = m_queue.peek()) == nullptr){
			if(end())
				throw QueueEndException();
			m_empty = true;
			this_thread::yield();
		}
		m_empty = false;
		return ptr;
	}

	T dequeue(){
		T elem;
		while(!m_queue.try_dequeue(elem)){
			if(end())
				throw QueueEndException();
			m_empty = true;
			this_thread::sleep_for(milliseconds(25));
		}
		m_empty = false;
		return elem;
	}

	void enqueue(T&& elem){
		while(!m_queue.try_enqueue(elem)){
			m_full = true;
			this_thread::sleep_for(milliseconds(25));
		}
		m_full = false;
	}

private:
	atomic<bool> m_atEnd;
	atomic<bool> m_full;
	atomic<bool> m_empty;
	ReaderWriterQueue<T> m_queue;
};