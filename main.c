/**
   \file main.c

   \author G. Icking-Konert
   \date 2014-03-14
   \version 0.1
   
   \brief implementation of main routine
   
   this is the main file containing browsing the input parameters,
   calling the import, programming, and check routines.
   
   \note program not yet fully tested!
*/

// include files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

// OS specific: Win32
#if defined(WIN32)
  #include <windows.h>

// OS specific: Posix
#elif defined(__APPLE__) || defined(__unix__)
  #define HANDLE  int     // comm port handler is int
  #include <fcntl.h>      // File control definitions
  #include <termios.h>    // Posix terminal control definitions
  #include <getopt.h>
  #include <errno.h>    /* Error number definitions */
  #include <dirent.h>
  #include <sys/ioctl.h>

#else
  #error OS not supported
#endif

#define _MAIN_
  #include "globals.h"
#undef _MAIN_
#include "main.h"
#include "misc.h"
#include "serial_comm.h"
#include "bootloader.h"
#include "hexfile.h"

// device dependent flash w/e routines
#include "E_W_ROUTINEs_32K_ver_1.0.h"
#include "E_W_ROUTINEs_32K_ver_1.2.h"
#include "E_W_ROUTINEs_32K_ver_1.3.h"
#include "E_W_ROUTINEs_32K_ver_1.4.h"
#include "E_W_ROUTINEs_128K_ver_2.0.h"
#include "E_W_ROUTINEs_128K_ver_2.1.h"
#include "E_W_ROUTINEs_128K_ver_2.2.h"
#include "E_W_ROUTINEs_128K_ver_2.4.h"
#include "E_W_ROUTINEs_256K_ver_1.0.h"


#define  STRLEN   1000
#define  BUFSIZE  1000000



/**
   \fn int main(int argc, char *argv[])
   
   \brief main routine
   
   \param argc      number of commandline arguments + 1
   \param argv      string array containing commandline arguments (argv[0] contains name of executable)
   
   \return dummy return code (not used)
   
   Main routine for import, programming, and check routines
*/
int main(int argc, char ** argv) {
 
  char      *appname;             // name of application without path
  char      portname[STRLEN];     // name of communication port
  int       baudrate;             // communication baudrate [Baud]
  uint8_t   LINmode;              // use normal UART or LIN reply mode 
  uint8_t   enableBSL;            // don't enable ROM bootloader after upload (caution!)
  uint8_t   jumpFlash;            // jump to flash after upload
  int       flashsize;            // size of flash (kB) for w/e routines
  uint8_t   versBSL;              // BSL version for w/e routines
  char      hexfile[STRLEN];      // name of file to flash
  HANDLE    ptrPort;              // handle to communication port
  char      buf[BUFSIZE];         // buffer for hexfiles
  char      image[BUFSIZE];       // memory image buffer
  uint32_t  imageStart;           // starting address of image
  uint32_t  numBytes;             // number of bytes in image
  char      *ptr=NULL;            // pointer to memory
  int       i;                    // generic variable  
  

  // initialize global variables
  g_pauseOnExit = 1;            // wait for <return> before terminating
  
  // initialize default arguments
  portname[0] = '\0';           // no default port name
  baudrate   = 115200;          // default baudrate
  LINmode    = 0;               // use normal UART mode 
  jumpFlash  = 1;               // jump to flash after uploade
  enableBSL  = 1;               // enable bootloader after upload
  hexfile[0] = '\0';            // no default hexfile
  
  // for debugging only
  //sprintf(portname, "/dev/tty.usbserial-A4009I0O");

  // required for strncpy()
  portname[STRLEN-1]  = '\0';
  hexfile[STRLEN-1]   = '\0';
    
  
  ////////
  // parse commandline arguments
  ////////
  for (i=1; i<argc; i++) {
    
    // name of communication port
    if (!strcmp(argv[i], "-p"))
      strncpy(portname, argv[++i], STRLEN-1);

    // communication baudrate
    else if (!strcmp(argv[i], "-b"))
      sscanf(argv[++i],"%d",&baudrate);

    // use reply mode with 1-line interface, e.g. LIN
    else if (!strcmp(argv[i], "-r"))
      LINmode = 1;

    // name of hexfile
    else if (!strcmp(argv[i], "-f"))
      strncpy(hexfile, argv[++i], STRLEN-1);

    // don't enable ROM bootloader after upload (caution!)
    else if (!strcmp(argv[i], "-x"))
      enableBSL = 0;

    // don't jump to address after upload
    else if (!strcmp(argv[i], "-j"))
      jumpFlash = 0;

    // don't prompt for <return> prior to exit
    else if (!strcmp(argv[i], "-q"))
      g_pauseOnExit = 0;

    // else print list of commandline arguments and language commands
    else {
      if (strrchr(argv[0],'\\'))
        appname = strrchr(argv[0],'\\')+1;         // windows
      else if (strrchr(argv[0],'/'))
        appname = strrchr(argv[0],'/')+1;          // Posix
      else
        appname = argv[0];
      printf("\n");
      printf("usage: %s [-h] [-p COMx] [-b BR] [-r] [-f file] [-x] [-j] [-q]\n\n", appname);
      printf("  -h    print this help\n");
      printf("  -p    name of communication port (default: list all ports and query)\n");
      printf("  -b    communication baudrate in Baud (default: 115200)\n");
      printf("  -r    use LIN reply mode (default: off)\n");
      printf("  -f    name of s19 or intel-hex file to flash (default: none)\n");
      printf("  -x    don't enable ROM bootloader after upload (default: enable)\n");
      printf("  -j    don't jump to flash after upload (default: jump to flash)\n");
      printf("  -q    don't prompt for <return> prior to exit (default: prompt)\n");
      printf("\n");
      Exit(0, 0);
    }

  } // process commandline arguments
  
  
  ////////
  // print message
  ////////
  printf("\n");
  printf("STM8 Serial Flasher (v%1.1f)\n", VERSION);
  
  
  ////////
  // if no port name is given, list all available ports and query
  ////////
  if (strlen(portname) == 0) {
    printf("  enter comm port name ( ");
    list_ports();
    printf(" ): ");
    scanf("%s", portname);
    getchar();
  } // if no comm port name


  ////////
  // open port with given properties
  ////////
  if (LINmode)
    ptrPort = init_port(portname, baudrate, 1000, 8, 0, 1, 0, 0);   // use no parity
  else  
    ptrPort = init_port(portname, baudrate, 1000, 8, 2, 1, 0, 0);   // use even parity


  ////////
  // open port with given properties
  ////////

  // synchronize baudrate
  bsl_sync(ptrPort, LINmode, 1);
  
  // get bootloader info for selecting flash w/e routines
  bsl_getInfo(ptrPort, LINmode, 1, &flashsize, &versBSL);

  // select device dependent flash routines for upload
  if ((flashsize==32) && (versBSL==0x10)) {
    ptr = (char*) STM8_Routines_E_W_ROUTINEs_32K_ver_1_0_s19;
    ptr[STM8_Routines_E_W_ROUTINEs_32K_ver_1_0_s19_len]=0;
  }
  else if ((flashsize==32) && (versBSL==0x12)) {
    ptr = (char*) STM8_Routines_E_W_ROUTINEs_32K_ver_1_2_s19;
    ptr[STM8_Routines_E_W_ROUTINEs_32K_ver_1_2_s19_len]=0;
  }
  else if ((flashsize==32) && (versBSL==0x13)) {
    ptr = (char*) STM8_Routines_E_W_ROUTINEs_32K_ver_1_3_s19;
    ptr[STM8_Routines_E_W_ROUTINEs_32K_ver_1_3_s19_len]=0;
  }
  else if ((flashsize==32) && (versBSL==0x14)) {
    ptr = (char*) STM8_Routines_E_W_ROUTINEs_32K_ver_1_4_s19;
    ptr[STM8_Routines_E_W_ROUTINEs_32K_ver_1_4_s19_len]=0;
  }
  else if ((flashsize==128) && (versBSL==0x20)) {
    ptr = (char*) STM8_Routines_E_W_ROUTINEs_128K_ver_2_0_s19;
    ptr[STM8_Routines_E_W_ROUTINEs_128K_ver_2_0_s19_len]=0;
  }
  else if ((flashsize==128) && (versBSL==0x21)) {
    ptr = (char*) STM8_Routines_E_W_ROUTINEs_128K_ver_2_1_s19;
    ptr[STM8_Routines_E_W_ROUTINEs_128K_ver_2_1_s19_len]=0;
  }
  else if ((flashsize==128) && (versBSL==0x22)) {
    ptr = (char*) STM8_Routines_E_W_ROUTINEs_128K_ver_2_2_s19;
    ptr[STM8_Routines_E_W_ROUTINEs_128K_ver_2_2_s19_len]=0;
  }
  else if ((flashsize==128) && (versBSL==0x24)) {
    ptr = (char*) STM8_Routines_E_W_ROUTINEs_128K_ver_2_4_s19;
    ptr[STM8_Routines_E_W_ROUTINEs_128K_ver_2_4_s19_len]=0;
  }
  else if ((flashsize==256) && (versBSL==0x10)) {
    ptr = (char*) STM8_Routines_E_W_ROUTINEs_256K_ver_1_0_s19;
    ptr[STM8_Routines_E_W_ROUTINEs_256K_ver_1_0_s19_len]=0;
  }
  else {
    fprintf(stderr, "\n\nerror: unsupported device, exit!\n\n");
    Exit(1, g_pauseOnExit);
  }
  
  // convert device dependent flash routines to memory image
  convert_s19(ptr, &imageStart, &numBytes, image, 0);

  // upload flash routines to RAM
  bsl_memWrite(ptrPort, LINmode, 0, imageStart, numBytes, image);


  // if specified import & upload hexfile
  if (strlen(hexfile)>0) {
    
    // import hexfile
    load_hexfile(hexfile, buf, BUFSIZE);
    
    // convert to memory image, depending on file type
    if (strstr(hexfile, ".s19") != NULL)                                              // Motorola S-record format
      convert_s19(buf, &imageStart, &numBytes, image, 1);
    else if ((strstr(hexfile, ".hex") != NULL) || (strstr(hexfile, ".ihx") != NULL))  // Intel HEX-format
      convert_hex(buf, &imageStart, &numBytes, image, 1);
    else {
      fprintf(stderr, "\n\nerror: unsupported file format, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    
    // for speed erase in 1kB blocks --> skip, because not faster but higher risk
    /*
    for (i=imageStart; i<imageStart+numBytes; i+=1024) {
      bsl_flashErase(ptrPort, LINmode, 1, i);    
    }
    */
    
    // upload data to flash or RAM
    bsl_memWrite(ptrPort, LINmode, 1, imageStart, numBytes, image);

  } // upload hexfile

  
  
  // memory read
  //imageStart = 0x8000;  numBytes = 128*1024;   // complete 128kB flash
  //imageStart = 0x00A0;  numBytes = 352;        // RAM
  //bsl_memRead(ptrPort, LINmode, 1, imageStart, numBytes, image);
  
  
  // enable ROM bootloader after upload
  if (enableBSL==1) {
    //bsl_activateBSL(ptrPort, LINmode, 1);
    bsl_memWrite(ptrPort, LINmode, 1,  0x487E, 2, (char*)"\x55\xAA");
  }
  
  // jump to flash start address after upload
  if (jumpFlash)
    bsl_jumpTo(ptrPort, LINmode, 1, 0x8000);
  
  
  ////////
  // clean up and exit
  ////////
  close_port(&ptrPort);
  printf("done with program\n");
  Exit(0, g_pauseOnExit);
  
  // avoid compiler warnings
  return(0);
  
} // main


// end of file
