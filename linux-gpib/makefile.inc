BOLD		= ""
NBOLD		= ""

ECHO		= /bin/echo

COMPILE	= gcc
DEPEND	= gcc
RLINK	= ld -r
LINK	= gcc
INSTALL	= install 
YACC	= bison -d -y
LEX	= flex 

CC	= $(COMPILE)


print_vars:
	$(ECHO) "CC = $(CC)"
	$(ECHO) "EXP_INC = $(EXP_INC)"
