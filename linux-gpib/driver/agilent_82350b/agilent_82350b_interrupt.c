/***************************************************************************
                          agilent_82350b/agilent_82350b_interrupt.c  -  description
                             -------------------
 interrupt service routine

    copyright            : (C) 2002, 2004-2005 by Frank Mori Hess
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

#include "agilent_82350b.h"

irqreturn_t agilent_82350b_interrupt(int irq, void *arg, struct pt_regs *registerp)
{
	int tms9914_status1, tms9914_status2;
	int event_status;
	gpib_board_t *board = arg;
	agilent_82350b_private_t *a_priv = board->private_data;
	unsigned long flags;
	irqreturn_t retval = IRQ_NONE;
	
	spin_lock_irqsave( &board->spinlock, flags );
	event_status = readb(a_priv->gpib_base + EVENT_STATUS_REG);
	printk("event_status=0x%x\n", event_status);
	if(event_status & IRQ_STATUS_BIT)
	{
		retval = IRQ_HANDLED;
	}
	if(event_status & TMS9914_IRQ_STATUS_BIT)
	{
		tms9914_status1 = read_byte( &a_priv->tms9914_priv, ISR0);
		tms9914_status2 = read_byte( &a_priv->tms9914_priv, ISR1);
		tms9914_interrupt_have_status(board, &a_priv->tms9914_priv, tms9914_status1, tms9914_status2);
	}
	//write-clear status bits
	if(event_status & (BUFFER_END_STATUS_BIT | TERM_COUNT_STATUS_BIT))
	{
		writeb(event_status & (BUFFER_END_STATUS_BIT | TERM_COUNT_STATUS_BIT), 
			a_priv->gpib_base + EVENT_STATUS_REG);
	}
	spin_unlock_irqrestore( &board->spinlock, flags );
	return retval;
}

