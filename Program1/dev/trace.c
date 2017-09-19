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
#include <arpa/inet.h>
#include <endian.h>

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
 * Enume to define types for ethernet
 **/
enum EthoType {ARP = 0x0806, IP = 0x0800, ETHO_LEN = 14}; 

/**
 * Ethernet header with packing 
 **/
typedef struct Ethernet
{
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t typ;
} __attribute__((packed)) Ethernet;


/**
 * ARP header with packing 
 **/
typedef struct Arp_layer
{
    uint16_t hrd; /*hrdwr typ*/
    uint16_t pro; /*pro typ */
    uint8_t hln; /* hardware adr len def 6 */
    uint8_t pln;/* prot adr len def 4 (ipv4)*/
    uint16_t op; /*opcode*/   
    /* need easy way to stor the variable len
     * fields for the addresses */
} __attribute__((packed)) Arp_layer;

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

   Ethernet ether;
   Arp_layer arp_l;
   uint8_t *sha;
   uint8_t *spa;
   uint8_t *tha;
   uint8_t *tpa;
   size_t offset = 0;
   
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
   
   /* get and display the ethernet data */
   memcpy(&ether, packet, sizeof(ether));  
   offset += sizeof(ether);
   fprintf(stderr, "DST Address = %02x:%02x:%02x:%02x:%02x:%02x\n", 
                   ether.dst[0], ether.dst[1] ,ether.dst[2], ether.dst[3], 
                   ether.dst[4], ether.dst[5]);
   fprintf(stderr, "SRC Address = %02x:%02x:%02x:%02x:%02x:%02x\n", 
                   ether.src[0], ether.src[1], ether.src[2], ether.src[3], 
                   ether.src[4], ether.src[5]);

   /* examples for endianess testing */
   #if __BYTE_ORDER == __LITTLE_ENDIAN
       ether.typ = ntohs(ether.typ);
       fprintf(stderr, "Little Endian\n");
   #elif __BYTE_ORDER == __BIG_ENDIAN
       fprintf(stderr, "Big Endian\n");
   #endif
       fprintf(stderr, "Type = %04x\n", ether.typ); 

   /* get the arp header from the packet */
   memcpy(&arp_l, packet + offset, sizeof(arp_l));
   offset += sizeof(arp_l);
   #if __BYTE_ORDER == __LITTLE_ENDIAN
       arp_l.op = ntohs(arp_l.op);
       arp_l.pro = ntohs(arp_l.pro);
       arp_l.op = ntohs(arp_l.hrd);
   #elif __BYTE_ORDER == __BIG_ENDIAN
   #endif

   /* print out arp type and shtuffs */    
   fprintf(stderr, "Op = %04x\nArp Type: %s\n", arp_l.op, arp_l.op == 1 ? 
		   "request": "other");
    /* mallocs */
    sha = malloc(arp_l.hln);
    spa = malloc(arp_l.pln);
    tha = malloc(arp_l.hln);
    tpa = malloc(arp_l.pln);
    
   memcpy(sha, packet + offset, arp_l.hln);
   offset += sizeof(arp_l.hln);
   memcpy(spa, packet + offset, arp_l.pln);
   offset += sizeof(arp_l.pln);
   memcpy(tha, packet + offset, arp_l.hln);
   offset += sizeof(arp_l.hln);
   memcpy(tpa, packet + offset, arp_l.pln);
   offset += sizeof(arp_l.pln);

    /* frees */
    free(sha);
    free(spa);
    free(tha);
    free(tpa);
    

   pcap_close(pfp);
   return 0;
}
