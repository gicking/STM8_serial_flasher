/**
  \file bootloader.c
   
  \author G. Icking-Konert
  \date 2014-03-14
  \version 0.1
   
  \brief implementation of STM bootloader routines
   
  implementation of of STM bootloader routines
*/


#include "bootloader.h"
#include "serial_comm.h"
#include "misc.h"
#include "globals.h"


/**
  \fn uint8_t bsl_sync(HANDLE ptrPort, uint8_t LINmode, uint8_t verbose)
   
  \brief synchronize to microcontroller BSL
   
  \param[in] ptrPort    handle to communication port
  \param[in] LINmode    normal(=0) or reply(=1) mode. Latter ignores LIN echo
  \param[in] verbose    level of verbosity and if application exits on fail 

  \return synchronization status (0=ok, 1=fail)
  
  synchronize to microcontroller BSL, e.g. baudrate. If already synchronized
  checks for NACK
*/
uint8_t bsl_sync(HANDLE ptrPort, uint8_t LINmode, uint8_t verbose) {
  
  int   i, count;
  int   lenTx, lenRx, len;
  char  Tx[1000], Rx[1000];

  // print message
  if (verbose) {
    printf("  synchronize ... ");
    fflush(stdout);
  }
  
  // init receive buffer
  for (i=0; i<1000; i++)
    Rx[i] = 0;

  // check if port is open
  if (!ptrPort) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_sync()': port not open, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  
  
  // purge input buffer
  flush_port(ptrPort); 
  
  // construct SYNC command
  lenTx = 1;
  Tx[0] = SYNCH;
  lenRx = 1;  
  
  count = 0;
  do {
    
    // send command
    len = send_port(ptrPort, lenTx, Tx);
    if (len != lenTx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_sync()': sending command failed, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }

    // for 1-wire line, ignore echo
    if (LINmode) {
      len = receive_port(ptrPort, lenTx, Rx);
      if (len != lenTx) {
        if (verbose) {
          fprintf(stderr, "\n\nerror in 'bsl_sync()': echo failed in reply mode, exit!\n\n");
          Exit(1, g_pauseOnExit);
        }
        else {
          return(1);
        }
      }
    } // if (ignoreEcho)
    
    // receive response with timeout
    len = receive_port(ptrPort, lenRx, Rx);

    // increase retry counter if no byte received
    if (len!=lenRx)
      count++;
    
    // just to make sure
    SLEEP(10);
    
  } while ((count!=10) && ((len!=lenRx) || ((Rx[0]!=ACK) && (Rx[0]!=NACK))));
  
  // check if ok
  if ((len==lenRx) && (Rx[0]==ACK)) {
    if (verbose) {
      printf("success (ACK)\n");
      fflush(stdout);
    }
    else {
      return(0);
    }
  }
  else if ((len==lenRx) && (Rx[0]==NACK)) {
    if (verbose) {
      printf("success (NACK)\n");
      fflush(stdout);
    }
    else {
      return(0);
    }
  }
  else if (len==lenRx) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_sync()': wrong response 0x%02x from BSL, exit!\n\n", Rx[0]);
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  else {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_sync()': no response from BSL, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }

  // return success
  return(0);

} // bsl_sync



/**
  \fn uint8_t bsl_getInfo(HANDLE ptrPort, uint8_t LINmode, uint8_t verbose, int *flashsize, uint8_t *vers))
   
  \brief get microcontroller type and BSL version (for correct w/e routines)
   
  \param[in]  ptrPort     handle to communication port
  \param[in]  LINmode     normal(=0) or reply(=1) mode. Latter ignores LIN echo
  \param[in]  verbose     level of verbosity and if application exits on fail 
  \param[out] flashsize   size of flashsize in kB (required for correct W/E routines)
  \param[out] vers        BSL version number (required for correct W/E routines)
  
  \return communication status (0=ok, 1=fail)
  
  query microcontroller type and BSL version info. This information is required
  to select correct version of flash write/erase routines
*/
uint8_t bsl_getInfo(HANDLE ptrPort, uint8_t LINmode, uint8_t verbose, int *flashsize, uint8_t *vers) {
  
  int   i;
  int   lenTx, lenRx, len;
  char  Tx[1000], Rx[1000];

  // print message
  if (verbose) {
    printf("  determine device ... ");
    fflush(stdout);
  }
 
  // init receive buffer
  for (i=0; i<1000; i++)
    Rx[i] = 0;

  // check if port is open
  if (!ptrPort) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_getInfo()': port not open, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  
  
  // purge input buffer
  flush_port(ptrPort); 
  SLEEP(50);              // required for some reason
  
  
  /////////
  // determine device flash size for selecting w/e routines (flash starts at 0x8000)
  /////////

  // read highest byte in flash
  if (bsl_memRead(ptrPort, LINmode, 0, 0x047FFF, 1, Rx) == 0)       // extreme density (256kB)
    *flashsize = 256;
  else if (bsl_memRead(ptrPort, LINmode, 0, 0x027FFF, 1, Rx) == 0)  // high density (128kB)
    *flashsize = 128;
  else if (bsl_memRead(ptrPort, LINmode, 0, 0x00FFFF, 1, Rx) == 0)  // medium density (32kB)
    *flashsize = 32;
  else if (bsl_memRead(ptrPort, LINmode, 0, 0x009FFF, 1, Rx) == 0)  // low density (8kB)
    *flashsize = 8;
  else {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_getInfo()': cannot identify device, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  
  
  /////////
  // get BSL version
  /////////
  
  // construct command
  lenTx = 2;
  Tx[0] = GET;
  Tx[1] = (Tx[0] ^ 0xFF);
  lenRx = 9;
  
  // send command
  len = send_port(ptrPort, lenTx, Tx);
  if (len != lenTx) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_getInfo()': sending command failed, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }

  // for 1-wire line, ignore echo
  if (LINmode) {
    len = receive_port(ptrPort, lenTx, Rx);
    if (len != lenTx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_getInfo()': echo failed in reply mode, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }
  } // if (ignoreEcho)
    
  // receive response with timeout
  len = receive_port(ptrPort, lenRx, Rx);
  if (len != lenRx) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_getInfo()': ACK timeout, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
    
  // check 2x ACKs
  if ((Rx[0]!=ACK) || (Rx[8]!=ACK)) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_getInfo()': ACK failure, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }

  
  // check if command codes are correct (just to be sure)
  if (Rx[3] != GET) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_getInfo()': wrong GET code (expect 0x%02x), exit!\n\n", GET);
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  if (Rx[4] != READ) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_getInfo()': wrong READ code (expect 0x%02x), exit!\n\n", READ);
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  if (Rx[5] != GO) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_getInfo()': wrong GO code (expect 0x%02x), exit!\n\n", GO);
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  if (Rx[6] != WRITE) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_getInfo()': wrong WRITE code (expect 0x%02x), exit!\n\n", WRITE);
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  if (Rx[7] != ERASE) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_getInfo()': wrong ERASE code (expect 0x%02x), exit!\n\n", ERASE);
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  
  
  // print BSL data
  /*
  printf("    version 0x%02x\n", Rx[2]);
  printf("    command codes:\n");
  printf("      GET   0x%02x\n", Rx[3]);
  printf("      READ  0x%02x\n", Rx[4]);
  printf("      GO    0x%02x\n", Rx[5]);
  printf("      WRITE 0x%02x\n", Rx[6]);
  printf("      ERASE 0x%02x\n", Rx[7]);
  fflush(stdout);
  */
  
  
  // copy version number
  *vers = Rx[2];
  
  // print message
  if (verbose) {
    printf("done (%dkB flash; BSL v%x.%x)\n", *flashsize, (((*vers)&0xF0)>>4), ((*vers) & 0x0F));
    fflush(stdout);
  }
  
  // avoid compiler warnings
  return(0);
  
} // bsl_getInfo



/**
  \fn uint8_t bsl_memRead(HANDLE ptrPort, uint8_t LINmode, uint8_t verbose, uint32_t addrStart, uint32_t numBytes, char *buf)
   
  \brief read from microcontroller memory
   
  \param[in] ptrPort    handle to communication port
  \param[in] LINmode    normal(=0) or reply(=1) mode. Latter ignores LIN echo
  \param[in] verbose    level of verbosity and if application exits on fail 
  \param[in] addrStart  starting address to read from
  \param[in] numBytes   number of bytes to read
  \param[in] buf        buffer to store data to
  
  \return communication status (0=ok, 1=fail)
  
  read from microcontroller memory via READ command
*/
uint8_t bsl_memRead(HANDLE ptrPort, uint8_t LINmode, uint8_t verbose, uint32_t addrStart, uint32_t numBytes, char *buf) {

  int       i, count;
  int       lenTx, lenRx, len;
  char      Tx[1000], Rx[1000];
  uint32_t  addrTmp, addrStep, idx=0;


  // print message
  if (verbose) {
    if (numBytes > 2048)
      printf("  read %1.1fkB from 0x%04x (%1.1fkB) ", (float) numBytes/1024.0, (int) addrStart, (float) idx/1024.0);
    else
      printf("  read %dB from 0x%04x (%dB) ", numBytes, (int) addrStart, idx);
    fflush(stdout);
  }
  
  // init receive buffer
  for (i=0; i<1000; i++)
    Rx[i] = 0;

  // check if port is open
  if (!ptrPort) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_memRead()': port not open, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  
  // init data buffer
  for (i=0; i<numBytes; i++)
    buf[i] = 0;


  // loop over addresses in <=256B steps
  count = 0;
  idx = 0;
  addrStep = 256;
  for (addrTmp=addrStart; addrTmp<addrStart+numBytes; addrTmp+=addrStep) {  
    
    // if addr too close to end of range reduce stepsize
    if (addrTmp+256 > addrStart+numBytes)
      addrStep = addrStart+numBytes-addrTmp;

  
    /////
    // send read command
    /////
  
    // construct command
    lenTx = 2;
    Tx[0] = READ;
    Tx[1] = (Tx[0] ^ 0xFF);
    lenRx = 1;
  
    // send command
    len = send_port(ptrPort, lenTx, Tx);
    if (len != lenTx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memRead()': sending command failed, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }

    // for 1-wire line, ignore echo
    if (LINmode) {
      len = receive_port(ptrPort, lenTx, Rx);
      if (len != lenTx) {
        if (verbose) {
          fprintf(stderr, "\n\nerror in 'bsl_memRead()': echo failed in reply mode, exit!\n\n");
          Exit(1, g_pauseOnExit);
        }
        else {
          return(1);
        }
      }
    } // if (ignoreEcho)
    
    // receive response with timeout
    len = receive_port(ptrPort, lenRx, Rx);
    if (len != lenRx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memRead()': ACK1 timeout, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }
    
    // check acknowledge
    if (Rx[0]!=ACK) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memRead()': ACK1 failure, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }

  
    /////
    // send address
    /////
  
    // construct address + checksum (XOR over address)
    lenTx = 5;
    Tx[0] = (char) (addrTmp >> 24);
    Tx[1] = (char) (addrTmp >> 16);
    Tx[2] = (char) (addrTmp >> 8);
    Tx[3] = (char) (addrTmp);
    Tx[4] = (Tx[0] ^ Tx[1] ^ Tx[2] ^ Tx[3]);
    lenRx = 1;
  
    // send command
    len = send_port(ptrPort, lenTx, Tx);
    if (len != lenTx) {      
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memRead()': sending address failed, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }

    // for 1-wire line, ignore echo
    if (LINmode) {
      len = receive_port(ptrPort, lenTx, Rx);
      if (len != lenTx) {
        if (verbose) {
          fprintf(stderr, "\n\nerror in 'bsl_memRead()': echo failed in reply mode, exit!\n\n");
          Exit(1, g_pauseOnExit);
        }
        else {
          return(1);
        }
      }
    } // if (ignoreEcho)
    
    // receive response with timeout
    len = receive_port(ptrPort, lenRx, Rx);
    if (len != lenRx) {      
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memRead()': ACK2 timeout, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }
    
    // check acknowledge
    if (Rx[0]!=ACK) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memRead()': ACK2 failure, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }

  
    /////
    // send number of bytes
    /////
  
    // construct number of bytes + checksum
    lenTx = 2;
    Tx[0] = addrStep-1;     // -1 from BSL
    Tx[1] = (Tx[0] ^ 0xFF);
    lenRx = addrStep + 1;
  
    // send command
    len = send_port(ptrPort, lenTx, Tx);
    if (len != lenTx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memRead()': sending range failed, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }

    // for 1-wire line, ignore echo
    if (LINmode) {
      len = receive_port(ptrPort, lenTx, Rx);
      if (len != lenTx) {
        if (verbose) {
          fprintf(stderr, "\n\nerror in 'bsl_memRead()': echo failed in reply mode, exit!\n\n");
          Exit(1, g_pauseOnExit);
        }
        else {
          return(1);
        }
      }
    } // if (ignoreEcho)
    
    // receive response with timeout
    len = receive_port(ptrPort, lenRx, Rx);
    if (len != lenRx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memRead()': data timeout, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }
    
    // check acknowledge
    if (Rx[0]!=ACK) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memRead()': ACK3 failure, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }

    // copy data to buffer
    for (i=1; i<lenRx; i++) {
      buf[idx++] = Rx[i];
      //printf("%d 0x%02x\n", i, (uint8_t) (Rx[i])); fflush(stdout);
    }
    
    // print progress
    if (((idx % 2048) == 0) && (verbose)) {
      if (numBytes > 1024)
        printf("%c  read %1.1fkB from 0x%04x (%1.1fkB) ", '\r', (float) numBytes/1024.0, (int) addrStart, (float) idx/1024.0);
      else
        printf("%c  read %dB from 0x%04x (%dB) ", '\r', numBytes, (int) addrStart, idx);
      fflush(stdout);
    }

  } // loop over address range 
  
  // print message
  if (verbose) {
    if (numBytes > 2048)
      printf("%c  read %1.1fkB from 0x%04x (%1.1fkB) ", '\r', (float) numBytes/1024.0, (int) addrStart, (float) idx/1024.0);
    else
      printf("%c  read %dB from 0x%04x (%dB) ", '\r', numBytes, (int) addrStart, idx);
    printf(" done\n");
    fflush(stdout);
  }
  
  // debug: print buffer
  /*
  printf("\n");
  printf("idx  addr  value\n");
  for (i=0; i<numBytes; i++) {
    printf("%3d   0x%04x    0x%02x\n", i+1, (int) (addrStart+i), (uint8_t) (buf[i]));
  }
  printf("\n");
  fflush(stdout);
  */
  
  // avoid compiler warnings
  return(0);
  
} // bsl_memRead



/**
  \fn uint8_t bsl_flashErase(HANDLE ptrPort, uint8_t LINmode, uint8_t verbose, uint32_t addr)
   
  \brief erase one microcontroller flash sector
  
  \param[in] ptrPort      handle to communication port
  \param[in] LINmode      normal(=0) or reply(=1) mode. Latter ignores LIN echo
  \param[in] verbose      level of verbosity and if application exits on fail 
  \param[in] addr         adress within 1kB sector to erase
  
  \return communication status (0=ok, 1=fail)
  
  sector erase for microcontroller flash
*/
uint8_t bsl_flashErase(HANDLE ptrPort, uint8_t LINmode, uint8_t verbose, uint32_t addr) {

  int       i;
  int       lenTx, lenRx, len;
  char      Tx[1000], Rx[1000];
  uint8_t   sector;

  // calculate sector code
  sector = (addr - 0x8000)/1024;


  // print message
  if (verbose) {
    if (addr>0xFFFFFF)
      printf("  erase flash address 0x%08x (code 0x%02x) ... ", addr, sector);
    else if (addr>0xFFFF)
      printf("  erase flash address 0x%06x (code 0x%02x) ... ", addr, sector);
    else
      printf("  erase flash address 0x%04x (code 0x%02x) ... ", addr, sector);
    fflush(stdout);
  }
  
  // init receive buffer
  for (i=0; i<1000; i++)
    Rx[i] = 0;

  // check if port is open
  if (!ptrPort) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_flashErase()': port not open, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  

  /////
  // send erase command
  /////
  
  // construct command
  lenTx = 2;
  Tx[0] = ERASE;
  Tx[1] = (Tx[0] ^ 0xFF);
  lenRx = 1;
  
  // send command
  len = send_port(ptrPort, lenTx, Tx);
  if (len != lenTx) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_flashErase()': sending command failed, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }

  // for 1-wire line, ignore echo
  if (LINmode) {
    len = receive_port(ptrPort, lenTx, Rx);
    if (len != lenTx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_flashErase()': echo failed in reply mode, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }
  } // if (ignoreEcho)
  
  // receive response with timeout
  len = receive_port(ptrPort, lenRx, Rx);
  if (len != lenRx) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_flashErase()': ACK1 timeout, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  
  // check acknowledge
  if (Rx[0]!=ACK) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_flashErase()': ACK1 failure, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }

  
  /////
  // send code of sector to erase
  /////

  // construct pattern
  lenTx = 3;
  Tx[0] = 0x00;
  Tx[1] = sector;
  Tx[2] = (Tx[0] ^ Tx[1]);
  lenRx = 1;

  // send command
  len = send_port(ptrPort, lenTx, Tx);
  if (len != lenTx) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_flashErase()': sending sector failed, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }

  // for 1-wire line, ignore echo
  if (LINmode) {
    len = receive_port(ptrPort, lenTx, Rx);
    if (len != lenTx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_flashErase()': echo failed in reply mode, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }
  } // if (ignoreEcho)
  
  
  // wait for erase to avoid communication timeout
  //SLEEP(10000);
  
  
  // receive response with timeout
  len = receive_port(ptrPort, lenRx, Rx);
  if (len != lenRx) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_flashErase()': ACK2 timeout, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  
  // check acknowledge
  if (Rx[0]!=ACK) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_flashErase()': ACK2 failure, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }

    
  // print message
  if (verbose) {
    printf("success\n");
    fflush(stdout);
  }
  
  // avoid compiler warnings
  return(0);
  
} // bsl_flashErase



/**
  \fn uint8_t bsl_memWrite(HANDLE ptrPort, uint8_t LINmode, uint8_t verbose, uint32_t addrStart, uint32_t numBytes, char *buf)
   
  \brief upload to microcontroller flash or RAM
   
  \param[in] ptrPort    handle to communication port
  \param[in] LINmode    normal(=0) or reply(=1) mode. Latter ignores LIN echo
  \param[in] verbose    level of verbosity and if application exits on fail 
  \param[in] addrStart  starting address to upload to
  \param[in] numBytes   number of bytes to upload
  \param[in] buf        buffer containing data
  
  \return communication status (0=ok, 1=fail)
  
  upload data to microcontroller memory via WRITE command
*/
uint8_t bsl_memWrite(HANDLE ptrPort, uint8_t LINmode, uint8_t verbose, uint32_t addrStart, uint32_t numBytes, char *buf) {

  int       i, count;
  int       lenTx, lenRx, len;
  char      Tx[1000], Rx[1000];
  uint32_t  addrTmp, addrStep, idx=0;
  uint8_t   chk;


  // print message
  if (verbose) {
    if (numBytes > 2048)
      printf("  upload %1.1fkB of %1.1fkB to 0x%04x ", (float) idx/1024.0, (float) numBytes/1024.0, (int) addrStart);
    else
      printf("  upload %dB of %dB to 0x%04x ", idx, numBytes, (int) addrStart);
    fflush(stdout);
  }

  // init receive buffer
  for (i=0; i<1000; i++)
    Rx[i] = 0;

  // check if port is open
  if (!ptrPort) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_memWrite()': port not open, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }


  // loop over addresses in <=128B steps
  count = 0;
  idx = 0;
  addrStep = 128;
  for (addrTmp=addrStart; addrTmp<addrStart+numBytes; addrTmp+=addrStep) {
  
    // if addr too close to end of range reduce stepsize
    if (addrTmp+128 > addrStart+numBytes)
      addrStep = addrStart+numBytes-addrTmp;

      
    /////
    // send write command
    /////
  
    // construct command
    lenTx = 2;
    Tx[0] = WRITE;
    Tx[1] = (Tx[0] ^ 0xFF);
    lenRx = 1;
  
    // send command
    len = send_port(ptrPort, lenTx, Tx);
    if (len != lenTx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memWrite()': sending command failed, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }

    // for 1-wire line, ignore echo
    if (LINmode) {
      len = receive_port(ptrPort, lenTx, Rx);
      if (len != lenTx) {
        if (verbose) {
          fprintf(stderr, "\n\nerror in 'bsl_memWrite()': echo failed in reply mode, exit!\n\n");
          Exit(1, g_pauseOnExit);
        }
        else {
          return(1);
        }
      }
    } // if (ignoreEcho)
    
    // receive response with timeout
    len = receive_port(ptrPort, lenRx, Rx);
    if (len != lenRx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memWrite()': ACK1 timeout, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }
    
    // check acknowledge
    if (Rx[0]!=ACK) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memWrite()': ACK1 failure, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }

  
    /////
    // send address
    /////
  
    // construct address + checksum (XOR over address)
    lenTx = 5;
    Tx[0] = (char) (addrTmp >> 24);
    Tx[1] = (char) (addrTmp >> 16);
    Tx[2] = (char) (addrTmp >> 8);
    Tx[3] = (char) (addrTmp);
    Tx[4] = (Tx[0] ^ Tx[1] ^ Tx[2] ^ Tx[3]);
    lenRx = 1;
  
    // send command
    len = send_port(ptrPort, lenTx, Tx);
    if (len != lenTx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memWrite()': sending address failed, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }

    // for 1-wire line, ignore echo
    if (LINmode) {
      len = receive_port(ptrPort, lenTx, Rx);
      if (len != lenTx) {
        if (verbose) {
          fprintf(stderr, "\n\nerror in 'bsl_memWrite()': echo failed in reply mode, exit!\n\n");
          Exit(1, g_pauseOnExit);
        }
        else {
          return(1);
        }
      }
    } // if (ignoreEcho)
    
    // receive response with timeout
    len = receive_port(ptrPort, lenRx, Rx);
    if (len != lenRx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memWrite()': ACK2 timeout, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }
    
    // check acknowledge
    if (Rx[0]!=ACK) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memWrite()': ACK2 failure, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }

  
    /////
    // send number of bytes and data
    /////
  
    // construct number of bytes + data + checksum
    lenTx = 0;
    Tx[lenTx++] = addrStep-1;     // -1 from BSL
    chk         = addrStep-1;
    for (i=0; i<addrStep; i++) {
      Tx[lenTx] = buf[idx++];
      chk ^= Tx[lenTx];
      lenTx++;
    }
    Tx[lenTx++] = chk;
    lenRx = 1;

      
    // send command
    len = send_port(ptrPort, lenTx, Tx);
    if (len != lenTx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memWrite()': sending data failed, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }

    // for 1-wire line, ignore echo
    if (LINmode) {
      len = receive_port(ptrPort, lenTx, Rx);
      if (len != lenTx) {
        if (verbose) {
          fprintf(stderr, "\n\nerror in 'bsl_memWrite()': echo failed in reply mode, exit!\n\n");
          Exit(1, g_pauseOnExit);
        }
        else {
          return(1);
        }
      }
    } // if (ignoreEcho)
    
    // receive response with timeout
    len = receive_port(ptrPort, lenRx, Rx);
    if (len != lenRx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memWrite()': ACK3 timeout, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }
    
    // check acknowledge
    if (Rx[0]!=ACK) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_memWrite()': ACK3 failure, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }
    
    // print progress
    if (((idx % 1024) == 0) && (verbose)) {
      if (numBytes > 2048)
        printf("%c  upload %1.1fkB of %1.1fkB to 0x%04x ", '\r', (float) idx/1024.0, (float) numBytes/1024.0, (int) addrStart);
      else
        printf("%c  upload %dB of %dB to 0x%04x ", '\r', idx, numBytes, (int) addrStart);
      fflush(stdout);
    }

  } // loop over address range 
  
  // print message
  if (verbose) {
    if (numBytes > 2048)
      printf("%c  upload %1.1fkB of %1.1fkB to 0x%04x ", '\r', (float) idx/1024.0, (float) numBytes/1024.0, (int) addrStart);
    else
      printf("%c  upload %dB of %dB to 0x%04x ", '\r', idx, numBytes, (int) addrStart);
    printf(" done\n");
    fflush(stdout);
  }
  
  // avoid compiler warnings
  return(0);
  
} // bsl_memWrite



/**
  \fn uint8_t bsl_jumpTo(HANDLE ptrPort, uint8_t LINmode, uint8_t verbose, uint32_t addr)
   
  \brief jump to flash or RAM
   
  \param[in] ptrPort    handle to communication port
  \param[in] LINmode    normal(=0) or reply(=1) mode. Latter ignores LIN echo
  \param[in] verbose    level of verbosity and if application exits on fail 
  \param[in] addr       address to jump to
  
  \return communication status (0=ok, 1=fail)
  
  jump to address and continue code execution. Generally RAM or flash
  starting address
*/
uint8_t bsl_jumpTo(HANDLE ptrPort, uint8_t LINmode, uint8_t verbose, uint32_t addr) {

  int       i;
  int       lenTx, lenRx, len;
  char      Tx[1000], Rx[1000];


  // print message
  if (verbose) {
    printf("  jump to address 0x%04x ... ", (int) addr);
    fflush(stdout);
  }
  
  // init receive buffer
  for (i=0; i<1000; i++)
    Rx[i] = 0;

  // check if port is open
  if (!ptrPort) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_jumpTo()': port not open, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  

  /////
  // send go command
  /////
  
  // construct command
  lenTx = 2;
  Tx[0] = GO;
  Tx[1] = (Tx[0] ^ 0xFF);
  lenRx = 1;
  
  // send command
  len = send_port(ptrPort, lenTx, Tx);
  if (len != lenTx) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_jumpTo()': sending command failed, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }

  // for 1-wire line, ignore echo
  if (LINmode) {
    len = receive_port(ptrPort, lenTx, Rx);
    if (len != lenTx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_jumpTo()': echo failed in reply mode, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }
  } // if (ignoreEcho)
  
  // receive response with timeout
  len = receive_port(ptrPort, lenRx, Rx);
  if (len != lenRx) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_jumpTo()': ACK1 timeout, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  
  // check acknowledge
  if (Rx[0]!=ACK) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_jumpTo()': ACK1 failure, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }

  
  /////
  // send address
  /////

  // construct address + checksum (XOR over address)
  lenTx = 5;
  Tx[0] = (char) (addr >> 24);
  Tx[1] = (char) (addr >> 16);
  Tx[2] = (char) (addr >> 8);
  Tx[3] = (char) (addr);
  Tx[4] = (Tx[0] ^ Tx[1] ^ Tx[2] ^ Tx[3]);
  lenRx = 1;

  // send command
  len = send_port(ptrPort, lenTx, Tx);
  if (len != lenTx) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_jumpTo()': sending address failed, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }

  // for 1-wire line, ignore echo
  if (LINmode) {
    len = receive_port(ptrPort, lenTx, Rx);
    if (len != lenTx) {
      if (verbose) {
        fprintf(stderr, "\n\nerror in 'bsl_jumpTo()': echo failed in reply mode, exit!\n\n");
        Exit(1, g_pauseOnExit);
      }
      else {
        return(1);
      }
    }
  } // if (ignoreEcho)
  
  // receive response with timeout
  len = receive_port(ptrPort, lenRx, Rx);
  if (len != lenRx) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_jumpTo()': ACK2 timeout, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }
  
  // check acknowledge
  if (Rx[0]!=ACK) {
    if (verbose) {
      fprintf(stderr, "\n\nerror in 'bsl_jumpTo()': ACK2 failure, exit!\n\n");
      Exit(1, g_pauseOnExit);
    }
    else {
      return(1);
    }
  }

    
  // print message
  if (verbose) {
    printf("success\n");
    fflush(stdout);
  }
  
  // avoid compiler warnings
  return(0);
  
} // bsl_jumpTo


// end of file
