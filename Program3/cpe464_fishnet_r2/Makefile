# Example makefile for CPE 464
#

CC = gcc
CFLAGS = -g -Wall -Werror
OS = $(shell uname -s)
PROC = $(shell uname -p)
EXEC_SUFFIX=$(OS)-$(PROC)

ifeq ("$(OS)", "SunOS")
	OSLIB=-L/opt/csw/lib -R/opt/csw/lib -lsocket -lnsl -fpermisive
	OSINC=-I/opt/csw/include
	OSDEF=-DSOLARIS
else
ifeq ("$(OS)", "Darwin")
	OSLIB=
	OSINC=
	OSDEF=-DDARWIN
else
	OSLIB=
	OSINC=
	OSDEF=-DLINUX
endif
endif

all:  fishnode-$(EXEC_SUFFIX)

fishnode-$(EXEC_SUFFIX):  
	$(CC) $(CFLAGS) $(OSINC) $(OSLIB) $(OSDEF) -o $@  fishnode.cpp libfish-$(EXEC_SUFFIX).a smartalloc.c

handin: README
	handin bellardo 464_p3 README fishnode.cpp fishnode.h libfish-Linux-x86_64.a fish.h smartalloc.h smartalloc.c Makefile

clean:
	-rm -rf fishnode-$(EXEC_SUFFIX) 
