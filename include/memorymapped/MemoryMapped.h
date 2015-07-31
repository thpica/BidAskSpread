// //////////////////////////////////////////////////////////
// MemoryMapped.h
// Copyright (c) 2013 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//

#pragma once

// define fixed size integer types
#ifdef _MSC_VER
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

#include <string>
#include <iterator>

//#include "MemoryMappedIterator.h"

/// Portable read-only memory mapping (Windows and Linux)
/** Filesize limited by size_t, usually 2^32 or 2^64 */
class MemoryMapped
{
public:
  /// tweak performance
  enum CacheHint
  {
    Normal,         ///< good overall performance
    SequentialScan, ///< read file only once with few seeks
    RandomAccess    ///< jump around
  };

  /// how much should be mappend
  enum MapRange
  {
    WholeFile = 0   ///< everything ... be careful when file is larger than memory
  };

  /// iterator
  /*class MemoryMappedIterator:
	  public std::iterator<std::input_iterator_tag, const char*>
  {
  public:
	  //typedef MemoryMappedIterator self_type;
	  typedef long long int difference_type;
	  typedef const char* value_type;
	  typedef const char*& reference;
	  typedef const char** pointer;
	  typedef std::input_iterator_tag iterator_category;

	  MemoryMappedIterator(MemoryMapped *mapped, size_t index = 0):
		  m_mapped(mapped),
		  m_index(index)
	  {}

	  bool operator==(const MemoryMappedIterator& other) const{ return m_index == other.m_index; }

	  bool operator!=(const MemoryMappedIterator& other) const{ return m_index != other.m_index; }

	  MemoryMappedIterator& operator++(){
		  if(m_index + 1 > m_mapped->size())
			  throw std::out_of_range("End of File reached");
		  m_index++;
		  return *this;
	  }
	  MemoryMappedIterator operator++(int){ MemoryMappedIterator copy = *this; ++(*this); return copy; }

	  MemoryMappedIterator& operator--(){
		  if(m_index - 1 > 0)
			  throw std::out_of_range("Begin of File reached");
		  m_index--;
		  return *this;
	  }

	  MemoryMappedIterator operator--(int){ MemoryMappedIterator copy = *this; --(*this); return copy; }

	  MemoryMappedIterator& operator+=(size_t offset){
		  if(m_index + offset > m_mapped->size())
			  throw std::out_of_range("Increment too large, EOF");
		  m_index += offset;
		  return *this;
	  }

	  MemoryMappedIterator operator+(size_t offset) const{ MemoryMappedIterator copy = *this; copy += offset; return copy; }

	  //friend self_type operator+(size_t, const self_type&); //optional

	  MemoryMappedIterator& operator-=(size_t offset){
		  if(m_index + offset < 0)
			  throw std::out_of_range("Decrement too large, BOF");
		  m_index -= offset;
		  return *this;
	  }

	  MemoryMappedIterator operator-(size_t offset) const{ MemoryMappedIterator copy = *this; copy += offset; return copy; }

	  difference_type operator-(MemoryMappedIterator other) const{ return m_index - other.m_index; }

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

  /// iterator typdef
	typedef MemoryMappedIterator const_iterator;*/

  /// do nothing, must use open()
  MemoryMapped();
  /// open file, mappedBytes = 0 maps the whole file
  MemoryMapped(const std::string& filename, size_t mappedBytes = WholeFile, CacheHint hint = Normal);
  /// move constructor
  MemoryMapped(MemoryMapped&& other);
  /// close file (see close() )
  ~MemoryMapped();

  /// move assignement
  MemoryMapped& operator=(MemoryMapped&& other);

  /// open file, mappedBytes = 0 maps the whole file
  bool open(const std::string& filename, size_t mappedBytes = WholeFile, CacheHint hint = Normal);
  /// close file
  void close();

  /// access position, no range checking (faster)
  const unsigned char operator[](size_t offset) const;
  /// access position, including range checking
  const unsigned char at        (size_t offset) const;

  /// raw access
  const unsigned char* getData() const;

  /// true, if file successfully opened
  bool isValid() const;

  /// get file size
  uint64_t size() const;
  /// get number of actually mapped bytes
  size_t mappedSize() const;

  /// replace mapping by a new one of the same file, offset MUST be a multiple of the page size
  bool remap(uint64_t offset, size_t mappedBytes);

  /*const_iterator begin() { return const_iterator(this); }
  const_iterator end(){ return const_iterator(this, size()); }*/

  /// get OS page size (for remap)
  static int getpagesize();

private:
  /// don't copy object
  MemoryMapped(const MemoryMapped&);
  /// don't copy object
  MemoryMapped& operator=(const MemoryMapped&);

  /// file name
  std::string _filename;
  /// file size
  uint64_t    _filesize;
  /// caching strategy
  CacheHint   _hint;
  /// mapped size
  size_t      _mappedBytes;

  /// define handle
#ifdef _MSC_VER
  typedef void* FileHandle;
  /// Windows handle to memory mapping of _file
  void*       _mappedFile;
#else
  typedef int   FileHandle;
#endif

  /// file handle
  FileHandle  _file;
  /// pointer to the file contents mapped into memory
  void*       _mappedView;
};