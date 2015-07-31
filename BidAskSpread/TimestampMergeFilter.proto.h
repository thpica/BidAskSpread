#pragma once

#include <vector>
#include <forward_list>
#include <future>
#include <atomic>
#include <thread>

#include "Filter.h"
#include "StatsUtils.h"
#include "semaphore.h"
#include "concurrentqueue.h"
using moodycamel::ConcurrentQueue;

using namespace std;

class TimestampMergeFilter: public Filter{
public:
	TimestampMergeFilter(ObsQueue* in, ObsQueue* out, MsgQueue* msg);

	//no copy
	TimestampMergeFilter(const TimestampMergeFilter&) = delete;
	TimestampMergeFilter& operator=(const TimestampMergeFilter&) = delete;

private:
	class Job{
	public:
		Job(){}
		Job(forward_list<Observation> buffer): m_buffer(buffer), m_promise(new promise<Observation>()){}
		Job(Job&) = delete;
		Job& operator=(Job&) = delete;
		Job(Job&& other): m_promise(move(other.m_promise)), m_buffer(move(other.m_buffer)){}
		Job& operator=(Job&& other){ 
			m_promise = move(other.m_promise);
			m_buffer = move(other.m_buffer);
			return *this;
		}
		~Job(){
			try{ delete m_promise; } catch(future_error&){}
		}

		future<Observation> getFuture(){ return m_promise->get_future(); }
		forward_list<Observation> getBuffer(){ return m_buffer; }
		void setResult(Observation&& obs){ 
			m_promise->set_value(forward<Observation&&>(obs)); 
		}
	private:
		promise<Observation> *m_promise;
		forward_list<Observation> m_buffer;
	};

	class Worker{
	public:
		Worker(ConcurrentQueue<Job>* jobsQueue, atomic<bool>* jobsQueueEnd);
		Worker(Worker&) = delete;
		Worker(Worker&& other);
		~Worker();

		void start();
		void stop();
		void waitUntilFinished();
	private:
		atomic<bool> m_running;
		thread* m_thread;
		atomic<bool>* m_jobsQueueEnd;
		ConcurrentQueue<Job>* m_jobsQueue;

		static void run(Job& job);
		static Observation getMergedObs(forward_list<Observation> buffer);
	};

	atomic<bool> m_jobsQueueEnd;
	ConcurrentQueue<Job> m_jobsQueue;
	AtomicQueue<shared_future<Observation>> m_futureQueue;
	virtual void filter();
};