#include <streambuf>
#include "MemoryMapped.h"

using namespace std;

class MemoryMappedStreamBuf: public streambuf{
public:
	MemoryMappedStreamBuf(string filename, size_t viewsize = 268435456):
		m_viewSize(viewsize),
		m_viewOffset(0)
	{
		m_memoryMapped.open(filename, m_viewSize, MemoryMapped::SequentialScan);
		setg(
			(char*)m_memoryMapped.getData(),
			(char*)m_memoryMapped.getData(),
			(char*)m_memoryMapped.getData() + m_viewSize);
	}

	~MemoryMappedStreamBuf(){
		m_memoryMapped.close();
	}

private:
	size_t m_viewSize;
	size_t m_viewOffset;
	MemoryMapped m_memoryMapped;
	
	virtual int underflow(){
		m_viewOffset += m_viewSize;
		if(m_memoryMapped.remap(m_viewOffset, m_viewSize) == false)
			return EOF;
		setg(
			(char*)m_memoryMapped.getData(),
			(char*)m_memoryMapped.getData(),
			(char*)m_memoryMapped.getData() + m_memoryMapped.mappedSize());
		return *gptr();
	}

	virtual streampos seekoff(streamoff offset, ios::seekdir way, ios::openmode mode = ios::in){
		streampos pos;

		switch(way){
		case ios::beg:
			pos = offset;
			break;
		case ios::cur:
			pos = (m_viewOffset + (eback() - gptr())) + offset;
			break;
		case ios::end:
			pos = m_memoryMapped.size() - offset;
			break;
		}
		return seekpos(pos);
	}

	virtual streampos seekpos(streampos pos, ios::openmode mode = ios::in){
		if((int)pos - (pos % m_viewSize) != m_viewOffset){
			m_viewOffset = (int)pos - (pos % m_viewSize);
			if(m_memoryMapped.remap(m_viewOffset, m_viewSize) == false)
				return EOF;
		}
		setg(
			(char*)m_memoryMapped.getData(),
			(char*)m_memoryMapped.getData() + (pos % m_viewSize),
			(char*)m_memoryMapped.getData() + m_memoryMapped.mappedSize());
		return pos;
	}
};