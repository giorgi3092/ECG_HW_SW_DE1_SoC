#include "mongoose.h"           //  include Mongoose API definitions
#include "adc.h"                //  include function prototypes for ADC control
#include "adc.c"                //  include function prototypes for ADC control

int fd = -1;                                // used to open /dev/mem
void *h2f_lw_virtual_base;                  // the light weight buss base
volatile int * ADC_ptr = NULL;     // virtual address pointer to read ADC

int main(void) {

    int i, Value;

    /************ ADC mapping / setup ************/
    // Create virtual memory access to the FPGA light-weight bridge
    if ((fd = open_physical (fd)) == -1) {
        printf( "ERROR: could not open \"/dev/mem\"...\n" );
        return (-1);
    }

    if (!(h2f_lw_virtual_base = map_physical (fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN))) {
            printf( "ERROR: mmap1() failed...\n" );
            close( fd );
        return (-1);
    }

    // Set virtual address pointer to I/O port
    ADC_ptr = (int *) (h2f_lw_virtual_base + ADC_BASE);

    while(1) {
        // set measure number for ADC convert
        *(ADC_ptr+1) = 10;

        // start measure
        *ADC_ptr = 0;
        *ADC_ptr = 1;
        *ADC_ptr = 0;
        usleep(1);

        // wait measure done
        while ((*ADC_ptr & 0x01) == 0x00);

		// read adc value
		for(i=0;i<10;i++){
			Value = *(ADC_ptr+1);
			printf("%.3fV (0x%04x)\r\n", (float)Value/1000.0, Value);
		}
        
    }

    return 0;
}