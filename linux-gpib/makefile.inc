BOLD		= "\033[1m"
NBOLD		= "\033[0m"

ECHO		= /bin/echo

COMPILE	= $(ECHO) "--- Compiling "$(BOLD)$<$(NBOLD)"..." ; gcc
DEPEND	= $(ECHO) "--- Checking dependencies..." ; gcc
RLINK	= $(ECHO) "--- Linking (relocatable) "$(BOLD)$@$(NBOLD)"..." ; ld -r
LINK	= $(ECHO) "--- Linking "$(BOLD)$@$(NBOLD)"..." ; gcc
INSTALL	= $(ROOT)/util/Install.sh
RM	= $(ROOT)/util/Remove.sh
YACC	= $(ECHO) --- Running bison on $(BOLD)$<$(NBOLD)...; bison -d -y
LEX	= $(ECHO) --- Running flex on $(BOLD)$<$(NBOLD)...; flex 

CC	= $(COMPILE)


print_vars:
	$(ECHO) "CC = $(CC)"
	$(ECHO) "EXP_INC = $(EXP_INC)"
