# Project: STM8_serial_flasher

CC            = gcc
CFLAGS        = -c -Wall -I./STM8_Routines
#LDFLAGS       = -g3 -lcurses -lm
LDFLAGS       = -g3 -lm
SOURCES       = bootloader.c hexfile.c main.c misc.c serial_comm.c
INCLUDES      = globals.h misc.h bootloader.h hexfile.h serial_comm.h main.h
STM8FLASH     = STM8_Routines/E_W_ROUTINEs_128K_ver_2.1.s19 STM8_Routines/E_W_ROUTINEs_128K_ver_2.0.s19 STM8_Routines/E_W_ROUTINEs_256K_ver_1.0.s19 STM8_Routines/E_W_ROUTINEs_32K_ver_1.3.s19 STM8_Routines/E_W_ROUTINEs_128K_ver_2.1.s19 STM8_Routines/E_W_ROUTINEs_32K_ver_1.4.s19 STM8_Routines/E_W_ROUTINEs_128K_ver_2.2.s19 STM8_Routines/E_W_ROUTINEs_32K_ver_1.0.s19 STM8_Routines/E_W_ROUTINEs_128K_ver_2.4.s19 STM8_Routines/E_W_ROUTINEs_32K_ver_1.2.s19
STM8INCLUDES  = $(STM8FLASH:.s19=.h)
OBJECTS       = $(SOURCES:.c=.o)
BIN           = STM8_serial_flasher
RM            = rm -f

.PHONY: all all-before all-after clean clean-custom

all: $(STM8INCLUDES) $(SOURCES) $(BIN)
	
clean:
	${RM} $(OBJECTS) $(BIN) $(BIN).exe *~ .DS_Store 
	
%.h: %.s19 $(STM8FLASH)
	xxd -i $< > $@
	  
$(BIN): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
