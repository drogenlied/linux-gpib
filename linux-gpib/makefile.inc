BOLD		= ""
NBOLD		= ""

ECHO		= /bin/echo

CC	= gcc

COMPILE	= $(CC)
DEPEND	= $(CC)
RLINK	= ld -r
LINK	= $(CC)
INSTALL	= install 
YACC	= bison -d -y
LEX	= flex 



print_vars:
	$(ECHO) "CC = $(CC)"
	$(ECHO) "EXP_INC = $(EXP_INC)"
