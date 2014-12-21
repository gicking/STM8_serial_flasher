/**
  \file misc.c
  
  \author G. Icking-Konert
  \date 2014-03-14
  \version 0.1
   
  \brief implementation of misc routines
   
  implementation of routines not really fitting anywhere else
*/


#include "misc.h"


/**
  \fn void Exit(uint8_t code, uint8_t pause)
   
  \brief terminate program
   
  \param[in] code    return code of application to commandline
  \param[in] pause   wait for keyboard input before terminating

  Terminate program. Replaces standard exit() routine which doesn't allow
  for a \<return\> request prior to closing of the console window.
*/
void Exit(uint8_t code, uint8_t pause) {

  // optionally prompt for <return>
  if (pause) {
    printf("\npress <return> to exit");
    fflush(stdout);
    fflush(stdin);
    getchar();
  }
  printf("\n");

  // terminate application
  exit(code);

} // Exit



/**
  \fn void stripPath(char *in, char *out)
   
  \brief strip path from application name
   
  \param[in] in      name of application incl. path
  \param[in] out     name of application excl. path

  strip pathname from application name 
*/
void stripPath(char *in, char *out) {

  int32_t   i;
  char      *tmp;

  // find position of last path delimiter '/' (Posix) or '\' (Win)
  tmp = in;
  for (i=0; i<strlen(in); i++) {
    if ((in[i] == '/') || (in[i] == '\\'))
      tmp = in+i+1;
  }

  // copy name without path
  sprintf(out, "%s", tmp);

} // stripPath


// end of file
