/**
 * CPE464 Networks
 * Program 1 "Trace"
 *
 * Zachary Eagan
 * opened: 9/16/2017
 * closed: --------
 *
 * Program reads and displays aspects of a network trace. 
 * reads only from file
 *
 **/



#include <stdio.h>
#include <stdlib.h>

#include "smartalloc.h"
#include "checksum.h"

/**
 * Usage: ./a.out file.pcap
 *
 * return: 0 for success 1 for usage error
 **/
int main(int argc, char *argv[])
{
   if (argc < 1)
      return 1;
   
   printf("hello\n");
   return 0;
}
