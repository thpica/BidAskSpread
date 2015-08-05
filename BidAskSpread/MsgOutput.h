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
	MsgOutput(const string filepath, MsgQueue* msg):
		m_msgQueue(msg), m_filepath(filepath)
	{
		printTable();
	}

private:
	const string m_filepath;
	MsgQueue* m_msgQueue;
	map<Sender, string> m_table;

	void run(){
		while(true){
			Message msg;
			if(m_msgQueue->try_dequeue(msg)){
				m_table[msg.sender] = msg.text;
				printTable();
			} else if(!m_shutdown.load()){
				this_thread::sleep_for(milliseconds(250));
			} else{
				this_thread::sleep_for(milliseconds(250));
				while(m_msgQueue->try_dequeue(msg)){
					m_table[msg.sender] = msg.text;
					printTable();
				}
				break;
			}
		}
	}

	void printTable(){
		ClearScreen();
		cout << "Bid-Ask Spread Processor" << endl
			 << "Processing file: " << m_filepath << endl
			 << "--------------------------------------------------" << endl << endl;
		for(auto const &line : m_table){
			cout << senderString[line.first] << ": " << line.second << endl << endl;
		}
	}
};