#declaring variables
CC = gcc
DEBUG = YES
GDBFLAG = -ggdb3
CIFLAGS = -Wall -c
CFFLAGS = -Wall
PROGRAM = ECG
OBJS = ECG.o mongoose.o

# update the flags
ifeq ($(DEBUG),YES)
	CIFLAGS += $(GDBFLAG)
	CFFLAGS += $(GDBFLAG)
endif

output: ECG.o mongoose.o
	$(CC) $(CFFLAGS) $(OBJS) -o $(PROGRAM)

ECG.o: ECG.c
	$(CC) $(CIFLAGS) ECG.c

mongoose.o: mongoose.c mongoose.h
	$(CC) $(CIFLAGS) mongoose.c

clean:
	rm *.o ECG