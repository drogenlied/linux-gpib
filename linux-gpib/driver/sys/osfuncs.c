#include <ibsys.h>

#include <linux/fcntl.h>

#define GIVE_UP(a) { osUnlockMutex(); DBGout(); return (a); }

int ib_opened=0;
int ib_exclusive=0;


IBLCL int ibopen(struct inode *inode, struct file *filep)
{
	MOD_INC_USE_COUNT;
	DBGin("ibopen");


	if( ib_exclusive )
	{
		DBGout();
		MOD_DEC_USE_COUNT;
		return (-EBUSY);
	}


	if ( filep->f_flags & O_EXCL )
	{
		if (ib_opened)
		{
			DBGout();
			MOD_DEC_USE_COUNT;
			return (-EBUSY);
		}
		ib_exclusive=1;	// XXX can be replaced by standard file locking I believe
	}


	ib_opened++; // XXX unnecessary, can be replaced

	/* if implicit adressing mode initialize the bus if necessary */
	if( MINOR(inode->i_rdev) > 0 )
	{
		osLockMutex(); /* let complete commands first */
		if ( !(pgmstat & PS_ONLINE) )
			ibonl(1);	// XXX no check for failure?
		ibsic();
		ibsre(1);
		if( ibsta & ERR )
		{
			ib_opened--;
			MOD_DEC_USE_COUNT;
			GIVE_UP(-EIO);
		}
		osUnlockMutex();
	}
	DBGout();
	return (OK);
}


IBLCL int ibclose(struct inode *inode, struct file *file)
{
	DBGin("ibclose");

	/* if implicit adressing mode  unset ren */

	if( (MINOR(inode->i_rdev) > 0 ) )
	{
		ibsre(0);
	}
	if ((pgmstat & PS_ONLINE) && ib_opened == 1 )
		ibonl(0);
	ib_opened--;
	MOD_DEC_USE_COUNT;

	if( ib_exclusive )
		ib_exclusive = 0;

	DBGout();
	return (OK);
}


IBLCL int ibioctl(struct inode *inode, struct file *filep, unsigned int cmd, unsigned long arg)
{
	int	retval = OK;		/* assume everything OK for now */
		ibarg_t m_ibarg,*ibargp;

	int	bufsize;
	int	remain;
	char 	*buf;
	char 	*userbuf;
	char 	c;
	ssize_t ret;
	int end_flag;

	DBGin("ibioctl");
	DBGprint(DBG_DATA,("cmd=%d",cmd));
	ibargp = (ibarg_t *) &m_ibarg;

	/* Check the arg buffer is readable & writable by the current process */
	retval = verify_area(VERIFY_WRITE, (void *)arg, sizeof(ibarg_t));
	if (retval)
	{
		DBGout();
		return (retval);
	}

	retval = verify_area(VERIFY_READ, (void *)arg, sizeof(ibarg_t));
	if (retval)
	{
		DBGout();
		return (retval);
	}

	copy_from_user( (ibarg_t *) ibargp , (ibarg_t *) arg , sizeof(ibarg_t));

	if( cmd == IBAPWAIT )
	{
		DBGprint(DBG_BRANCH,("IBAPWAIT called"));
		/* special case for IBAPWAIT : does his own locking */
		ibAPWait(ibargp->ib_arg);
		ibargp->ib_ibsta = ibsta;
		ibargp->ib_iberr = iberr;
		ibargp->ib_ibcnt = ibcnt;
		copy_to_user((ibarg_t *) arg, (ibarg_t *) ibargp , sizeof(ibarg_t));

		DBGout();
		return retval;
	}

	osLockMutex();        /* lock other processes from performing commands */
                              /* quick & dirty hack (glenn will flame me :-)  */

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
				if(ret < 0) break; 	//XXX
				copy_to_user( userbuf, buf, ret );
				remain -= ret;
				userbuf += ret;
			}while (remain > 0 && end_flag == 0);
			ibcnt = ibargp->ib_cnt - remain;
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
				ibwrt( buf, (bufsize < remain) ? bufsize : remain, (bufsize < remain) );
				remain -= ibcnt;
				userbuf += ibcnt;
			}while (remain > 0 && ibcnt > 0 && !(ibsta & (ERR | TIMO)));
			ibcnt = ibargp->ib_cnt - remain;
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
				ibcmd( buf, (bufsize < remain) ? bufsize : remain );
				remain -= ibcnt;
				userbuf += ibcnt;
			} while (remain > 0 && ibcnt > 0 && !(ibsta & (ERR | TIMO)));
			ibcnt = ibargp->ib_cnt - remain;

			/* Free the DMA buffer */
			osFreeDMABuffer( buf );

			break;

		case IBWAIT:
			DBGprint(DBG_DATA,("**arg=%x",ibargp->ib_arg));
			ibwait(ibargp->ib_arg);
			break;
		case IBRPP:
			/* Check write access to Poll byte */
			retval = verify_area(VERIFY_WRITE, ibargp->ib_buf, 1);
			if (retval)
				GIVE_UP(retval);

			ibrpp(&c);
			put_user( c, ibargp->ib_buf );
			break;
		case IBONL:
			ibonl(ibargp->ib_arg);
			break;
		case IBAPE:
			ibAPE(ibargp->ib_arg,ibargp->ib_cnt);
			break;
		case IBSIC:
			ibsic();
			break;
		case IBSRE:
			ibsre(ibargp->ib_arg);
			break;
		case IBGTS:
			ibgts();
			break;
		case IBCAC:
			ibcac(ibargp->ib_arg);
			break;
		case IBSDBG:
	#if DEBUG
			dbgMask= ibargp->ib_arg | DBG_1PPL;
			DBGprint(DBG_DATA,("dbgMask=0x%x",dbgMask));
	#endif
			break;
		case IBLINES:
			iblines(&ibargp->ib_ret);
			break;
		case IBPAD:
			ibpad(ibargp->ib_arg);
			break;
		case IBSAD:
			ibsad(ibargp->ib_arg);
			break;
		case IBTMO:
			ibtmo(ibargp->ib_arg);
			break;
		case IBEOT:
			ibeot(ibargp->ib_arg);
			break;
		case IBEOS:
			ibeos(ibargp->ib_arg);
			break;
		case IBRSV:
			ibrsv(ibargp->ib_arg);
			break;
		case DVTRG:
			dvtrg(ibargp->ib_arg);
			break;
		case DVCLR:
			dvclr(ibargp->ib_arg);
			break;
		case DVRSP:
			/* Check write access to Poll byte */
			retval = verify_area(VERIFY_WRITE, ibargp->ib_buf, 1);
			if (retval)
			{
				GIVE_UP(retval);
			}
			dvrsp(ibargp->ib_arg, &c);

			put_user( c, ibargp->ib_buf );

			break;
				case IBAPRSP:
			retval = verify_area(VERIFY_WRITE, ibargp->ib_buf, 1);
			if (retval)
			{
				GIVE_UP(retval);
			}
			ibAPrsp(ibargp->ib_arg, &c);
			put_user( c, ibargp->ib_buf );
			break;
		case DVRD:
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
			if (receive_setup(ibargp->ib_arg))
				break;	// XXX

			/* Read DMA buffer loads till we fill the user supplied buffer */
			userbuf = ibargp->ib_buf;
			remain = ibargp->ib_cnt;
			do
			{
				ret = ibrd(buf, (bufsize < remain) ? bufsize : remain, &end_flag);
				if(ret < 0) break;
				copy_to_user( userbuf, buf, ret );
				remain -= ret;
				userbuf += ret;
			}while (remain > 0  && end_flag == 0);	//!(board.update_status() & TIMO));
			ibcnt = ibargp->ib_cnt - remain;

			/* Free the DMA buffer */
			osFreeDMABuffer( buf );
			break;
		case DVWRT:

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
				dvwrt( ibargp->ib_arg, buf, (bufsize < remain) ? bufsize : remain );
				remain -= ibcnt;
				userbuf += ibcnt;
			} while (remain > 0 && ibcnt > 0 && !(ibsta & (ERR | TIMO)));
			ibcnt = ibargp->ib_cnt - remain;

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
			DBGprint(DBG_DATA,("No such Command : %d",cmd));
			retval = -EINVAL;
			break;
	}
	ibargp->ib_ibsta = ibsta;
	ibargp->ib_iberr = iberr;
	ibargp->ib_ibcnt = ibcnt;
	copy_to_user((ibarg_t *) arg, (ibarg_t *) ibargp , sizeof(ibarg_t));

	GIVE_UP(retval);
}






































