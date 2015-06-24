/*----------------------------------------------------------
    INCLUDE FILES
----------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include "stm8as.h"
#include "flash.h"
#define _MAIN_
  #include "globals.h"
#undef _MAIN_


/////////////////
//	main routine
/////////////////
void main (void) {
  
  uint32_t    i, period;
  

  // if required, activate ROM-bootloader
  if ((*((uint8_t*) OPT17) != 0x55) || (*((uint8_t*) NOPT17)  != 0xAA)) {
    flash_write_option_byte(OPT17,  0x55);
    flash_write_option_byte(NOPT17, 0xAA);
    //WWDG_CR.byte = 0xBF;                      // trigger SW reset
    period = 100000;                            // if BSL activated blink fast
  }
  else  
    period = 500000;                            // if BSL already active blink slow
  
  
  /////////////////
  //	init peripherals
  /////////////////
  
  // disable interrupts
  DISABLE_INTERRUPTS;

  // switch to 16MHz (default is 2MHz)
  CLK_CKDIVR.byte = 0x00;  
  
  // init LED pin (STM8 Discovery and muBoard) 
#if defined(STM8S105)
  PD.ODR.bit.b0 = 1;   // init output
  PD.DDR.bit.b0 = 1;   // input(=0) or output(=1)
  PD.CR1.bit.b0 = 1;   // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
  PD.CR2.bit.b0 = 1;   // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope
#elif defined(STM8S207)
  PH.ODR.bit.b2 = 1;   // init output
  PH.DDR.bit.b2 = 1;   // input(=0) or output(=1)
  PH.CR1.bit.b2 = 1;   // input: 0=float, 1=pull-up; output: 0=open-drain, 1=push-pull
  PH.CR2.bit.b2 = 1;   // input: 0=no exint, 1=exint; output: 0=2MHz slope, 1=10MHz slope
#endif
      

  /////////////////
  //	main loop
  /////////////////
  while (1) {
    
    // blink LED
#if defined(STM8S105)
    PD.ODR.bit.b0 ^= 1;
#elif defined(STM8S207)
    PH.ODR.bit.b2 ^= 1;
#endif

    // wait a bit
    for (i=0; i<period; i++)
      _NOP_;
    
  } // main loop

} // main


/*-----------------------------------------------------------------------------
    END OF MODULE
-----------------------------------------------------------------------------*/
