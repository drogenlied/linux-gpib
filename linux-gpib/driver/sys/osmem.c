#include <ibsys.h>

/*
 * Memory Management routines
 *
 * To avoid memory fragmentation a Buffer of desired size can be preallocated
 * at module loading time or with the CFCDMABUFFER call
 * this buffer will be used for I/O and if it too small the transfer will
 * be splitted into multiple chunks
 *
 * Note:
 * On kernels < 1.3.x frequent calls to kmalloc(..,GFP_DMA) will fail 
 * so on this kernels this approach can only be used with less than 16Mb
 * main Memory
 *
 */

#if defined(USE_DMA)
#define GFP_DMA_DEFINED
#define MEMORY_PRIORITY (GFP_KERNEL | GFP_DMA)
#else
#define MEMORY_PRIORITY (GFP_KERNEL)
#endif

#ifndef DEFAULT_GPIB_DMA_SIZE
#define DEFAULT_GPIB_DMA_SIZE 0
#endif





/* Pointers for the gpib dma buffers */
static char *gpib_dma_buffer = NULL;
 int   gpib_dma_size = DEFAULT_GPIB_DMA_SIZE;
 int   gpib_default_dma_size = 0;


IBLCL void osMemInit(void) 
{

  DBGin("osMemInit");

  if( !gpib_dma_size)
    gpib_dma_size = gpib_default_dma_size;


  if (gpib_dma_buffer)   /* release any preallocated buffer */
    osFreeDMABuffer( gpib_dma_buffer );


  if (gpib_dma_size) {
    /* Pre-allocate a DMA buffer for I/O */
    gpib_dma_buffer = osGetDMABuffer( &gpib_dma_size );
    if (gpib_dma_buffer == NULL) {
      printk( "-- Unable to pre-allocate %d Bytes for DMA buffer",
	      gpib_dma_size );
      printk( " - trying to use dynamic buffer allocation\n" );
    } else {
      printk( "-- Pre-allocated %d Byte buffer for DMA\n",
	      gpib_dma_size  );
    }
  }
  else {
    printk( "-- using dynamic buffer allocation\n" );
  }

  DBGout();
}

IBLCL void osMemRelease(void) 
{
  DBGin("osMemRelease");

	  if (gpib_dma_buffer) {
	    osFreeDMABuffer( gpib_dma_buffer );
	    gpib_dma_buffer = NULL;
	  }

  DBGout();
}

/* Return a pointer to a DMA buffer */
IBLCL char *osGetDMABuffer( int *size )
{
	char *buf;
	DBGin("osGetDMABuffer");
	if (gpib_dma_buffer) {
		/* Use a pre-allocated DMA buffer */
		buf = gpib_dma_buffer;
		*size = gpib_dma_size;
	} else {
		/* No pre-allocated buffer - allocate one dynamically */
		if (*size > MAX_DMA_SIZE)
			*size = MAX_DMA_SIZE;

		buf = (char *)kmalloc( *size, MEMORY_PRIORITY );


	}
        DBGout();
	return buf;
}

/* Free a DMA buffer if it was dynamically allocated */
IBLCL void osFreeDMABuffer( char *buf )
{
        DBGin("osFreeDMABuffer");
	if (!gpib_dma_buffer) {
		kfree( buf );
	}
        DBGout();

}


