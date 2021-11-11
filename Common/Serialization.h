#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include <string>
#include <vector>

namespace Common
{
  class Serializer
  {
  public:
    virtual ~Serializer() {}
    virtual void Serialize(char ch) = 0;
    virtual void Serialize(int i) = 0;
    virtual void Serialize(float f) = 0;
    virtual void Serialize(unsigned long ul) = 0;
    virtual void Serialize(unsigned long long ull) = 0;
    virtual void Serialize(const std::string& s) = 0;
    virtual void Serialize(const void* data, int size) = 0;
  };

  class BinarySerializer : public Serializer
  {
  public:
    BinarySerializer(std::vector<unsigned char>& destination);

    virtual void Serialize(char ch);
    virtual void Serialize(int i);
    virtual void Serialize(float f);
    virtual void Serialize(unsigned long ul);
    virtual void Serialize(unsigned long long ull);
    virtual void Serialize(const std::string& s);
    virtual void Serialize(const void* data, int size);

  private:
      std::vector<unsigned char>& m_Destination;
  };

  class Deserializer
  {
  public:
    virtual ~Deserializer() {}

    virtual void Ignore(int sz) = 0;
    virtual void Deserialize(char& ch) const = 0;
    virtual void Deserialize(int& i) const = 0;
    virtual void Deserialize(float& f) const = 0;
    virtual void Deserialize(unsigned long& ul) const = 0;
    virtual void Deserialize(unsigned long long& ull) const = 0;
    virtual void Deserialize(std::string& s) const = 0;
    virtual void Deserialize(void* data, int size) const = 0;
  };

  class BinaryDeserializer : public Deserializer
  {
  public:
    BinaryDeserializer(const unsigned char* data, int size);

    virtual void Ignore(int sz);
    virtual void Deserialize(char& ch)const ;
    virtual void Deserialize(int& i)const ;
    virtual void Deserialize(float& f)const ;
    virtual void Deserialize(unsigned long& ul)const ;
    virtual void Deserialize(unsigned long long& ull)const ;
    virtual void Deserialize(std::string& s)const ;
    virtual void Deserialize(void* data, int size)const ;

  private:
    const unsigned char* m_Source;
    int m_SourceSize;
    mutable int m_Position;
  };
}
#endif
