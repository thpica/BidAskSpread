#pragma once

#include <cstddef>
#include <iterator>

#include "MemoryMapped.h"

class MemoryMappedIterator
{
public:
	typedef typename MemoryMappedIterator self_type;
	typedef typename long long int difference_type;
	typedef typename const char* value_type;
	typedef typename const char*& const_reference;
	typedef typename const char** const_pointer;
	typedef std::random_access_iterator_tag iterator_category;

	MemoryMappedIterator(MemoryMapped *mapped, size_t index = 0):
		m_mapped(mapped),
		m_index(index)
	{}

	bool operator==(const self_type& other) const{ return m_index == other.m_index; }

	bool operator!=(const self_type& other) const{ return m_index != other.m_index; }

	self_type& operator++(){
		if(m_index + 1 > m_mapped->size())
			throw std::out_of_range("End of File reached");
		m_index++; 
		return *this;
	}
	self_type operator++(int){ self_type copy = *this; ++(*this); return copy; }

	self_type& operator--(){
		if(m_index - 1 > 0)
			throw std::out_of_range("Begin of File reached");
		m_index--;
		return *this;
	}

	self_type operator--(int){ self_type copy = *this; --(*this); return copy; }

	self_type& operator+=(size_t offset){
		if(m_index + offset > m_mapped->size())
			throw std::out_of_range("Increment too large, EOF");
		m_index += offset;
		return *this;
	}

	self_type operator+(size_t offset) const{ self_type copy = *this; copy += offset; return copy; }

	//friend self_type operator+(size_t, const self_type&); //optional

	self_type& operator-=(size_t offset){
		if(m_index + offset < 0)
			throw std::out_of_range("Decrement too large, BOF");
		m_index -= offset;
		return *this;
	}

	self_type operator-(size_t offset) const{ self_type copy = *this; copy += offset; return copy; }

	difference_type operator-(self_type other) const{ return m_index - other.m_index; }

	value_type operator*(){
		if(!m_mapped->isValid())
			throw std::invalid_argument("No file mapped");
		if(m_index < m_mapOffset || m_index > m_mapOffset + m_mapped->mappedSize()){
			m_mapOffset = m_index - (m_index % m_mapped->mappedSize());
			m_mapped->remap(m_mapOffset, m_mapped->mappedSize());
		}
		return (const char*)(m_mapped->getData() + (m_index % m_mapped->mappedSize()));
	}
	//const_pointer operator->() const{}
	//const_reference operator[](size_t) const; //optional

private:
	MemoryMapped *m_mapped;
	size_t m_mapOffset;
	size_t m_index;
};

