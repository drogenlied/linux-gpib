
ROOT=../..
INCLUDES = -I. -I$(ROOT)/include
LIBS = -L.. -lgpib -lfl


LD = ld
BIN = rgpib_svc testcl 
GEN = rgpib_clnt.c rgpib_svc.c rgpib.h rgpib_xdr.c
LIB =
OBJS = rgpib_lib.o rgpib_clnt.o rgpib_xdr.o

RPCCOM = rpcgen 
#RPCCOM = ../utils/rpcgen -I

CFLAGS = $(INCLUDES) -g


all: $(GEN) rgpib.o

###
# note: no xdr routines are generated here, due this service's
#       use of basic data types.
###
$(GEN): rgpib.x
	$(ECHO) Creating $(BOLD)rgpib.h$(NBOLD)...
	$(RPCCOM) -h rgpib.x -o rgpib.h
	$(ECHO) Creating $(BOLD)rgpib_clnt.c$(NBOLD)...
	$(RPCCOM) -l rgpib.x -o rgpib_clnt.c
	$(ECHO) Creating $(BOLD)rgpib_svc.c$(NBOLD)...
	$(RPCCOM) -m rgpib.x -o rgpib_svc.c
	$(ECHO) Creating $(BOLD)rgpib_xdr.c$(NBOLD)...
	$(RPCCOM) -c rgpib.x -o rgpib_xdr.c


##
# the gpib server
#
##
SERVEROBJS = rgpib_proc.o rgpib_svc.o rgpib_main.o pmatch.o rgpib_auth.o $(OBJS)

rgpib_svc: $(SERVEROBJS) ../libgpib.a
	$(LINK) -o $@ $(SERVEROBJS) $(LIB) $(LIBS)


##
# a simple testclient (will be removed)
##
testcl: testcl.o rgpib_clnt.o
	$(CC) -o $@ testcl.o rgpib_clnt.o rgpib_xdr.o $(LIB) 

testcl.o: testcl.c rgpib.h

rgpib_proc.o: rgpib_proc.c rgpib.h
rgpib_main.o: rgpib_main.c rgpib.h
rgpib_xdr.o: rgpib_xdr.c rgpib.h

rgpib.o: $(OBJS)
	$(LD) -r $(OBJS) -o rgpib.o


clean cleanup:
	$(RM) *.o $(BIN) 

realclean:
	$(RM) $(GEN) *.o $(BIN)


.SILENT:
include $(ROOT)/makefile.inc

