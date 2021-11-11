#include "Utils.h"

#include <stdlib.h>

#if defined(WIN32) || defined(WINNT)
#else
#include <stdio.h>
#include <string.h>
#endif
namespace Utils
{
 /* namespace
  {
    static LARGE_INTEGER Freq;
    static __int64 Start;
    static __int64 FFreq;
  }

  Timer::Timer()
  {
    QueryPerformanceFrequency(&Freq);
    //for float timing
    QueryPerformanceFrequency((LARGE_INTEGER*)&FFreq);
    QueryPerformanceCounter((LARGE_INTEGER*)&Start);
  }

  Timer& Timer::GetInstance()
  {
    static Timer instance;
    return instance;
  }

  unsigned long long Timer::GetTime() const
  {
#ifdef _WIN32
    LARGE_INTEGER tval;
    QueryPerformanceCounter(&tval);
    return (unsigned long long)(tval.QuadPart * 1000 / Freq.QuadPart);
#else
    return 0;
#endif
  }

  float Timer::GetFTime() const
  {
    __int64 counter = 0;
    QueryPerformanceCounter((LARGE_INTEGER*)&counter);
    return (float) ((counter - Start) / double(FFreq));
  }*/

  std::string GetString()
  {
    const char* eols = "\r\n";
    char str[255] = {0};

	  fgets(str, sizeof(str), stdin);
	  if (strchr(eols, str[0]))
		  str[0]=0;

	  size_t len=strlen(str);
	  if (len>0 && (strchr(eols, str[len - 1])))
		  str[len-1]=0;
	  if (len>1 && (strchr(eols, str[len - 2])))
		  str[len-2]=0;

	  return std::string(str);
  }
}
