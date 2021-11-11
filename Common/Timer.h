/**
 * -------------------------------------
 * Copyright (c) 2012 SberTech
 * All rights are reserved
 * -------------------------------------
 *
 * @file Time.h
 *
 * @brief Time retrieving
 *
 * SVN information
 * @version $Rev: 182 $
 * @author  $Author: sbt-platonov-sv $
 * @date    $Date: 2013-10-15 12:15:52 +0400 (Вт, 15 окт 2013) $
 */

#ifndef COMMON_TIMER_H
#define COMMON_TIMER_H
#include "Common/Types.h"

#if defined(WIN32) || defined (WINNT) || defined(__BORLANDC__)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif

namespace Common
{
  typedef unsigned long TimeValueType;

  class Timer
  {
  public:
    Timer();
    static Timer& GetInstance();
    TimeValueType GetTime() const;

  private:
#if defined(WIN32) || defined (WINNT) || defined(__BORLANDC__)
    LARGE_INTEGER Freq;
#endif
  };
}
//---------------------------------------------------------------------------

#endif //COMMON_TIMER_H
