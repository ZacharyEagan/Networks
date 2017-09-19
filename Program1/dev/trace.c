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

#include "trace.h"


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
   int i, pcount = 1;
   
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

while (1)
{
   /* get a packet from pcap */
   packet = pcap_next(pfp, &header);
   if (packet == NULL) //not sure if this is the right test need to check
   {
       break;
   }
   fprintf(stderr, "\nPacket number: %d  Packet Len: %d\n\n",pcount++, header.len);
   
   /* get and display the ethernet data */
   memcpy(&ether, packet, sizeof(ether));  
   offset += sizeof(ether);
  fprintf(stderr, "\tEthernet Header\n");
  fprintf(stderr, "\t\tDest MAC: %x:%x:%x:%x:%x:%x\n",
                   ether.dst[0], ether.dst[1] ,ether.dst[2], ether.dst[3], 
                   ether.dst[4], ether.dst[5]);
   fprintf(stderr, "\t\tSource MAC: %x:%x:%x:%x:%x:%x\n", 
                   ether.src[0], ether.src[1], ether.src[2], ether.src[3], 
                   ether.src[4], ether.src[5]);

   /* examples for endianess testing */
   #if __BYTE_ORDER == __LITTLE_ENDIAN
       ether.typ = ntohs(ether.typ);
//       fprintf(stderr, "Little Endian\n");
   #elif __BYTE_ORDER == __BIG_ENDIAN
//       fprintf(stderr, "Big Endian\n");
   #endif
       if (ether.typ == ARP)
           fprintf(stderr, "\t\tType: ARP\n\n");
       if (ether.typ == IP)
           fprintf(stderr, "\t\tType: IP\n\n");  

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
   fprintf(stderr, "\tARP Header\n");
   fprintf(stderr, "\t\tOpcode: %s\n", arp_l.op == 1 ? "Request": "Reply");

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

   fprintf(stderr, "\t\tSender MAC: ");
   i = 0;
   fprintf(stderr, "%x", sha[i++]);
   do 
   {
       fprintf(stderr, ":%x", sha[i++]);
   } while (i < arp_l.hln); 
   fprintf(stderr, "\n");

   fprintf(stderr, "\t\tSender IP: ");
   i = 0;
   fprintf(stderr, "%x", spa[i++]);
   do 
   {
       fprintf(stderr, ":%x", spa[i++]);
   } while (i < arp_l.pln); 
   fprintf(stderr, "\n");

   fprintf(stderr, "\t\tTarget MAC: ");
   i = 0;
   fprintf(stderr, "%x", tha[i++]);
   do 
   {
       fprintf(stderr, ":%x", tha[i++]);
   } while (i < arp_l.hln); 
   fprintf(stderr, "\n");
  

   fprintf(stderr, "\t\tTarget IP: ");
   i = 0;
   fprintf(stderr, "%x", tpa[i++]);
   do 
   {
       fprintf(stderr, ":%x", tpa[i++]);
   } while (i < arp_l.pln); 
   fprintf(stderr, "\n");


    /* frees */
    free(sha);
    free(spa);
    free(tha);
    free(tpa);
}    

   pcap_close(pfp);
   return 0;
}
