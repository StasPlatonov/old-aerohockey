/**
 * -------------------------------------
 * Copyright (c) 2012 SberTech
 * All rights are reserved
 * -------------------------------------
 *
 * @file
 *
 * @brief Common Types
 *
 * SVN information
 * @version $Rev: 185 $
 * @author  $Author: sbt-platonov-sv $
 * @date    $Date: 2013-10-21 19:38:25 +0400 (Пн, 21 окт 2013) $
 */

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#if defined (__BORLANDC__)
  #include "_null.h"
#endif

#if defined(WIN32) || defined(WINNT)
  typedef __int64 int64;
  #define PATH_SEPARATOR '\\'
#else
  #include <inttypes.h>
  typedef int64_t int64;
  #define PATH_SEPARATOR '/'
#endif

//to exclude windows.h from includes, redefine types used
typedef unsigned char BYTE;
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned int uint;

#if defined(WIN32) || defined(WINNT) || defined (__BORLANDC__)
#else
  typedef int BOOL;
  typedef int SOCKET;
  #define INVALID_SOCKET (-1)
#endif

#ifndef FALSE
  #define FALSE (0)
#endif

#ifndef TRUE
  #define TRUE (1)
#endif

#endif //COMMON_TYPES_H
