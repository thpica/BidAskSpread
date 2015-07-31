#pragma once

#include <Windows.h>
#include <atomic>
using std::atomic;
#include <iostream>
using std::cout;
using std::endl;
#include <exception>
using std::exception;
#include <string>
using std::string;

class Callable{
public:
	Callable(): m_shutdown(false){}
	virtual ~Callable(){};

	Callable(const Callable& other): m_shutdown(other.m_shutdown.load()){}

	Callable(const Callable&& other): m_shutdown(other.m_shutdown.load()){}

	void operator()(){
		try{
			run();
		} catch(exception& e){
			cout << "Exception raised in worker: " << name() << ": " << endl << e.what() << endl << endl;
		}
	}

	void shutdown(){ m_shutdown = true; }

protected:
	atomic<bool> m_shutdown;

	string name() const { return string(typeid(*this).name()); }
	virtual void run() = 0;
};