#include "TimestampMergeFilter.h"

TimestampMergeFilter::TimestampMergeFilter(ObsQueue* in, ObsQueue* out, MsgQueue* msg):
Filter(in, out, msg),
m_futureQueue()
{}

void TimestampMergeFilter::filter(){
	vector<Worker> workerList;
	forward_list<Observation> buffer;
	//start output thread
	thread ouputThread([&](){
		try{
			while(true){
				shared_future<Observation> f = m_futureQueue.dequeue();
				Observation obs = f.get();
				m_outputQueue->enqueue(obs);
			}
		} catch(ObsQueue::QueueEndException&){
			m_outputQueue->setQueueEnd();
		} catch(exception& e){
			cout << "Exception raised in TimestampMergeFilter: " << e.what() << endl;
		}
	});

	//instantiate some workers
	for(int i = 0; i < 2; i++){
		Worker worker(&m_jobsQueue, &m_jobsQueueEnd);
		workerList.push_back(move(worker));
	}

	try{
		for(int i = 0; i < workerList.size(); i++)
			workerList.at(i).start();

		while(true){
			Observation obs = m_inputQueue->dequeue();
			if(buffer.empty()){
				buffer.push_front(obs);
			} else{
				if(*obs.date == *buffer.front().date && *obs.time == *buffer.front().time){
					buffer.push_front(obs);
				} else{
					Job job(move(buffer));
					m_futureQueue.enqueue(job.getFuture());
					m_jobsQueue.enqueue(move(job));
					buffer.clear();
					buffer.push_front(obs);
				}
			}
		}
	} catch(ObsQueue::QueueEndException&){
		if(!buffer.empty()){
			Job job(move(buffer));
			m_futureQueue.enqueue(job.getFuture());
			m_futureQueue.setQueueEnd();
			m_jobsQueueEnd.store(true);
			buffer.clear();
			for(int i = 0; i < workerList.size(); i++)
				workerList.at(i).waitUntilFinished();
		}
	}
}

TimestampMergeFilter::Worker::Worker(ConcurrentQueue<Job>* jobsQueue, atomic<bool>* jobsQueueEnd):
m_running(false),
m_jobsQueueEnd(jobsQueueEnd),
m_jobsQueue(jobsQueue),
m_thread(nullptr)
{}

TimestampMergeFilter::Worker::Worker(Worker&& other):
m_running(other.m_running.load()),
m_jobsQueueEnd(other.m_jobsQueueEnd),
m_jobsQueue(other.m_jobsQueue),
m_thread(other.m_thread)
{}

TimestampMergeFilter::Worker::~Worker(){
	stop();
	waitUntilFinished();
	delete m_thread;
}

void TimestampMergeFilter::Worker::start(){
	m_running = true;
	m_thread = new thread([&](){
		try{
			while(m_running.load()){
				Job job;
				if(m_jobsQueue->try_dequeue(job))
					run(job);
				else{
					if(m_jobsQueueEnd->load())
						break;
					Sleep(25);
				}
			}
		} catch(exception& e){
			_CrtDbgBreak();
		}
	});
}

void TimestampMergeFilter::Worker::stop(){
	m_running.store(false);
}

void TimestampMergeFilter::Worker::waitUntilFinished(){
	if(m_thread != nullptr){
		if(m_thread->joinable())
			m_thread->join();
	}
}

void TimestampMergeFilter::Worker::run(Job& job){
	forward_list<Observation> buff = job.getBuffer();	
	Observation res = getMergedObs(buff);
	job.setResult(move(res));
}

Observation TimestampMergeFilter::Worker::getMergedObs(forward_list<Observation> buffer){
	Observation result;

	vector<float> sortedBids;
	vector<float> sortedOffers;

	if(distance(buffer.begin(), buffer.end()) > 1){
		for(forward_list<Observation>::iterator it = buffer.begin(); it != buffer.end(); it++){
			sortedBids.push_back(*it->bid);
			sortedOffers.push_back(*it->offer);
		}

		//sort buffers
		sort(sortedBids.begin(), sortedBids.end());
		sort(sortedOffers.begin(), sortedOffers.end());

		float* medOffer = new float(StatsUtils::median(sortedOffers));
		float* medBid = new float(StatsUtils::median(sortedBids));

		result = Observation(
			buffer.front().symbol,
			buffer.front().date,
			buffer.front().time,
			medOffer,
			medBid,
			buffer.front().mode
			);

		//cleanup
		delete buffer.front().bid;
		delete buffer.front().offer;
		for(forward_list<Observation>::iterator it = ++buffer.begin(); it != buffer.end(); it++)
			it->deleteAll();
	} else{
		result = buffer.front();
	}
	return result;
}