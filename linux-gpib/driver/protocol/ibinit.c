
#include <ibprot.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>

int timeidx	= T1s;	/* timeout index into timeTable */
int pollTimeidx = T100ms;	/* timeidx for serial and parallel polls */
int myPAD = 0;		/* current primary address */
int mySAD = 0;		/* current secondary address */

#if !defined(HP82335) && !defined(TMS9914)
#else
int ccrbits	= 0;	/* static bits for AUXRA (EOS modes) */
#endif

// store number of jiffies to wait for various timeouts
#define usec_to_jiffies(usec) (((usec) + 1000000 / HZ - 1) / (1000000 / HZ))
unsigned long timeTable[] =
{
	0,                      	/*  0: TNONE    */
	usec_to_jiffies(10),        	/*  1: T10us    */
	usec_to_jiffies(30),        	/*  2: T30us    */
	usec_to_jiffies(100),       	/*  3: T100us   */
	usec_to_jiffies(300),       	/*  4: T300us   */
	usec_to_jiffies(1000),             	/*  5: T1ms     */
	usec_to_jiffies(3000),             	/*  6: T3ms     */
	usec_to_jiffies(10000),            	/*  7: T10ms    */
	usec_to_jiffies(30000),            	/*  8: T30ms    */
	usec_to_jiffies(100000),           	/*  9: T100ms   */
	usec_to_jiffies(300000),           	/* 10: T300ms   */
	usec_to_jiffies(1000000),                	/* 11: T1s      */
	usec_to_jiffies(3000000),                	/* 12: T3s      */
	usec_to_jiffies(10000000),               	/* 13: T10s     */
	usec_to_jiffies(30000000),               	/* 14: T30s     */
	usec_to_jiffies(100000000),              	/* 15: T100s    */
	usec_to_jiffies(300000000),              	/* 16: T300s    */
	usec_to_jiffies(1000000000),              	/* 17: T1000s   */
};




/*
* IBONL
* Initialize the interface hardware.  If v is non-zero then
* the GPIB chip is enabled online.  If v is zero then it is
* left disabled and offline.
*
* NOTE:
*      1.  Ibonl must be called before any other function.
*/

extern int ib_opened;

int drvstat = 0;

int ibonl(gpib_board_t *board, int v)
{
	/*
	* ibonl must be called first time a process is entering the driver
	* if one process is working on the driver ibonl is dummied
	*
	*/
	if( ib_opened == 1 || !(board->online) )
	{
		timeidx = T1s; /* initialize configuration variables... */
		myPAD = 0;
		mySAD = 0;
#if !defined(HP82335) && !defined(TMS9914)
#else
		ccrbits = 0;
#endif
	}

	if (v)
	{
		if( (ib_opened <= 1) && !(board->online))
		{
			board->buffer_length = 0x1000;
			if(board->buffer)
				vfree(board->buffer);
			board->buffer = vmalloc(board->buffer_length);
			if(board->buffer == NULL)
				return -ENOMEM;

			if(board->interface->attach(board) < 0)
			{
				board->interface->detach(board);
				printk("GPIB Hardware Error! (Chip type not found or wrong Base Address?)\n");
				return -1;
			}
		}
		/* initialize system support functions */
		board->online = 1;
		if(board->master)
			ibsic(board);
	}else
	{		/* OFFLINE: leave SYSFAIL red */
		if( ib_opened <= 1)
		{
			board->interface->detach(board);
			if(board->buffer)
			{
				vfree(board->buffer);
				board->buffer = NULL;
			}
			board->buffer_length = 0;
			board->online = 0;
		}
	}
	return 0;
}

