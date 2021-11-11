/**
 * -------------------------------------
 * Copyright (c) 2012 SberTech
 * All rights are reserved
 * -------------------------------------
 *
 * @file Time.cpp
 *
 * @brief Time retrieving logic
 *
 * SVN information
 * @version $Rev: 182 $
 * @author  $Author: sbt-platonov-sv $
 * @date    $Date: 2013-10-15 12:15:52 +0400 (Вт, 15 окт 2013) $
 */

#include "Timer.h"

#if !defined (WIN32) && !defined(WINNT) && !defined(__BORLANDC__)
#include <sys/time.h>
#endif

namespace Common
{
  Timer::Timer()
  {
#if defined(WIN32) || defined (WINNT) || defined(__BORLANDC__)
    QueryPerformanceFrequency(&Freq);
#endif
  }

  Timer& Timer::GetInstance()
  {
    static Timer instance;
    return instance;
  }

  TimeValueType Timer::GetTime() const
  {
#if defined(WIN32) || defined (WINNT) || defined(__BORLANDC__)
    LARGE_INTEGER tval;
    QueryPerformanceCounter(&tval);
    return (TimeValueType)(tval.QuadPart * 1000 / Freq.QuadPart);
#else
    timeval tval;
    gettimeofday(&tval, 0);
    return tval.tv_sec * 1000 + tval.tv_usec / 1000;
#endif
  }
}
//---------------------------------------------------------------------------

