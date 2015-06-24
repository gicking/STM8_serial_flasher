#######################
# SDCC Makefile for making a hexfile from all .C files in this directory.
# Output files are generated in a directory with name of device.
#######################

DEVICE=STM8S105
#DEVICE=STM8S207

# define compiler (has to be in PATH)
CC = sdcc

# define output path, compiler/linker options etc.
CFLAGS = -mstm8 --std-sdcc99 -D$(DEVICE)
LFLAGS = -mstm8 -lstm8 $(OPTIMIZE) --out-fmt-ihx
OBJDIR = $(DEVICE)
OBJECTS = $(patsubst %.c, $(OBJDIR)/%.rel, $(wildcard *.c))
HEADERS = $(wildcard *.h)

# set defaults 
TARGET=$(OBJDIR)/main.ihx

.PHONY: clean all default objects

.PRECIOUS: $(TARGET) $(OBJECTS)

default: $(TARGET)

all: default

# create output folder
$(OBJDIR):
	mkdir -p $(OBJDIR)

# compile all *c files
$(OBJDIR)/%.rel: %.c $(HEADERS) $(OBJDIR)
	$(CC) $(CFLAGS) -I./$(MULIB) -c $< -o $@

# link all object files and libaries
$(TARGET): $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o $@
	
# clean up
clean:
	rm -fr .DS_Store
	rm -fr $(OBJDIR)
	rm -fr Release
	rm -fr Debug

# upload via stm8flash / SWIM (https://github.com/vdudouyt/stm8flash)
# stm8flash requires device in lower-case -> http://gnu-make.2324884.n4.nabble.com/how-to-achieve-conversion-to-uppercase-td14496.html 
swim:
	stm8flash -c stlink -w $(TARGET) -p $(shell echo $(DEVICE) | tr A-Z a-z)

serial:
ifeq ($(DEVICE),STM8S105)
	../STM8_serial_flasher -p /dev/ttyAMA0 -f $(TARGET) -q -Q -r -u 2
else
	../STM8_serial_flasher -p /dev/ttyUSB0 -f $(TARGET) -q
endif	

