#ifndef ADC_HEADERS
#define ADC_HEADERS

#include "address_map_arm.h"    //  include tha hardware address map

#include <sys/mman.h>
#include <sys/shm.h> 
#include <sys/ipc.h> 


/* Prototypes for functions used to access physical memory addresses */
int open_physical (int);
void * map_physical (int, unsigned int, unsigned int);
void close_physical (int);
int unmap_physical (void *, unsigned int);

#define ADC_VALUE_MASK 0xFFF

#endif