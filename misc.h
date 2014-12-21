/**
  \file misc.h
   
  \author G. Icking-Konert
  \date 2014-03-14
  \version 0.1
   
  \brief declaration of misc routines
   
  declaration of routines not really fitting anywhere else
*/

// for including file only once
#ifndef _MISC_H_
#define _MISC_H_


// include files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


/// for sleep(ms) use system specific routines
#if defined(WIN32)
  #define SLEEP(a)    Sleep(a)
#elif defined(__APPLE__) || defined(__unix__)
  #define SLEEP(a)    usleep((int32_t) a*1000L)
#else
  #error OS not supported
#endif

/// terminate program after cleaning up
void        Exit(uint8_t code, uint8_t pause);

/// strip path from application name
void        stripPath(char *in, char *out);

#endif // _MISC_H_

// end of file
