#pragma once
#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::endl;

using std::move;
#include <assert.h>

#include "MemoryMapped.h"

class MemoryMappedHelper
{
public:
	MemoryMappedHelper(size_t viewsize = 268435456):
		m_viewSize(viewsize),
		m_mapOffset(0),
		m_readIndex(0)
	{
		assert(m_viewSize >= MemoryMapped::getpagesize());
	}
	
	MemoryMappedHelper(const MemoryMappedHelper&) = delete;
	MemoryMappedHelper& operator=(const MemoryMappedHelper&) = delete;

	MemoryMappedHelper(MemoryMappedHelper&& other):
		m_mapped(move(other.m_mapped)),
		m_viewSize(move(other.m_viewSize)),
		m_mapOffset(move(other.m_mapOffset)),
		m_readIndex(move(other.m_readIndex))
	{}

	MemoryMappedHelper& operator=(MemoryMappedHelper&& other){
		m_mapped = move(other.m_mapped);
		m_viewSize = move(other.m_viewSize);
		m_mapOffset = move(other.m_mapOffset);
		m_readIndex = move(other.m_readIndex);
		return *this;
	}

	~MemoryMappedHelper(){ m_mapped.close(); }

	void open(string filepath){
		m_mapped.open(filepath, m_viewSize, MemoryMapped::SequentialScan);
		if(!m_mapped.isValid())
			throw std::ios::failure("Unable to open input file");
	}

	const char* at(size_t pos, size_t guaranteedMappedLength = 1){
		if(!m_mapped.isValid())
			throw std::invalid_argument("No view mapped");
		if(pos > m_mapped.size())
			throw std::out_of_range("File is too small");
		if(pos < m_mapOffset || pos >= m_mapOffset + m_viewSize){
			m_mapOffset = pos - (pos % m_viewSize);
			m_mapped.remap(m_mapOffset, m_viewSize);
		}
		if(pos + guaranteedMappedLength - 1 > m_mapOffset + m_viewSize - 1){
			m_mapped.remap(m_mapOffset, m_viewSize + guaranteedMappedLength - 1); //view size can grow a little
		}
		return reinterpret_cast<const char*>(m_mapped.getData()) + (pos % m_viewSize);
	}

	void skip(size_t count = 1){ m_readIndex += count; }

	const char* get(){ return this->at(m_readIndex); }

	const char* next(){ return this->at(++m_readIndex); }

	const char* seek(size_t pos){
			m_readIndex = pos;
			return get();
	}

	size_t getIndex() const{
		return m_readIndex;
	}

	bool isValid(){ return m_mapped.isValid(); }

private:
	MemoryMapped m_mapped;
	size_t m_viewSize;
	size_t m_mapOffset;
	size_t m_readIndex;
};