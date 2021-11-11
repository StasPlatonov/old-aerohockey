/**
 * -------------------------------------
 * Copyright (c) 2013 SberTech
 * All rights are reserved
 * -------------------------------------
 *
 * @file DLL.h
 *
 * @brief Thread object
 *
 * SVN information
 * @version $Rev: 185 $
 * @author  $Author: sbt-platonov-sv $
 * @date    $Date: 2013-10-21 19:38:25 +0400 (Пн, 21 окт 2013) $
 */
#ifndef COMMON_THREAD_H
#define COMMON_THREAD_H

#include "Common/Mutex.h"
#include "Common/Templates.h"

#include <boost/shared_ptr.hpp>

#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#else
  #include <pthread.h>
#endif

namespace Common
{
  class EventObject: public Common::NonCopyable
  {
  public:
    EventObject(bool manualReset = true, bool initState = false);
    ~EventObject();

    //! @brief sets event to signaled state
    void Set();

    //! @brief reset event to non-signaled state
    void Reset();

    //! @brief checks if event is in signaled state
    //! @return true if event is in signaled state
    bool IsSet() const;

    //! @brief waits event to become signaled for a timeout interval
    //! @return true if event is in signaled state or false if timeout is expired
    int Wait(int timeoutMS) const;

  private:
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
    typedef HANDLE HANDLE_T;
#else
    typedef struct HANDLE
    {
      Common::Mutex Mut;
      pthread_cond_t CondVar;
      bool ManualReset;
      bool Signaled;
    }* HANDLE_T;
#endif
    mutable HANDLE Handle;
  };
  //@todo: WaitForMultipleEvents(EventObject *events, int count, bool waitAll, int milliseconds, int &waitIndex);

  /*!
   Simple class that executes member function in separate thread.
   */
  class ThreadObject: public Common::NonCopyable
  {
  public:
    typedef boost::shared_ptr<ThreadObject> Ptr;

    /*! @brief c_tor()
     *  @param waitOnTermination timeout in ms to wait thread in d_tor() before termination
     */
    explicit ThreadObject(int waitOnTermination);
    virtual ~ThreadObject();

    //! @brief Start thread function
    void StartThread();

    //! @brief Set cancel event/ cancels thread and WaitThread(stopWaitTime)
    //! @param waitTimeout timeout to wait thread before termination
    void StopThread(int waitTimeout = 0);

    //! @brief Wait for thread object to complete its function.
    //! @param timeOut time to wait. 0 for infinite wait.
    bool WaitThread(int timeOut = 0);

    /*!
     @brief Checks if thread is stopping now.
     Another words this tells us has StopThread() been called or not.
     */
    bool IsStopping() const;

    //! @brief wait for StopEvent for the specific period of time.
    bool WaitForStopEvent(int interval) const;

    /*! @brief this will be executed in separate thread.
     The derived classes have to guarantee that this function takes no more than stopWaitTime or
     check StopEvent too.
     */
    virtual void OnRunThread() = 0;

  private:
    static
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
    unsigned long __stdcall
#else
    void*
#endif
    ThreadProc(void* param);

  protected:
    int StopWaitTime;
#if defined(WIN32) || defined(WINNT) || defined(__BORLANDC__)
    HANDLE HThread;
#else
    pthread_t HThread;
#endif
    EventObject StopEvent;
  };
  //--------------------------------------------------------------------------

  /*!
   @brief Class that provides periodical member function's call after a constant interval of time
   */
  class TimerObject: protected ThreadObject
  {
  public:
    //! @param interval time after which OnTimer() will be called.
    explicit TimerObject(int interval);
    virtual ~TimerObject();

    void Start()
    {
      StartThread();
    }

    //! @brief stop timer
    //! @param waitTimeout time to wait timer for. After this time passed, timer's thread will be terminated.
    //! Pass 0 for infinite wait.
    void Stop(int waitTimeout = 0)
    {
      StopThread(waitTimeout);
    }

    void TimerEvent();

    //! @brief main timer callback (in separate thread)
    virtual void OnTimer() = 0;

    virtual bool IsStopping() const
    {
      return ThreadObject::IsStopping();
    }

  private:
    //Hide some interfaces
    void StartThread()
    {
      return ThreadObject::StartThread();
    }
    void StopThread(int waitTimeout)
    {
      return ThreadObject::StopThread(waitTimeout);
    }
    virtual void OnRunThread();

  private:
    int Interval;
  };
  //--------------------------------------------------------------------------

  /*!
   @brief Asynchronous job class.
   the main difference from Timer is that there is
   */
  class JobObject: protected ThreadObject
  {
  public:
    JobObject();
    virtual ~JobObject();

    /*!
     All job's initialization have to be implemented here. Called in separate thread.
     */
    virtual void Init() = 0;

    /*!
     @brief Checks have job to continue iterations
     */
    virtual bool Condition() const = 0;
    /*!
     @brief a job working procedure. All jobs have to make their calculations here.
     It is possible to split job's work into several iterations.
     */
    virtual void Iteration() = 0;

    /*!
     All job's deinitialization have to be implemented here
     @param[out] cancelled returns if the job was cancelled by Cancel() call
     */
    virtual void Done(bool cancelled) = 0;

    /*!
     @brief starts a job
     */
    void Start()
    {
      StartThread();
    }

    //! Forces job to finish work
    //! @param waitTimeout time to wait job for. After this time passed, job's thread will be terminated.
    //! Pass 0 for infinite wait.
    void Cancel(int waitTimeout = 0)
    {
      StopThread(waitTimeout);
    }

    //! @brief wait for thread to complete
    //! @param waitTimeout time to wait.
    //! @result false if timeout passed or true if job is successfully completed.
    bool Wait(int waitTimeout = 0)
    {
      return ThreadObject::WaitThread(waitTimeout);
    }

  private:
    //Hide some interfaces
    void StartThread()
    {
      return ThreadObject::StartThread();
    }
    void StopThread(int waitTimeout)
    {
      return ThreadObject::StopThread(waitTimeout);
    }
    virtual void OnRunThread();
  };
//--------------------------------------------------------------------------
}

#endif
