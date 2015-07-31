#pragma once

#include <exception>
#include <atomic>
#include <cstdint>
#include "readerwriterqueue.h"
using namespace moodycamel;

using namespace std;

template<typename T>
class AtomicQueue
{
public:
	AtomicQueue(uint64_t queue_size = 5000): //5K queue max size
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

	bool empty(){ return m_queue.peek() == nullptr;	}

	bool full(){ return m_full.load(); }

	bool end(){ return m_atEnd.load() && empty(); }

	T* peek(){ return m_queue.peek(); }

	T dequeue(){
		T elem;
		while(!m_queue.try_dequeue(elem)){
			if(end())
				throw QueueEndException();
			this_thread::yield();
		}
		return elem;
	}

	void enqueue(T&& elem){
		while(!m_queue.try_enqueue(elem)){
			m_full.store(true);
			this_thread::yield();
		}
		m_full.store(false);
	}

private:
	atomic<bool> m_atEnd;
	atomic<bool> m_full;
	ReaderWriterQueue<T> m_queue;
};