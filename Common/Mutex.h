/**
 * -------------------------------------
 * Copyright (c) 2012 SberTech
 * All rights are reserved
 * -------------------------------------
 *
 * @file Mutex.h
 *
 * @brief Simple mutex (implemented via critical section!).
 *
 * SVN information
 * @version $Rev: 185 $
 * @author  $Author: sbt-platonov-sv $
 * @date    $Date: 2013-10-21 19:38:25 +0400 (Пн, 21 окт 2013) $
 */

#ifndef MUTEX_H
#define MUTEX_H

#include "Common/Templates.h"

#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #include <pthread.h>
#endif

namespace Common
{
  class Mutex : public Common::NonCopyable
  {
    public:
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
    typedef CRITICAL_SECTION HANDLE_T;
#else
    typedef pthread_mutex_t HANDLE_T;
#endif

      Mutex()
      {
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
        InitializeCriticalSection(&m_Handle);
#else
        pthread_mutexattr_t mutexAttr;
        pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE_NP);
        pthread_mutex_init(&m_Handle, &mutexAttr);
        pthread_mutexattr_destroy(&mutexAttr);
#endif
      }
      ~Mutex()
      {
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
        DeleteCriticalSection(&m_Handle);
#else
        pthread_mutex_destroy(&m_Handle);
#endif
      }
      void Lock()
      {
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
        EnterCriticalSection(&m_Handle);
#else
        pthread_mutex_lock(&m_Handle);
#endif
      }

      bool TyLock() const
      {
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
        return (TryEnterCriticalSection(&m_Handle) == TRUE);
#else
        return !pthread_mutex_trylock(&m_Handle);
#endif
      }

      void Unlock()
      {
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
        LeaveCriticalSection(&m_Handle);
#else
        pthread_mutex_unlock(&m_Handle);
#endif
      }

      operator HANDLE_T*() const
      {
        return &m_Handle;
      }

  private:
    mutable HANDLE_T m_Handle;
  };

  typedef Locker<Common::Mutex> MutexLocker;
}

#endif //MUTEX_H
