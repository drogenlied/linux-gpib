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
	printf("**First checking your config file....                ");
	if(( envptr = (char *) getenv("IB_CONFIG"))== (char *)0 )
	{
		if(ibParseConfigFile("/etc/gpib.conf") < 0  )
		{
			printf("\n"
				"OOps! There is a Problem With your Default Config File\n"
				"/etc/gpib.conf. Please check out if one exists or look for Syntax Errors.\n");
				exit(1);
		}
	}else
	{
		if(ibParseConfigFile(envptr) < 0)
		{
			printf("\n"
				"OOps! There is a Problem With your Config File set in IB_CONFIG\n"
				"environment Variable. Please check out if one exists or look for Syntax Errors.\n");
			exit(1);
		}
	}

	printf("OK\n");

	printf("**Next Check The Driver.....                         ");

	sprintf(str, "/dev/gpib%d", ind);
	if(( fd=open(str,O_RDWR) ) < 0 )
	{
		printf("\n There is a Problem with the Driver: ");
		printf("\n open(%s) says \"%s\" ", str, strerror(errno));
		printf("\n Have you loaded the Driver Module into the Kernel?\n"
			"Or check if '%s' exists!\n",str);
		exit(1);
	}

	close(fd);
	printf("OK\n");

	/************/
	printf("**Check if Board present....                         ");
	if(ibBdChrConfig(ind, ibBoard[CONF(ind,board)].base,
		ibBoard[CONF(ind,board)].irq,
		ibBoard[CONF(ind,board)].dma) & ERR )
	{
		printf("\n  Problems while setting up Base and Irq\n"
			"Perhaps you changed Base and Irq Without reloading the Driver?\n");
		exit(1);
	}

	if( ibonl( ind, 1) & ERR )
	{
		if (iberr == ENEB)
		{
			printf("\n  Board not found at Base Adress: 0x%x. \n"
				"Check if Base-Adress or IRQ/DMA has correctly been set\n"
				"or if you have an unsupported board.\n"
				"Dump of Additional Messages follows:\n"
				"--------------------------\n", ibBoard[CONF(ind,board)].base );
			system("tail -n5 /usr/adm/messages");
			printf("\n  --------------------------\n");
		}
		exit(1);
	}
	printf("OK\n");
	/***************/

	printf("**Checking some Bus Functions...                     ");

	if( ibsic(ind) & ERR )
	{
		printf("\n  Problem Sending IFC\n");
		exit(1);
	}
	if(ibcmd(ind, "  ", 3) & ERR )
	{
		printf("\n  Bus or another Hardware Problem\n");
		if( ibsta & TIMO )
			printf("Look if your Card's IRQ and DMA Jumper matches those\n"
				"in the Configuration file and check if you have at least\n"
				"one device connected to your bus.\n");
		exit(1);
	}

	printf("OK\n");

	/****************/


	printf("\n\n  Everything seems to be OK for me.(Found %d Devices).\n"
		"It's now your turn to check out\n"
		"Some Device Commands with ibsh or with own Programs that uses GPIB Commands.\n"
		"Good Luck.\n", ibGetNrDev());

	return 0;
}






