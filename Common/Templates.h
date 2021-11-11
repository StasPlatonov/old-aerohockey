#ifndef SINGLETON_H
#define SINGLETON_H

#include <memory>
//----------------------------------------------------------------------------

namespace Common
{
  template<class T>
  class Locker
  {
  public:
    explicit Locker(T& t)
     : m_Target(t)
    {
      m_Target.Lock();
    }
    ~Locker()
    {
      m_Target.Unlock();
    }
  private:
    T& m_Target;
  };
  //----------------------------------------------------------------------------

  class NonCopyable
  {
  public:
    virtual ~NonCopyable() {}

  protected:
    NonCopyable() {}

  private:
    NonCopyable(const NonCopyable&) {};
    void operator =(const NonCopyable&) {};
  };
  //----------------------------------------------------------------------------

  template<typename T>
  class Singleton
  {
  public:
    static T& GetInstance()
    {
      static std::auto_ptr<T> instance;
      if (!instance.get())
      {
        instance.reset(new T);
      }
      return *instance.get();
    }

  protected:
    Singleton() {};
    virtual ~Singleton() {}

  private:
    Singleton(const Singleton&) {};
    void operator =(const Singleton&) {};
  };
  //---------------------------------------------------------------------------
}

#endif //SINGLETON_H
