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
#include "version.h"


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
  uint8_t   resetSTM8;            // 0=no reset; 1=HW reset via DTR (RS232/USB) or GPIO18 (Raspi); 2=SW reset by sending 0x55+0xAA
  uint8_t   enableBSL;            // don't enable ROM bootloader after upload (caution!)
  uint8_t   jumpFlash;            // jump to flash after upload
  uint8_t   pauseOnLaunch;        // prompt for <return> prior to upload
  int       flashsize;            // size of flash (kB) for w/e routines
  uint8_t   versBSL;              // BSL version for w/e routines
  char      hexfile[STRLEN];      // name of file to flash
  HANDLE    ptrPort;              // handle to communication port
  char      buf[BUFSIZE];         // buffer for hexfiles
  char      image[BUFSIZE];       // memory image buffer
  uint32_t  imageStart;           // starting address of image
  uint32_t  numBytes;             // number of bytes in image
  char      *ptr=NULL;            // pointer to memory
  int       i, j;                 // generic variables  
  //char      Tx[100], Rx[100];     // debug: buffer for tests
  

  // initialize global variables
  g_pauseOnExit = 1;            // wait for <return> before terminating
  g_UARTmode    = 0;            // 2-wire interface with UART duplex mode
  
  
  // initialize default arguments
  portname[0] = '\0';           // no default port name
  baudrate   = 230400;          // default baudrate
  resetSTM8  = 0;               // don't automatically reset STM8
  jumpFlash  = 1;               // jump to flash after uploade
  pauseOnLaunch = 1;            // prompt for return prior to upload
  enableBSL  = 1;               // enable bootloader after upload
  hexfile[0] = '\0';            // no default hexfile
  
  // for debugging only
  //sprintf(portname, "/dev/tty.usbserial-A4009I0O");

  // required for strncpy()
  portname[STRLEN-1]  = '\0';
  hexfile[STRLEN-1]   = '\0';
    
    
  // reset console color (needs to be called once for Win32)      
  setConsoleColor(PRM_COLOR_DEFAULT);


  ////////
  // parse commandline arguments
  ////////
  for (i=1; i<argc; i++) {
    
    // debug: print argument
    //printf("arg %d: '%s'\n", (int) i, argv[i]);
    
    // name of communication port
    if (!strcmp(argv[i], "-p"))
      strncpy(portname, argv[++i], STRLEN-1);

    // communication baudrate
    else if (!strcmp(argv[i], "-b"))
      sscanf(argv[++i],"%d",&baudrate);

    // UART mode: 0=duplex, 1=1-wire reply, 2=2-wire reply (default: duplex)\n");
    else if (!strcmp(argv[i], "-u")) {
      sscanf(argv[++i], "%d", &j);
      g_UARTmode = j;
    }

    // name of hexfile
    else if (!strcmp(argv[i], "-f"))
      strncpy(hexfile, argv[++i], STRLEN-1);

    // HW reset STM8 via DTR line (RS232/USB) or GPIO18 (Raspi only)
    else if (!strcmp(argv[i], "-r")) {
      sscanf(argv[++i], "%d", &j);
      resetSTM8 = j;
    }
    
    // don't enable ROM bootloader after upload (caution!)
    else if (!strcmp(argv[i], "-x"))
      enableBSL = 0;

    // don't jump to address after upload
    else if (!strcmp(argv[i], "-j"))
      jumpFlash = 0;

    // don't prompt for <return> prior to upload
    else if (!strcmp(argv[i], "-Q"))
      pauseOnLaunch = 0;

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
      printf("usage: %s [-h] [-p COMx] [-b BR] [-u mode] [-f file] [-r] [-x] [-j] [-Q] [-q]\n\n", appname);
      printf("  -h        print this help\n");
      printf("  -p name   name of communication port (default: list all ports and query)\n");
      printf("  -b baud   communication baudrate in Baud (default: 230400)\n");
      printf("  -u N      UART mode: 0=duplex, 1=1-wire reply, 2=2-wire reply (default: duplex)\n");
      printf("  -f file   name of s19 or intel-hex file to flash (default: none)\n");
      #ifdef __ARMEL__
        printf("  -r rst    reset STM8: 1=DTR line (RS232), 2=send 'Re5eT!' @ 115.2kBaud, 3=GPIO18 pin (Raspi) (default: no reset)\n");
      #else
        printf("  -r rst    reset STM8: 1=DTR line (RS232), 2=send 'Re5eT!' @ 115.2kBaud (default: no reset)\n");
      #endif
      printf("  -x        don't enable ROM bootloader after upload (default: enable)\n");
      printf("  -j        don't jump to flash after upload (default: jump to flash)\n");
      printf("  -Q        don't prompt for <return> prior to upload (default: prompt)\n");
      printf("  -q        don't prompt for <return> prior to exit (default: prompt)\n");
      printf("\n");
      Exit(0, 0);
    }

  } // process commandline arguments
  
  

  ////////
  // print app name & version, and change console title
  ////////
  get_app_name(argv[0], VERSION, buf);
  printf("\n%s\n", buf);
  setConsoleTitle(buf);  
  
  
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
  // put STM8 into bootloader mode
  ////////
  if (pauseOnLaunch) {
    printf("  activate STM8 bootloader and press <return>");
    fflush(stdout);
    fflush(stdin);
    getchar();
  }


  ////////
  // open port with given properties
  ////////
  printf("  open port '%s' with %gkBaud ... ", portname, (float) baudrate / 1000.0);
  fflush(stdout);
  if (g_UARTmode == 0)
    ptrPort = init_port(portname, baudrate, 1000, 8, 2, 1, 0, 0);   // use even parity
  else
    ptrPort = init_port(portname, baudrate, 1000, 8, 0, 1, 0, 0);   // use no parity
  printf("ok\n");
  fflush(stdout);
  
 
  // debug: communication test (echo+1 test-SW on STM8)
  /*
  printf("open: %d\n", ptrPort);
  for (i=0; i<254; i++) {
    Tx[0] = i;
    send_port(ptrPort, 1, Tx);
    receive_port(ptrPort, 1, Rx);
	printf("%d  %d\n", (int) Tx[0], (int) Rx[0]);
  }
  printf("done\n");
  Exit(1,0);
  */
  

  ////////
  // communicate with STM8 bootloader
  ////////

  // HW reset STM8 using DTR line (USB/RS232)
  if (resetSTM8 == 1) {
    printf("  reset via DTR ... ");
    pulse_DTR(ptrPort, 10);
    printf("done\n");
    SLEEP(5);                       // allow BSL to initialize
  }
  
  // SW reset STM8 via command 'Re5eT!' at 115.2kBaud (requires respective STM8 SW)
  else if (resetSTM8 == 2) {
    set_baudrate(ptrPort, 115200);    // expect STM8 SW to receive at 115.2kBaud
    printf("  reset via UART command ... ");
    sprintf(buf, "Re5eT!");           // reset command (same as in STM8 SW!)
    for (i=0; i<6; i++) {
      send_port(ptrPort, 1, buf+i);   // send reset command bytewise to account for slow handling
      SLEEP(10);
    }
    printf("done\n");
    set_baudrate(ptrPort, baudrate);  // restore specified baudrate
  }
  
  // HW reset STM8 using GPIO18 pin (only Raspberry Pi!)
  #ifdef __ARMEL__
    else if (resetSTM8 == 3) {
      printf("  reset via GPIO18 ... ");
      pulse_GPIO(18, 10);
      printf("done\n");
      SLEEP(5);                       // allow BSL to initialize
    }
  #endif // __ARMEL__
  
  // synchronize baudrate
  bsl_sync(ptrPort);
  
  // get bootloader info for selecting flash w/e routines
  bsl_getInfo(ptrPort, &flashsize, &versBSL);

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
    setConsoleColor(PRM_COLOR_RED);
    fprintf(stderr, "\n\nerror: unsupported device, exit!\n\n");
    Exit(1, g_pauseOnExit);
  }
  
  // convert device dependent flash routines to memory image
  convert_s19(ptr, &imageStart, &numBytes, image);

  // upload flash routines to RAM
  bsl_memWrite(ptrPort, imageStart, numBytes, image, 0);


  // if specified import & upload hexfile
  if (strlen(hexfile)>0) {
    
    // import hexfile
    load_hexfile(hexfile, buf, BUFSIZE);
    
    // convert to memory image, depending on file type
    if (strstr(hexfile, ".s19") != NULL)                                              // Motorola S-record format
      convert_s19(buf, &imageStart, &numBytes, image);
    else if ((strstr(hexfile, ".hex") != NULL) || (strstr(hexfile, ".ihx") != NULL))  // Intel HEX-format
      convert_hex(buf, &imageStart, &numBytes, image);
    else {
      setConsoleColor(PRM_COLOR_RED);
      fprintf(stderr, "\n\nerror: unsupported file format, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    
    // for speed erase in 1kB blocks --> skip, because not faster but higher risk
    /*
    for (i=imageStart; i<imageStart+numBytes; i+=1024) {
      bsl_flashErase(ptrPort, i);    
    }
    */
    
    // upload data to flash or RAM
    bsl_memWrite(ptrPort, imageStart, numBytes, image, 1);

  } // upload hexfile

  
  
  // memory read
  //imageStart = 0x8000;  numBytes = 128*1024;   // complete 128kB flash
  //imageStart = 0x00A0;  numBytes = 352;        // RAM
  //bsl_memRead(ptrPort, imageStart, numBytes, image);
  
  
  // enable ROM bootloader after upload (option bytes always on same address)
  if (enableBSL==1) {
    printf("  activate bootloader ... ");
    bsl_memWrite(ptrPort, 0x487E, 2, (char*)"\x55\xAA", 0);
    printf("done\n");
  }
  
  // jump to flash start address after upload (reset vector always on same address)
  if (jumpFlash)
    bsl_jumpTo(ptrPort, 0x8000);
  
  
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
