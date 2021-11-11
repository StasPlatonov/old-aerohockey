#ifndef TYPES_H
#define TYPES_H

#include <deque>
#include <vector>

typedef unsigned short uint16_t;

typedef std::vector<unsigned char> Bytes;

// size should be a power of two
template<int bufferSize, class DataType = unsigned char>
class RingBuffer
{
public:
  typedef uint16_t IndexType;

  RingBuffer()
  {
    m_Mask = bufferSize - 1;
    assert(((bufferSize & m_Mask) == 0) || "Size of circular buffer must be a power of two (performance reason)!");
  }

  inline bool Write(DataType value)
  {
    if (IsFull())
      return false;
    m_Data[m_WriteCount++ & m_Mask] = value;
    return true;
  }

  inline bool Read(DataType &value)
  {
    if (IsEmpty())
      return false;
    value = m_Data[m_ReadCount++ & m_Mask];
    return true;
  }

  inline DataType First() const { return operator[](0); }

  inline DataType Last()const { return operator[](Count()); }

  inline DataType& operator[] (IndexType i)
  {
    if (IsEmpty() || i > Count())
      return DataType();
    return m_Data[(m_ReadCount + i) & m_Mask];
  }

  inline const DataType operator[] (IndexType i)const
  {
    if (IsEmpty())
      return DataType();
    return m_Data[(m_ReadCount + i) & m_Mask];
  }

  inline bool IsEmpty() const { return m_WriteCount == m_ReadCount; }
  inline bool IsFull() const { return ((IndexType)(m_WriteCount - m_ReadCount) & (IndexType)~(m_Mask)) != 0; }
  inline IndexType Count() const { return (m_WriteCount - m_ReadCount) & m_Mask; }
  inline unsigned Size() const { return bufferSize; }

  inline void Clear()
  {
    m_ReadCount=0;
    m_WriteCount=0;
  }

private:
  DataType m_Data[bufferSize];
  volatile IndexType m_ReadCount;
  volatile IndexType m_WriteCount;
  IndexType m_Mask;
};

template<typename T>
struct CircularBuffer
{
  int m_Head;
  int m_Tail;

  CircularBuffer()
    : m_Head(0)
    , m_Tail(0)
  {}

  void Resize(int size)
  {
    m_Head = 0;
    m_Tail = 0;
    m_Data.clear();
    m_Data.resize(size);
  }

  int GetSize() const
  {
    int count = m_Head - m_Tail;
    if (count < 0)
    {
      count += (int)m_Data.size();
    }
    return count;
  }

  void Add(const T &move)
  {
    m_Data[m_Head] = move;
    Next(m_Head);
  }

  void Remove()
  {
    //assert(!Empty());
    Next(m_Tail);
  }

  T& Oldest()
  {
    //assert(!Empty());
    return m_Data[m_Tail];
  }

  T& Newest()
  {
    //assert(!Empty());
    int index = m_Head - 1;
    if (index == -1)
    {
      index = (int)m_Data.size() - 1;
    }
    return m_Data[index];
  }

  bool Empty() const
  {
    return m_Head == m_Tail;
  }

  void Next(int &index)
  {
    ++index;
    if (index >= (int)m_Data.size())
    {
      index -= (int)m_Data.size();
    }
  }

  void Previous(int &index)
  {
    --index;
    if (index < 0)
    {
      index += (int)m_Data.size();
    }
  }

  T& operator[](int index)
  {
    assert(index >= 0);
    assert(index < (int)m_Data.size());
    
    return m_Data[index];
  }

private:
  std::vector<T> m_Data;
};

namespace Network
{
  enum
  {
    ID_RANDOM_DATA,
    ID_MESSAGE_STRING,
    ID_CLIENT_INFO,
    ID_SERVER_INFO,
    ID_UPDATE,

    ID_LAST
  };
}


#endif