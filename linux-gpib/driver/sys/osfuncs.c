/***************************************************************************
                               sys/osfuncs.c
                             -------------------

    begin                : Dec 2001
    copyright            : (C) 2001, 2002 by Frank Mori Hess
    email                : fmhess@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <ibsys.h>

#include <linux/fcntl.h>

#define GIVE_UP(a) {up(&inode_mutex); return a;}

int ib_opened=0;
int ib_exclusive=0;

DECLARE_MUTEX(inode_mutex);

IBLCL int ibopen(struct inode *inode, struct file *filep)
{
	if( ib_exclusive )
	{
		return (-EBUSY);
	}


	if ( filep->f_flags & O_EXCL )
	{
		if (ib_opened)
		{
			return (-EBUSY);
		}
		ib_exclusive=1;
	}

	ib_opened++;

	return 0;
}


IBLCL int ibclose(struct inode *inode, struct file *file)
{
	if ((pgmstat & PS_ONLINE) && ib_opened == 1 )
		ibonl(0);
	ib_opened--;

	if( ib_exclusive )
		ib_exclusive = 0;

	return 0;
}

IBLCL int ibioctl(struct inode *inode, struct file *filep, unsigned int cmd, unsigned long arg)
{
	int	retval = 0; 		/* assume everything OK for now */
	ibarg_t m_ibarg,*ibargp;

	int	bufsize;
	int	remain;
	char 	*buf;
	char 	*userbuf;
	char 	c;
	ssize_t ret;
	int end_flag = 0;

	ibargp = (ibarg_t *) &m_ibarg;

	/* Check the arg buffer is readable & writable by the current process */
	retval = verify_area(VERIFY_WRITE, (void *)arg, sizeof(ibarg_t));
	if (retval)
	{
		return (retval);
	}

	retval = verify_area(VERIFY_READ, (void *)arg, sizeof(ibarg_t));
	if (retval)
	{
		return (retval);
	}

	copy_from_user( ibargp , (ibarg_t *) arg , sizeof(ibarg_t));

	ibargp->ib_iberr = EDVR;

// XXX
#if 0
	if( cmd == IBAPWAIT )
	{
		/* special case for IBAPWAIT : does his own locking */
		ibAPWait(ibargp->ib_arg);
		ibargp->ib_ibsta = ibsta;
		ibargp->ib_iberr = iberr;
		ibargp->ib_ibcnt = ibcnt;
		copy_to_user((ibarg_t *) arg, (ibarg_t *) ibargp , sizeof(ibarg_t));

		DBGout();
		return retval;
	}
#endif

	/* lock other processes from performing commands */
	retval = down_interruptible(&inode_mutex);
	if(retval)
	{
		printk("gpib: ioctl interrupted while waiting on lock\n");
		return -ERESTARTSYS;
	}

//XXX a lot of the content of this switch should be split out into seperate functions
	switch (cmd)
	{
		case IBRD:	// XXX read should not be an ioctl
			/* Check write access to buffer */
			retval = verify_area(VERIFY_WRITE, ibargp->ib_buf, ibargp->ib_cnt);
			if (retval)
				GIVE_UP (retval);

			/* Get a DMA buffer */
			bufsize = ibargp->ib_cnt;
			if ((buf = osGetDMABuffer( &bufsize )) == NULL)
			{
				GIVE_UP( -ENOMEM ) ;
			}
			/* Read DMA buffer loads till we fill the user supplied buffer */
			userbuf = ibargp->ib_buf;
			remain = ibargp->ib_cnt;
			do
			{
				ret = ibrd( buf, (bufsize < remain) ? bufsize : remain, &end_flag);
				if(ret < 0)
				{
					retval = -EIO;
					break;
				}
				copy_to_user( userbuf, buf, ret );
				remain -= ret;
				userbuf += ret;
			}while (remain > 0 && end_flag == 0);
			ibargp->ib_ibcnt = ibargp->ib_cnt - remain;
			/* Free the DMA buffer */
			osFreeDMABuffer( buf );
			break;
		case IBWRT:	// XXX write should not be an ioclt

			/* Check read access to buffer */
			retval = verify_area(VERIFY_READ, ibargp->ib_buf, ibargp->ib_cnt);
			if (retval)
				GIVE_UP(retval);
			/* Get a DMA buffer */
			bufsize = ibargp->ib_cnt;
			if ((buf = osGetDMABuffer( &bufsize )) == NULL)
			{
				GIVE_UP(-ENOMEM);
			}
			/* Write DMA buffer loads till we empty the user supplied buffer */
			userbuf = ibargp->ib_buf;
			remain = ibargp->ib_cnt;
			do
			{
				copy_from_user( buf, userbuf, (bufsize < remain) ? bufsize : remain );
				ret = ibwrt( buf, (bufsize < remain) ? bufsize : remain, (bufsize < remain) );
				if(ret < 0)
				{
					retval = -EIO;
					break;
				}
				remain -= ret;
				userbuf += ret;
			}while (remain > 0);
			ibargp->ib_ibcnt = ibargp->ib_cnt - remain;
			/* Free the DMA buffer */
			osFreeDMABuffer( buf );
			break;
		case IBCMD:
			/* Check read access to buffer */
			retval = verify_area(VERIFY_READ, ibargp->ib_buf, ibargp->ib_cnt);
			if (retval)
				GIVE_UP(retval);

			/* Get a DMA buffer */
			bufsize = ibargp->ib_cnt;
			if ((buf = osGetDMABuffer( &bufsize )) == NULL) {
				GIVE_UP(-ENOMEM);
			}

			/* Write DMA buffer loads till we empty the user supplied buffer */
			userbuf = ibargp->ib_buf;
			remain = ibargp->ib_cnt;
			do
			{
				copy_from_user( buf, userbuf, (bufsize < remain) ? bufsize : remain );
				ret = ibcmd( buf, (bufsize < remain) ? bufsize : remain );
				if(ret < 0)
				{
					retval = -EIO;
					break;
				}
				remain -= ret;
				userbuf += ret;
			} while (remain > 0 && !(driver->update_status(driver) & (TIMO)));
			ibargp->ib_ibcnt = ibargp->ib_cnt - remain;

			/* Free the DMA buffer */
			osFreeDMABuffer( buf );

			break;

		case IBWAIT:
			DBGprint(DBG_DATA,("**arg=%x",ibargp->ib_arg));
			retval = ibwait(ibargp->ib_arg);
			break;
		case IBRPP:
			/* Check write access to Poll byte */
			retval = verify_area(VERIFY_WRITE, ibargp->ib_buf, 1);
			if (retval)
				GIVE_UP(retval);

			retval = ibrpp(&c);
			put_user( c, ibargp->ib_buf );
			break;
		case IBONL:
			retval = ibonl(ibargp->ib_arg);
			break;
		case IBAPE:
			ibAPE(ibargp->ib_arg,ibargp->ib_cnt);
			break;
		case IBSIC:
			retval = ibsic();
			break;
		case IBSRE:
			retval = ibsre(ibargp->ib_arg);
			break;
		case IBGTS:
			retval = ibgts();
			break;
		case IBCAC:
			retval = ibcac(ibargp->ib_arg);
			break;
		case IBSDBG:
	#if DEBUG
			dbgMask= ibargp->ib_arg | DBG_1PPL;
			DBGprint(DBG_DATA,("dbgMask=0x%x",dbgMask));
	#endif
			break;
		case IBLINES:
			retval = iblines(&ibargp->ib_ret);
			break;
		case IBPAD:
			retval = ibpad(ibargp->ib_arg);
			break;
		case IBSAD:
			retval = ibsad(ibargp->ib_arg);
			break;
		case IBTMO:
			retval = ibtmo(ibargp->ib_arg);
			break;
		case IBEOT:
			retval = ibeot(ibargp->ib_arg);
			break;
		case IBEOS:
			retval = ibeos(ibargp->ib_arg);
			break;
		case IBRSV:
			retval = ibrsv(ibargp->ib_arg);
			break;
		case DVTRG:
			retval = dvtrg(ibargp->ib_arg);
			break;
		case DVCLR:
			retval = dvclr(ibargp->ib_arg);
			break;
		case DVRSP:
			/* Check write access to Poll byte */
			retval = verify_area(VERIFY_WRITE, ibargp->ib_buf, 1);
			if (retval)
			{
				GIVE_UP(retval);
			}
			retval = dvrsp(ibargp->ib_arg, &c);

			put_user( c, ibargp->ib_buf );

			break;
		case IBAPRSP:
			retval = verify_area(VERIFY_WRITE, ibargp->ib_buf, 1);
			if (retval)
			{
				GIVE_UP(retval);
			}
			retval = ibAPrsp(ibargp->ib_arg, &c);
			put_user( c, ibargp->ib_buf );
			break;
		case DVRD:	// XXX unnecessary, should be in user space lib
			/* Check write access to buffer */
			retval = verify_area(VERIFY_WRITE, ibargp->ib_buf, ibargp->ib_cnt);
			if (retval)
				GIVE_UP(retval);

			/* Get a DMA buffer */
			bufsize = ibargp->ib_cnt;
			if ((buf = osGetDMABuffer( &bufsize )) == NULL)
			{
				GIVE_UP(-ENOMEM);
			}
			if(receive_setup(ibargp->ib_arg))
			{
				retval = -EIO;
				break;
			}
			/* Read DMA buffer loads till we fill the user supplied buffer */
			userbuf = ibargp->ib_buf;
			remain = ibargp->ib_cnt;
			do
			{
				ret = ibrd(buf, (bufsize < remain) ? bufsize : remain, &end_flag);
				if(ret < 0)
				{
					retval = -EIO;
					break;
				}
				copy_to_user( userbuf, buf, ret );
				remain -= ret;
				userbuf += ret;
			}while (remain > 0  && end_flag == 0);	//!(driver->update_status(driver) & TIMO));
			ibargp->ib_ibcnt = ibargp->ib_cnt - remain;
			/* Free the DMA buffer */
			osFreeDMABuffer( buf );
			break;
		case DVWRT:	// XXX unnecessary, should be in user space lib

			/* Check read access to buffer */
			retval = verify_area(VERIFY_READ, ibargp->ib_buf, ibargp->ib_cnt);
			if (retval)
				GIVE_UP(retval);

			/* Get a DMA buffer */
			bufsize = ibargp->ib_cnt;
			if ((buf = osGetDMABuffer( &bufsize )) == NULL) {
				GIVE_UP(-ENOMEM);
			}

			/* Write DMA buffer loads till we empty the user supplied buffer */
			userbuf = ibargp->ib_buf;
			remain = ibargp->ib_cnt;
			do {
				copy_from_user( buf, userbuf, (bufsize < remain) ? bufsize : remain );
				ret = dvwrt( ibargp->ib_arg, buf, (bufsize < remain) ? bufsize : remain );
				if(ret < 0)
				{
					retval = -EIO;
					break;
				}
				remain -= ret;
				userbuf += ret;
			} while (remain > 0  && !(driver->update_status(driver) & (TIMO)));
			ibargp->ib_ibcnt = ibargp->ib_cnt - remain;

			/* Free the DMA buffer */
			osFreeDMABuffer( buf );

			break;

		/* special configuration options */
		case CFCBASE:
			osChngBase(ibargp->ib_arg);
			break;
		case CFCIRQ:
			osChngIRQ(ibargp->ib_arg);
			break;
		case CFCDMA:
			osChngDMA(ibargp->ib_arg);
			break;
		case CFCDMABUFFER:
			if (ibargp->ib_arg > MAX_DMA_SIZE)
			{
				GIVE_UP(-EINVAL);
			}
			if ( ibargp->ib_arg > gpib_dma_size )
			{
				gpib_dma_size = ibargp->ib_arg;
				osMemInit();
				printk("-- DMA Buffer now %d Bytes\n",gpib_dma_size);
			}else
			{
				printk("-- DMA Buffer Not Changed \n");
			}
			break;

		default:
			retval = -ENOTTY;
			break;
	}

	// return status bits
	ibargp->ib_ibsta &= ~DRIVERBITS;
	ibargp->ib_ibsta |= driver->update_status(driver) & DRIVERBITS;
	if(retval)
		ibargp->ib_ibsta |= ERR;
	else
		ibargp->ib_ibsta &= ~ERR;
	if(end_flag)
		ibargp->ib_ibsta |= END;
	else
		ibargp->ib_ibsta &= ~END;
	// XXX io is always complete since we don't support asynchronous transfers yet
	ibargp->ib_ibsta |= CMPL;

	copy_to_user((ibarg_t *) arg, (ibarg_t *) ibargp , sizeof(ibarg_t));

	GIVE_UP(retval);
}






































