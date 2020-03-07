#ifndef ADC_HEADERS
#define ADC_HEADERS

// define 10 channels for ADC acquisition
// first ADC
#define ADC_V1 0 
#define ADC_V2 1 
#define ADC_V3 2 
#define ADC_V4 3 
#define ADC_V5 4 
#define ADC_V6 5 
#define ADC_RA 6 
#define ADC_LA 7 

// second ADC
#define ADC_LL 0
#define ADC_WT 1
#define ADC_BT 2

#define DEFAULT_TIMER_PERIOD 0.004

#include "address_map_arm.h"    //  include tha hardware address map

#include <sys/mman.h>
#include <sys/shm.h> 
#include <sys/ipc.h> 


/* Prototypes for functions used to access physical memory addresses */
int open_physical (int);
void * map_physical (int, unsigned int, unsigned int);
void close_physical (int);
int unmap_physical (void *, unsigned int);
char* split_string(char* strPtr, char delim[], int word_pos);

#define ADC_VALUE_MASK 0xFFF

#endif