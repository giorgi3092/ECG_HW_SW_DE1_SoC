#include "address_map_arm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/mman.h>
#include <sys/time.h> 
#include <stdbool.h>

/*******************************************************************************
 * This program demonstrates use of the ADC port.
 *
 * It performs the following:
 * 1. Performs a conversion operation on all eight channels.
 * 2. Displays the converted values on the terminal window.
*******************************************************************************/
int fd;                     // used to open /dev/mem
void *h2p_lw_virtual_base;          // the light weight buss base
volatile unsigned int * ADC_ptr = NULL;    // virtual address pointer to read ADC


// measure time
struct timeval t1 = (struct timeval){0};
struct timeval t2 = (struct timeval){0};
double elapsedTime = 0.0;
double elapsed_500 = 0.0;
bool samples_500 = false;
double sampling_rate = 0.0;

int main(void) {
    
    // Open /dev/mem
	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) 	{
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
    }

    // get virtual addr that maps to physical
	h2p_lw_virtual_base = mmap( NULL, LW_BRIDGE_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, LW_BRIDGE_BASE );	
	if( h2p_lw_virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap1() failed...\n" );
		close( fd );
		return(1);
	}

    // Set virtual address pointer to I/O port
    ADC_ptr = (unsigned int *) (h2p_lw_virtual_base + ADC_BASE);

    volatile int delay_count; // volatile so that the C compiler doesn't remove the loop
    int port, value;

    unsigned int num_samples_by_all = 0;       // sampling all 8 channels counts as one sample
    unsigned int num_samples_by_single = 0;       // sampling all 8 channels counts as one sample

    printf("\033[2J"); // erase terminal window
    printf("\033[H");  // move cursor to 0,0 position

    *(ADC_ptr + 1) = 1; // Sets the ADC up to automatically perform conversions.

    // start timer for the first time
    gettimeofday(&t1, NULL); 
    while (1) {
        if ((*ADC_ptr) & 0x8000) // check the refresh bit (bit 15) of port 1 to
                                 // check for update
        {
            printf("\033[H");
            //printf("\033[2J"); // sets the cursor to the top-left of the terminal
                              // window
            for (port = 0; port < 8; ++port) {
                value = (*(ADC_ptr + port)) & 0xFFF;
                printf("ADC port %d: %1.4f Volts\n", port, value/1000.0);
                num_samples_by_single++;    // one sample taken
            }


            /****** statistics computations ******/
            num_samples_by_all++;       // 8 samples taken

            // time
            gettimeofday(&t2, NULL);
            elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
            elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
            elapsed_500 += elapsedTime;
            gettimeofday(&t1, NULL);

            printf("\n\n");
            printf("************ stats *************\n");

            printf("individual samples: %09u X8 samples\n", num_samples_by_single);
            printf("individual samples: %09u samples\n\n", num_samples_by_all);

            printf("period of sample: %4.5lf ms\n", elapsedTime);
            samples_500 = num_samples_by_all%500==0;    // check if consecutive 500 samples were taken
            sampling_rate = samples_500 ? (double) 500/elapsed_500*1000 : sampling_rate;
            printf("sampling rate (all 8 channels): %6.0lf samples/sec\n", sampling_rate);
            elapsed_500 = samples_500 ? 0.0 : elapsed_500;
        }

        //for (delay_count = 1500000; delay_count != 0; --delay_count); // delay loop
    }

    return 0;
}
