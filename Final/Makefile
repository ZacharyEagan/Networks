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

all:  server-$(EXEC_SUFFIX)

server-$(EXEC_SUFFIX):  
	$(CC) $(CFLAGS) $(OSINC) $(OSLIB) $(OSDEF) -o $@ json-server.c  smartalloc.c

handin: README
	handin bellardo 464_fp README json-server.c json-server.h extrafile.h  smartalloc.h smartalloc.c Makefile

clean:
	-rm -rf *.o *.out *.a server-$(EXEC_SUFFIX)
