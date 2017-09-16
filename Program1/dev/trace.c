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
#include <pcap.h>

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
   if (argc < 2)
   {
      fprintf(stderr, "Usage: ./a.out file\n");
      return 1;
   }

   
   char err[PCAP_ERRBUF_SIZE];
   pcap_t *pfp;
   
   pfp = pcap_open_offline (argv[1], err); 
   
   if (pfp == NULL)
     fprintf(stderr, "open failed %s\n", err);
   
   return 0;
}
