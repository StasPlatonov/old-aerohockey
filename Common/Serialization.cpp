#include "Serialization.h"

#if defined(__BORLANDC__)
  #include <mem.h>
#else 
  #if !defined(WIN32) && !defined(WINNT)
    #include <string.h>
  #endif
#endif

namespace Common
{
  BinarySerializer::BinarySerializer(std::vector<unsigned char>& destination)
    : m_Destination(destination)
  {
  }

  void BinarySerializer::Serialize(char ch)
  {
    m_Destination.push_back(ch);
  }

  void BinarySerializer::Serialize(int i)
  {
    const int sz = sizeof(i);
    std::vector<char> data(sz, 0);
    memcpy(&data.front(), &i, sz);
    m_Destination.insert(m_Destination.end(), data.begin(), data.end());
  }

  void BinarySerializer::Serialize(float f)
  {
    const int sz = sizeof(f);
    std::vector<char> data(sz, 0);
    memcpy(&data.front(), &f, sz);
    m_Destination.insert(m_Destination.end(), data.begin(), data.end());
  }

  void BinarySerializer::Serialize(unsigned long ul)
  {
    const int sz = sizeof(ul);
    std::vector<char> data(sz, 0);
    memcpy(&data.front(), &ul, sz);
    m_Destination.insert(m_Destination.end(), data.begin(), data.end());
  }

  void BinarySerializer::Serialize(unsigned long long ull)
  {
    const int sz = sizeof(ull);
    std::vector<char> data(sz, 0);
    memcpy(&data.front(), &ull, sz);
    m_Destination.insert(m_Destination.end(), data.begin(), data.end());
  }

  void BinarySerializer::Serialize(const std::string& s)
  {
    Serialize((int)s.size());
    Serialize(s.c_str(), s.size());
  }

  void BinarySerializer::Serialize(const void* p, int size)
  {
    std::vector<char> data(size, 0);
    memcpy(&data.front(), p, size);
    m_Destination.insert(m_Destination.end(), data.begin(), data.end());
  }
  //-----------------

  BinaryDeserializer::BinaryDeserializer(const unsigned char* data, int size)
    : m_Source(data)
    , m_SourceSize(size)
    , m_Position(0)
  {
  }

  void BinaryDeserializer::Ignore(int sz)
  {
    m_Position += sz;
  }

  void BinaryDeserializer::Deserialize(char& ch) const
  {
    Deserialize(&ch, sizeof(ch));
  }

  void BinaryDeserializer::Deserialize(int& i) const
  {
    Deserialize(&i, sizeof(i));
  }

  void BinaryDeserializer::Deserialize(float& f) const
  {
    Deserialize(&f, sizeof(f));
  }

  void BinaryDeserializer::Deserialize(unsigned long& ul) const
  {
    Deserialize(&ul, sizeof(ul));
  }

  void BinaryDeserializer::Deserialize(unsigned long long& ull) const
  {
    Deserialize(&ull, sizeof(ull));
  }

  void BinaryDeserializer::Deserialize(std::string& s) const
  {
    int sz = 0;
    Deserialize((int&)sz);
    s.resize(sz);
#if defined(__BORLANDC__)
    Deserialize(s.begin(), sz);
    return;
#else
    std::vector<char> tmp(sz, 0);
    Deserialize(&tmp.front(), sz);
    s = std::string(tmp.begin(), tmp.end());
#endif
  }
 
  void BinaryDeserializer::Deserialize(void* data, int size) const
  {
    memcpy(data, m_Source + m_Position, size);
    m_Position += size;
  }
}
