#include "adc.h"
#include "mongoose.h"

#ifndef ADC_FUNCTION_DEFINITIONS
#define ADC_FUNCTION_DEFINITIONS

/* Open /dev/mem to give access to physical addresses */
int open_physical (int fd)
{
    if (fd == -1) // check if already open
        if ((fd = open( "/dev/mem", (O_RDWR | O_SYNC))) == -1)
        {
            printf ("ERROR: could not open \"/dev/mem\"...\n");
            return (-1);
        }
    return fd;
}

/* Close /dev/mem to give access to physical addresses */
void close_physical (int fd)
{
    close (fd);
}

/* Establish a virtual address mapping for the physical addresses starting
* at base and extending by span bytes */
void* map_physical(int fd, unsigned int base, unsigned int span)
{
    void *virtual_base;
    // Get a mapping from physical addresses to virtual addresses
    virtual_base = mmap (NULL, span, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, base);
    if (virtual_base == MAP_FAILED)
    {
        printf ("ERROR: mmap() failed...\n");
        close (fd);
        return (NULL);
    }
    return virtual_base;
}

/* Close the previously-opened virtual address mapping */
int unmap_physical(void * virtual_base, unsigned int span)
{
    if (munmap (virtual_base, span) != 0)
    {
        printf ("ERROR: munmap() failed...\n");
        return (-1);
    }
    return 0;
}

char* split_string(char* strPtr, char delim[], int word_pos) {
	
	char *ptr = strPtr;
	char * pointer_to_return = NULL;
	pointer_to_return = strtok(ptr, delim);

	if (word_pos == 0){
		return pointer_to_return;  
	}

	int i = 0;

	for(i = 0; i < word_pos && pointer_to_return != NULL; i++){
		pointer_to_return = strtok(NULL, delim);
	}

	return pointer_to_return;
}

#endif