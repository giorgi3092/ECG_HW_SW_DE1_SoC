#declaring variables
CC = gcc
DEBUG = YES
GDBFLAG = -ggdb3
CIFLAGS = -Wall -c
CFFLAGS = -Wall
PROGRAM = ECG
OBJS = ECG.o mongoose.o adc.o

# update the flags
ifeq ($(DEBUG),YES)
	CIFLAGS += $(GDBFLAG)
	CFFLAGS += $(GDBFLAG)
endif

ECG: ECG.o mongoose.o adc.o
	$(CC) $(CFFLAGS) $(OBJS) -o $(PROGRAM)

ECG.o: ECG.c
	$(CC) $(CIFLAGS) ECG.c

mongoose.o: mongoose.c mongoose.h
	$(CC) $(CIFLAGS) mongoose.c

adc.o: adc.c adc.h address_map_arm.h
	$(CC) $(CIFLAGS) adc.c

clean:
	rm *.o ECG