/**
 * -------------------------------------
 * Copyright (c) 2013 SberTech
 * All rights are reserved
 * -------------------------------------
 *
 * @file Thread.cpp
 *
 * @brief Thread object
 *
 * SVN information
 * @version $Rev: 185 $
 * @author  $Author: sbt-platonov-sv $
 * @date    $Date: 2013-10-21 19:38:25 +0400 (Пн, 21 окт 2013) $
 */
#include "Thread.h"

#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
  #include <process.h>
#else
  #include <time.h>
  #include <sys/time.h>
  #include <errno.h>

  #define INFINITE ~0
  #define WAIT_OBJECT_0 0
  #define WAIT_FAILED ~0
  #define WAIT_TIMEOUT 0x00000102L

#endif

#include <stdio.h>

namespace Common
{
  EventObject::EventObject(bool manualReset, bool initState)
  {
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
    Handle = CreateEvent(0, manualReset ? TRUE : FALSE, initState ? TRUE : FALSE, 0);
#else
    //specify clock source for condition variable (as we gonna be able to lock it for a small amount of time)
    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);

    pthread_cond_init(&Handle.CondVar, &attr);

    pthread_condattr_destroy(&attr);

    Handle.ManualReset = manualReset;
    Handle.Signaled = initState;
#endif
  }

  EventObject::~EventObject()
  {
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
    CloseHandle(Handle);
#else
    pthread_cond_destroy(&Handle.CondVar);
#endif
  }
  //--------------------------------------------------------------------------

  void EventObject::Set()
  {
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
    SetEvent(Handle);
#else
    Common::MutexLocker lock(Handle.Mut);
    Handle.ManualReset ? pthread_cond_broadcast(&Handle.CondVar) : pthread_cond_signal(&Handle.CondVar);
    Handle.Signaled = true;
#endif
  }
  //--------------------------------------------------------------------------

  void EventObject::Reset()
  {
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
    ResetEvent(Handle);
#else
    Common::MutexLocker lock(Handle.Mut);
    Handle.Signaled = false;
#endif
  }
  //--------------------------------------------------------------------------

  bool EventObject::IsSet() const
  {
    return (Wait(0) == WAIT_OBJECT_0);
  }
  //--------------------------------------------------------------------------

  int EventObject::Wait(int timeoutMS) const
  {
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
    return WaitForSingleObject(Handle, timeoutMS);
#else
    Common::MutexLocker lock(Handle.Mut);
    int ret = 0;

    if (!Handle.Signaled)
    {
      if (!timeoutMS)
      {
        return WAIT_TIMEOUT;
      }

      timespec timeout;
      if (timeoutMS != INFINITE)
      {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        unsigned long long int ns = (timeoutMS % 1000) * 1000000 + now.tv_nsec;

        timeout.tv_sec = now.tv_sec + timeoutMS / 1000 + ns / 1000000000;
        timeout.tv_nsec = ns % 1000000000;
      }
      do
      {
        ret = (
            timeoutMS == INFINITE ?
                pthread_cond_wait(&Handle.CondVar, Handle.Mut) :
                pthread_cond_timedwait(&Handle.CondVar, Handle.Mut, &timeout));
      }
      while (!ret && !Handle.Signaled);
    }

    //adjust value
    switch (ret)
    {
    case 0:
      if (!Handle.ManualReset)
      {
        Handle.Signaled = false;
      }
      return WAIT_OBJECT_0;

    case ETIMEDOUT:
    default:
      return WAIT_TIMEOUT;
    }

    return WAIT_TIMEOUT;
#endif
  }
  //--------------------------------------------------------------------------

  ThreadObject::ThreadObject(int waitOnTermination)
      : StopWaitTime(waitOnTermination), HThread(0), StopEvent(true, false)
  {
  }

  ThreadObject::~ThreadObject()
  {
    StopThread(StopWaitTime);
  }
  //--------------------------------------------------------------------------

#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
  unsigned long __stdcall
#else
  void*
#endif
  ThreadObject::ThreadProc(void* param)
  {
    if (param)
    {
      ThreadObject* thrd = static_cast<ThreadObject*>(param);
      thrd->OnRunThread();
      thrd->HThread = 0;
    }
    return 0;
  }

  //--------------------------------------------------------------------------

  void ThreadObject::StartThread()
  {
    StopEvent.Reset();
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
    HThread = CreateThread(NULL, 4096, &ThreadObject::ThreadProc, this, 0, 0);
#else
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setstacksize(&threadAttr, 65536);
    if (int error = pthread_create(&HThread, &threadAttr, &ThreadObject::ThreadProc, this))
    {
      printf("Failed to create thread (code: %d)\n", error);
    }

    pthread_attr_destroy(&threadAttr);
    //printf("thread created with id 0x%X\n", HThread);
#endif
  }
  //--------------------------------------------------------------------------

  bool ThreadObject::IsStopping() const
  {
    return StopEvent.IsSet();
  }
  //--------------------------------------------------------------------------

  bool ThreadObject::WaitForStopEvent(int interval) const
  {
    return (StopEvent.Wait(interval) != WAIT_TIMEOUT);
  }
  //--------------------------------------------------------------------------

  bool ThreadObject::WaitThread(int timeOut)
  {
    if (!HThread)
      return true;
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
    return (WaitForSingleObject(HThread, timeOut ? timeOut : INFINITE) != WAIT_TIMEOUT);
#else
    bool joined = false;

    if (timeOut)
    {
      // try timed join
      void* res = 0;
      struct timespec ts;
      if (clock_gettime(CLOCK_REALTIME, &ts) != -1)
      {
        ts.tv_sec += timeOut / 1000;
        ts.tv_nsec = (timeOut % 1000) * 1000;
        printf("Timed wait for %d s...\n", timeOut / 1000);

        if (int error = pthread_timedjoin_np(HThread, &res, &ts))
        {
          printf("Failed to timed join thread (code: %d)\n", error);
          return false;
        }
        else
        {
          joined = true;
        }
      }
    }

    if (!timeOut || !joined) // simple infinite join
    {
      printf("infinite wait...\n");
      void* res = 0;
      if (int error = pthread_join(HThread, &res))
      {
        printf("Failed to join thread (code: %d)\n", error);
        return false;
      }
    }

    return true;
#endif
  }
  //--------------------------------------------------------------------------

  void ThreadObject::StopThread(int timeout)
  {
    if (!HThread)
    {
      return;
    }

    printf("Set stop event\n");

    StopEvent.Set();
    if (!WaitThread(timeout))
    {
      printf("Thread %lu hanged. Terminating.\n", HThread);
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
      TerminateThread(HThread, 0);
      CloseHandle(HThread);
#else
      if (int error = pthread_cancel(HThread))
      {
        printf("Failed to cancel thread: %d\n", error);
        return;
      }
      //kill?
#endif
    }
    else
    {
      printf("Thread successfully stopped.\n");
    }
    HThread = 0;
  }
  //--------------------------------------------------------------------------

  TimerObject::TimerObject(int interval)
      : ThreadObject(1000), Interval(interval)
  {
  }

  TimerObject::~TimerObject()
  {
    Stop();
  }
  //--------------------------------------------------------------------------

  void TimerObject::TimerEvent()
  {
    //@todo: add event to force call Timer handler
  }
  //--------------------------------------------------------------------------

  void TimerObject::OnRunThread()
  {
    while (!WaitForStopEvent(Interval))
    {
      OnTimer();
    }
  }
  //--------------------------------------------------------------------------

  JobObject::JobObject()
      : ThreadObject(0)
  {
  }

  JobObject::~JobObject()
  {
    Cancel();
  }
  //--------------------------------------------------------------------------

  void JobObject::OnRunThread()
  {
    Init();
    while (!IsStopping() && Condition())
    {
      Iteration();
    }
    const bool cancelled = IsStopping();
    Done(cancelled);
  }
//--------------------------------------------------------------------------
} //Common

