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

int board_type_ioctl(gpib_board_t *board, unsigned long arg);
int read_ioctl(gpib_board_t *board, unsigned long arg);
int write_ioctl(gpib_board_t *board, unsigned long arg);
int command_ioctl(gpib_board_t *board, unsigned long arg);
int status_ioctl(gpib_board_t *board, unsigned long arg);

#define GIVE_UP(a) {up(&board->mutex); return a;}

int ib_opened=0;
int ib_exclusive=0;

int ibopen(struct inode *inode, struct file *filep)
{
	unsigned int minor = MINOR(inode->i_rdev);

	if(minor >= MAX_NUM_GPIB_BOARDS)
	{
		printk("gpib: invalid minor number of device file\n");
		return -ENODEV;
	}

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


int ibclose(struct inode *inode, struct file *file)
{
	unsigned int minor = MINOR(inode->i_rdev);
	gpib_board_t *board;

	if(minor >= MAX_NUM_GPIB_BOARDS)
	{
		printk("gpib: invalid minor number of device file\n");
		return -ENODEV;
	}
	board = &board_array[minor];
 
	if (board->online && ib_opened == 1 )
		ibonl(&board_array[minor], 0);
	ib_opened--;

	if( ib_exclusive )
		ib_exclusive = 0;

	return 0;
}

int ibioctl(struct inode *inode, struct file *filep, unsigned int cmd, unsigned long arg)
{
	int	retval = 0; 		/* assume everything OK for now */
	ibarg_t m_ibarg,*ibargp;
	char c;
	int end_flag = 0;
	unsigned int minor = MINOR(inode->i_rdev);
	gpib_board_t *board;

	if(minor >= MAX_NUM_GPIB_BOARDS)
	{
		printk("gpib: invalid minor number of device file\n");
		return -ENODEV;
	}
	board = &board_array[minor];
	if(cmd == CFCBOARDTYPE)
		return board_type_ioctl(board, arg);
	if(board->interface == NULL)
	{
		printk("gpib: no gpib board configured on /dev/gpib%i\n", minor);
		return -ENODEV;
	}

printk("minor %i ioctl %i\n", minor, cmd);

	switch( cmd )
	{
		case IBRD:
			return read_ioctl(board, arg);
			break;
		case IBWRT:
			return write_ioctl(board, arg);
			break;
		case IBCMD:
			return command_ioctl(board, arg);
			break;
		case IBSTATUS:
			return status_ioctl(board, arg);
			break;
		case IBTMO:
			retval = ibtmo(board, arg);
			break;
		default:
			break;
	}

	/* XXX lock other processes from performing commands */
	retval = down_interruptible(&board->mutex);
	if(retval)
	{
		printk("gpib: ioctl interrupted while waiting on lock\n");
		return -ERESTARTSYS;
	}

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

		return retval;
	}
#endif

	switch (cmd)
	{
		case IBWAIT:
			retval = ibwait(board, ibargp->ib_arg);
			break;
		case IBRPP:
			/* Check write access to Poll byte */
			retval = verify_area(VERIFY_WRITE, ibargp->ib_buf, 1);
			if (retval)
				GIVE_UP(retval);

			retval = ibrpp(board, &c);
			put_user( c, ibargp->ib_buf );
			break;
		case IBONL:
			retval = ibonl(board, ibargp->ib_arg);
			break;
		case IBAPE:
			ibAPE(board, ibargp->ib_arg,ibargp->ib_cnt);
			break;
		case IBSIC:
			retval = ibsic(board);
			break;
		case IBSRE:
			retval = ibsre(board, ibargp->ib_arg);
			break;
		case IBGTS:
			retval = ibgts(board);
			break;
		case IBCAC:
			retval = ibcac(board, ibargp->ib_arg);
			break;
		case IBSDBG:
			break;
		case IBLINES:
			retval = iblines(board, &ibargp->ib_ret);
			break;
		case IBPAD:
			retval = ibpad(board, ibargp->ib_arg);
			break;
		case IBSAD:
			retval = ibsad(board, ibargp->ib_arg);
			break;
		case IBEOS:
			retval = ibeos(board, ibargp->ib_arg);
			break;
		case IBRSV:
			retval = ibrsv(board, ibargp->ib_arg);
			break;
		case DVRSP:
			/* Check write access to Poll byte */
			retval = verify_area(VERIFY_WRITE, ibargp->ib_buf, 1);
			if (retval)
			{
				GIVE_UP(retval);
			}
			retval = dvrsp(board, ibargp->ib_arg, &c);

			put_user( c, ibargp->ib_buf );

			break;
		case IBAPRSP:
			retval = verify_area(VERIFY_WRITE, ibargp->ib_buf, 1);
			if (retval)
			{
				GIVE_UP(retval);
			}
			retval = ibAPrsp(board, ibargp->ib_arg, &c);
			put_user( c, ibargp->ib_buf );
			break;
		/* special configuration options */
		case CFCBASE:
			osChngBase(board, ibargp->ib_arg);
			break;
		case CFCIRQ:
			osChngIRQ(board, ibargp->ib_arg);
			break;
		case CFCDMA:
			osChngDMA(board, ibargp->ib_arg);
			break;
		default:
			retval = -ENOTTY;
			break;
	}

	// return status bits
	ibargp->ib_ibsta = ibstatus(board);
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

int board_type_ioctl(gpib_board_t *board, unsigned long arg)
{
	struct list_head *list_ptr;
	board_type_ioctl_t cmd;
	int retval;

	retval = copy_from_user(&cmd, (void*)arg, sizeof(board_type_ioctl_t));
	if(retval)
	{
		return retval;
	}

	for(list_ptr = registered_drivers.next; list_ptr != &registered_drivers; list_ptr = list_ptr->next)
	{
		gpib_interface_t *interface;

		interface = list_entry(list_ptr, gpib_interface_t, list);
		if(strcmp(interface->name, cmd.name) == 0)
		{
			board->interface = interface;
			return 0;
		}
	}

	return -EINVAL;
}

int read_ioctl(gpib_board_t *board, unsigned long arg)
{
	read_write_ioctl_t read_cmd;
	uint8_t *userbuf;
	unsigned long remain;
	int end_flag = 0;
	int retval;
	ssize_t ret;

	retval = copy_from_user(&read_cmd, (void*) arg, sizeof(read_cmd));
	if (retval)
		return -EFAULT;

	/* Check write access to buffer */
	retval = verify_area(VERIFY_WRITE, read_cmd.buffer, read_cmd.count);
	if (retval)
		return -EFAULT;

	/* Read buffer loads till we fill the user supplied buffer */
	userbuf = read_cmd.buffer;
	remain = read_cmd.count;
	while(remain > 0 && end_flag == 0)
	{
		ret = ibrd(board, board->buffer, (board->buffer_length < remain) ? board->buffer_length :
			remain, &end_flag);
		if(ret < 0)
		{
			retval = -EIO;
			break;
		}
		copy_to_user(userbuf, board->buffer, ret);
		remain -= ret;
		userbuf += ret;
	}
	read_cmd.count -= remain;
	read_cmd.end = end_flag;

	retval = copy_to_user((void*) arg, &read_cmd, sizeof(read_cmd));
	if(retval) return -EFAULT;

	return 0;
}

int command_ioctl(gpib_board_t *board, unsigned long arg)
{
	read_write_ioctl_t cmd;
	uint8_t *userbuf;
	unsigned long remain;
	int retval;
	ssize_t ret;

	retval = copy_from_user(&cmd, (void*) arg, sizeof(cmd));
	if (retval)
		return -EFAULT;

	/* Check read access to buffer */
	retval = verify_area(VERIFY_READ, cmd.buffer, cmd.count);
	if (retval)
		return -EFAULT;

	/* Write buffer loads till we empty the user supplied buffer */
	userbuf = cmd.buffer;
	remain = cmd.count;
	while (remain > 0 && !(ibstatus(board) & (TIMO)))
	{
		copy_from_user(board->buffer, userbuf, (board->buffer_length < remain) ?
			board->buffer_length : remain );
		ret = ibcmd(board, board->buffer, (board->buffer_length < remain) ?
			board->buffer_length : remain );
		if(ret < 0)
		{
			retval = -EIO;
			break;
		}
		remain -= ret;
		userbuf += ret;
	}

	cmd.count -= remain;

	retval = copy_to_user((void*) arg, &cmd, sizeof(cmd));
	if(retval) return -EFAULT;

	return 0;
}

int write_ioctl(gpib_board_t *board, unsigned long arg)
{
	read_write_ioctl_t write_cmd;
	uint8_t *userbuf;
	unsigned long remain;
	int retval;
	ssize_t ret;

	retval = copy_from_user(&write_cmd, (void*) arg, sizeof(write_cmd));
	if (retval)
		return -EFAULT;

	/* Check read access to buffer */
	retval = verify_area(VERIFY_READ, write_cmd.buffer, write_cmd.count);
	if (retval)
		return -EFAULT;

	/* Write buffer loads till we empty the user supplied buffer */
	userbuf = write_cmd.buffer;
	remain = write_cmd.count;
	while(remain > 0)
	{
		int send_eoi;
		send_eoi = board->buffer_length <= remain && write_cmd.end;
		copy_from_user(board->buffer, userbuf, (board->buffer_length < remain) ?
			board->buffer_length : remain );
		ret = ibwrt(board, board->buffer, (board->buffer_length < remain) ?
			board->buffer_length : remain, send_eoi);
		if(ret < 0)
		{
			retval = -EIO;
			break;
		}
		remain -= ret;
		userbuf += ret;
	}

	write_cmd.count -= remain;

	retval = copy_to_user((void*) arg, &write_cmd, sizeof(write_cmd));
	if(retval) return -EFAULT;

	return 0;
}

int status_ioctl(gpib_board_t *board, unsigned long arg)
{
	int status;
	int retval;

	status = ibstatus(board);

	retval = put_user(status, (int *) arg);
	if (retval)
		return -EFAULT;

	return 0;
}







