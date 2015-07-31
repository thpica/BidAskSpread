#pragma once

#include <string>
#include <iostream>
#include <map>
#include <thread>
#include "Callable.h"
#include "DataStructures.h"
#include "ClearScreen.h"

using namespace std;

class MsgOutput: public Callable{
public:
	MsgOutput(MsgQueue* msg): m_msgQueue(msg)
	{
		printTable();
	}

private:
	MsgQueue* m_msgQueue;
	map<Sender, string> m_table;

	void run(){
		while(!m_shutdown){
			Message msg;
			if(m_msgQueue->try_dequeue(msg)){
				m_table[msg.sender] = msg.text;
				printTable();
			} else
				this_thread::yield();
		}
	}

	void printTable(){
		ClearScreen();
		cout << "Bid-Ask Spread Processor" << endl
			 << "--------------------------------------------------" << endl << endl;
		for(auto const &line : m_table){
			cout << senderString[line.first] << ": " << line.second << endl << endl;
		}
	}
};

