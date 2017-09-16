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
#include <stdint.h>

#include "smartalloc.h"
#include "checksum.h"


/**
 * Example Structure with packing
 **/
typedef struct packed 
{
   uint8_t a;
   uint16_t b;
   uint8_t c;
   uint32_t d;
} __attribute__((packed))Package;   

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
