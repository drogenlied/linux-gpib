#include "board.h"
#include <asm/dma.h>

/*
 *  BDWRT (DMA)
 *  This function performs a single DMA write operation.
 */

IBLCL int bdDMAwrt(ibio_op_t *wrtop)
{
	return bdPIOwrt(wrtop);
}

/*
 *  BDWRT (PIO)
 *  This function performs a single Programmed I/O write operation.
 *  Returns negative value on error, or number of bytes written on success 
 */
IBLCL int bdPIOwrt( ibio_op_t *wrtop)
{
	faddr_t		buf;
	unsigned	cnt;
	extern int eosmodes;
	gpib_char_t data;
	unsigned long flags;
//	int ibcnt = 0; //ibcnt is currently a global variable (ugh) but will be fixed eventually

	DBGin("bdwrt");

	buf = wrtop->io_vbuf;
	cnt = wrtop->io_cnt;
	if(cnt == 0) return 0;

	DBGprint(DBG_DATA, ("buf=0x%p cnt=%d  ", buf, cnt));

	GPIBout(AUXMR, auxrabits);	/* normal handshaking */

	DBGprint(DBG_BRANCH, ("begin PIO loop  "));

	if(wrtop->io_flags & IO_LAST)
	{
		cnt-- ; /* save the last byte for sending EOI */
	}

	if(test_and_set_bit(0, &write_in_progress))
	{
		printk("gpib: bug? write already in progress");
		return -1;
	}

#if DMAOP	// isa dma transfer
	if(ibcnt < cnt)
	{
		/* program dma controller */
		flags = claim_dma_lock();
		disable_dma(ibdma);
		wrtop->io_pbuf = virt_to_bus(wrtop->io_vbuf);
		clear_dma_ff ( ibdma );
		// XXX what if io_cnt is too big?
		set_dma_count( ibdma, cnt );
		set_dma_addr ( ibdma, wrtop->io_pbuf);
		set_dma_mode( ibdma, DMA_MODE_WRITE );
		enable_dma(ibdma);
		release_dma_lock(flags);

		// enable board's dma for output
		imr2_bits |= HR_DMAO;
		GPIBout(IMR2, imr2_bits);

		// enable 'data out' interrupts
		imr1_bits |= HR_DOIE;
		GPIBout(IMR1, imr1_bits);

		// suspend until message is sent
		if(wait_event_interruptible(nec7210_write_wait, test_bit(0, &write_in_progress) == 0))
		{
			printk("gpib write interrupted!\n");
			// XXX
		}

		ibcnt += cnt;

		// disable board's dma
		imr2_bits &= ~HR_DMAO;
		GPIBout(IMR2, imr2_bits);
	}

#else	// PIO transfer

	data.end = 0;
	if(ibcnt < cnt)
	{
		// load message into buffer
		while (ibcnt < cnt)
		{
			data.value = buf[ibcnt];
			if(gpib_buffer_put(write_buffer, data))
			{
				printk("gpib: write buffer full!\n");
				// XXX
				break;
			}
			ibcnt++;
		}

		// enable 'data out' interrupts
		imr1_bits |= HR_DOIE;
		GPIBout(IMR1, imr1_bits);

		// suspend until message is sent
		if(wait_event_interruptible(nec7210_write_wait, test_bit(0, &write_in_progress) == 0))
		{
			printk("gpib write interrupted!\n");
			// XXX
		}
	}

#endif	// DMAOP

	if(wrtop->io_flags & IO_LAST)
	{
		DBGprint(DBG_BRANCH, ("send EOI  "));
		/*send EOI */

		if( eosmodes & XEOS )
		{
			DBGprint(DBG_BRANCH, ("send EOS with EOI  "));
			set_bit(0, &write_in_progress);
			GPIBout(CDOR, buf[ibcnt]);
			ibcnt++;
			wait_event_interruptible(nec7210_write_wait, test_bit(0, &write_in_progress) == 0);
			set_bit(0, &write_in_progress);
			bdSendAuxCmd(AUX_SEOI);
			GPIBout(CDOR, bdGetEOS() );
		} else {
			DBGprint(DBG_BRANCH, ("send EOI with last byte "));
			set_bit(0, &write_in_progress);
			bdSendAuxCmd(AUX_SEOI);
			GPIBout(CDOR, buf[ibcnt]);
			ibcnt++;
		}
		wait_event_interruptible(nec7210_write_wait, test_bit(0, &write_in_progress) == 0);
	}
	// disable 'data out' interrupts
	imr1_bits &= ~HR_DOIE;
	GPIBout(IMR1, imr1_bits);

	DBGprint(DBG_BRANCH, ("done  "));

	if (!noTimo) {
		DBGprint(DBG_BRANCH, ("timeout  "));
		ibsta |= (ERR | TIMO);
		iberr = EABO;
	}

	DBGout();

	return ibcnt;
}













