#declaring variables
CC = gcc
DEBUG = YES
GDBFLAG = -ggdb3
CIFLAGS = -Wall -c
CFFLAGS = -Wall
PROGRAM = ECG
OBJS = ECG.o mongoose.o adc.o dll.o

# update the flags
ifeq ($(DEBUG),YES)
	CIFLAGS += $(GDBFLAG)
	CFFLAGS += $(GDBFLAG)
endif

ECG: $(OBJS)
	$(CC) $(CFFLAGS) $(OBJS) -o $(PROGRAM)

ECG.o: ECG.c
	$(CC) $(CIFLAGS) ECG.c

mongoose.o: mongoose.c mongoose.h
	$(CC) $(CIFLAGS) mongoose.c

adc.o: adc.c adc.h address_map_arm.h
	$(CC) $(CIFLAGS) adc.c

dll.o: dll.c dll.h
	$(CC) $(CIFLAGS) dll.c

clean:
	rm *.o ECG ECG.csv