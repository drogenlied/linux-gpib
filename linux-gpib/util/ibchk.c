/***************************************************************************
                          ibchk.c

	Test and diagnostic program for GPIB-Library
	This is not a example file for own GPIB experiments!

                             -------------------
    copyright            : (C) by Claus Shroeter
                           (C) 2002 by Frank Mori Hess
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


#include <ib.h>
#include <ibP.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc,char **argv)
{
	char *envptr;
	int fd;
	int ind=0;
	char str[30];

	/****************/
	fprintf(stderr, "**First checking your config file....                ");
	if(( envptr = (char *) getenv("IB_CONFIG"))== (char *)0 )
	{
		if(ibParseConfigFile("/etc/gpib.conf") < 0  )
		{
			fprintf(stderr, "\n"
				"OOps! There is a Problem With your Default Config File\n"
				"/etc/gpib.conf. Please check out if one exists or look for Syntax Errors.\n");
				exit(1);
		}
	}else
	{
		if(ibParseConfigFile(envptr) < 0)
		{
			fprintf(stderr, "\n"
				"OOps! There is a Problem With your Config File set in IB_CONFIG\n"
				"environment Variable. Please check out if one exists or look for Syntax Errors.\n");
			exit(1);
		}
	}

	fprintf(stderr, "OK\n");

	fprintf(stderr, "**Next Check The Driver.....                         ");

	sprintf(str, "/dev/gpib%d", ind);
	if(( fd=open(str,O_RDWR) ) < 0 )
	{
		fprintf(stderr, "\n There is a Problem with the Driver: ");
		fprintf(stderr, "\n open(%s) says \"%s\" ", str, strerror(errno));
		fprintf(stderr, "\n Have you loaded the Driver Module into the Kernel?\n"
			"Or check if '%s' exists!\n",str);
		exit(1);
	}

	close(fd);
	fprintf(stderr, "OK\n");

	/************/
	fprintf(stderr, "**Check if Board present....                         ");
	if(ibBdChrConfig(ind, ibBoard[CONF(ind,board)].base,
		ibBoard[CONF(ind,board)].irq,
		ibBoard[CONF(ind,board)].dma) & ERR )
	{
		fprintf(stderr, "\n  Problems while setting up Base and Irq\n"
			"Perhaps you changed Base and Irq Without reloading the Driver?\n");
		exit(1);
	}

	if( ibonl( ind, 1) & ERR )
	{
		if (iberr == ENEB)
		{
			fprintf(stderr, "\n  Board not found at Base Adress: 0x%x. \n"
				"Check if Base-Adress or IRQ/DMA has correctly been set\n"
				"or if you have an unsupported board.\n"
				"Dump of Additional Messages follows:\n"
				"--------------------------\n", ibBoard[CONF(ind,board)].base );
			system("tail -n5 /usr/adm/messages");
			fprintf(stderr, "\n  --------------------------\n");
		}
		exit(1);
	}
	fprintf(stderr, "OK\n");
	/***************/

	fprintf(stderr, "**Checking some Bus Functions...                     ");

	if( ibsic(ind) & ERR )
	{
		fprintf(stderr, "\n  Problem Sending IFC\n");
		exit(1);
	}
	if(ibcmd(ind, "  ", 3) & ERR )
	{
		fprintf(stderr, "\n  Bus or another Hardware Problem\n");
		if( ibsta & TIMO )
			fprintf(stderr, "Look if your Card's IRQ and DMA Jumper matches those\n"
				"in the Configuration file and check if you have at least\n"
				"one device connected to your bus.\n");
		exit(1);
	}

	fprintf(stderr, "OK\n");

	/****************/


	fprintf(stderr, "\n\n  Everything seems to be OK to me (found %d devices).\n"
		"It's now your turn to check out\n"
		"some device commands with your own programs that use GPIB commands.\n"
		"Good luck.\n", ibGetNrDev());

	return 0;
}






