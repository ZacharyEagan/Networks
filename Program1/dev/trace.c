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
 * Ethernet header with packing 
 */
typedef struct ether
{
    uint8_t dst[6];
    uint8_t src[6];
    uint8_t typ[2];
} __attribute__((packed)) Ethernet;


/**
 * Usage: ./a.out file.pcap
 *
 * return: 0 for success 1 for usage error
 **/
int main(int argc, char *argv[])
{
   char err[PCAP_ERRBUF_SIZE];
   pcap_t *pfp;

   struct pcap_pkthdr header;
   const u_char *packet;
   
   /* check for usage */
   if (argc < 2)
   {
      fprintf(stderr, "Usage: ./a.out file\n");
      return 1;
   }
  

   /* open a pcap file and check validity, check for ethernet type */
   pfp = pcap_open_offline (argv[1], err); 
   if (pfp == NULL)
     fprintf(stderr, "open failed %s\n", err);
   if (pcap_datalink(pfp) != DLT_EN10MB)
     fprintf(stderr, "wrong datalink type");

   /* get a packet from pcap */
   packet = pcap_next(pfp, &header);
   fprintf(stderr, "Packet length = %d\n", header.len);
   fprintf(stderr, "Dangerous but here's the packet\n%s\n",packet);
  
   

   pcap_close(pfp);
   return 0;
}
