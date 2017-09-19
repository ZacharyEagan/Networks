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
 * prints a hexedecimal formatted address
 **/
void print_address(uint8_t *data, uint8_t size)
{
   int i = 0;
   if (size)
      printf("%x", data[i++]); 
   while (i < size)
   {
       printf(":%x", data[i++]);
   }
}


/**
 * build and print etho header
 * update offset
 * return type of next packet
 **/
uint16_t parse_Etho(const u_char *packet, size_t *offset)
{ 
   Ethernet ether;

   /* get and display the ethernet data */
   memcpy(&ether, packet + *offset, sizeof(ether));  
   *offset += sizeof(ether);

   printf("\tEthernet Header\n");
   printf("\t\tDest MAC: ");
   print_address(ether.dst, 6);
   printf("\n");

   printf("\t\tSource MAC: ");
   print_address(ether.src, 6);
   printf("\n");


   /* examples for endianess testing */
   #if __BYTE_ORDER == __LITTLE_ENDIAN
      ether.typ = ntohs(ether.typ);
   #elif __BYTE_ORDER == __BIG_ENDIAN
   #endif

   if (ether.typ == ARP)
      printf("\t\tType: ARP\n\n");
   if (ether.typ == IP)
      printf("\t\tType: IP\n\n");  
   
   return ether.typ;
}

/**
 * print data from the arp header
 **/
void parse_ARP(const u_char *packet, size_t *offset)
{
   Arp_layer arp_l;
   uint8_t *sha;
   uint8_t *spa;
   uint8_t *tha;
   uint8_t *tpa;

   /* get the arp header from the packet */
   memcpy(&arp_l, packet + *offset, sizeof(arp_l));
   *offset += sizeof(arp_l);
   #if __BYTE_ORDER == __LITTLE_ENDIAN
       arp_l.op = ntohs(arp_l.op);
       arp_l.pro = ntohs(arp_l.pro);
       arp_l.op = ntohs(arp_l.hrd);
   #elif __BYTE_ORDER == __BIG_ENDIAN
   #endif

   /* print out arp type and shtuffs */   
   printf("\tARP Header\n");
   printf("\t\tOpcode: %s\n", arp_l.op == 1 ? "Request": "Reply");

   /* mallocs */
   sha = malloc(arp_l.hln);
   spa = malloc(arp_l.pln);
   tha = malloc(arp_l.hln);
   tpa = malloc(arp_l.pln);
    
   memcpy(sha, packet + *offset, arp_l.hln);
   *offset += sizeof(arp_l.hln);
   memcpy(spa, packet + *offset, arp_l.pln);
   *offset += sizeof(arp_l.pln);
   memcpy(tha, packet + *offset, arp_l.hln);
   *offset += sizeof(arp_l.hln);
   memcpy(tpa, packet + *offset, arp_l.pln);
   *offset += sizeof(arp_l.pln);

   printf("\t\tSender MAC: ");
   print_address(sha, arp_l.hln);
   printf("\n");
   

   printf("\t\tSender IP: ");
   print_address(spa, arp_l.pln);
   printf("\n");

   printf("\t\tTarget MAC: ");
   print_address(tha, arp_l.hln);
   printf("\n");
  

   printf("\t\tTarget IP: ");
   print_address(tpa, arp_l.pln);
   printf("\n");


    /* frees */
    free(sha);
    free(spa);
    free(tha);
    free(tpa);
}


/**
 * Usage: ./a.out file.pcap
 *
 * return: 0 for success 1 for usage error
 **/
int main(int argc, char *argv[])
{
   char err[PCAP_ERRBUF_SIZE];
   pcap_t *pfp;

   struct pcap_pkthdr *header;
   const u_char *packet;

   size_t offset = 0;
   int  pcount = 1;

   uint16_t type;
   
   /* check for usage */
   if (argc != 2)
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
   /* loop for all packets */
   while (pcap_next_ex(pfp, &header, &packet) != -2)
   {
      offset = 0;
      fprintf(stderr, "\nPacket number: %d  Packet Len: %d\n\n", pcount++, header->len);
      type = parse_Etho(packet, &offset);
      if (type)
         type++;
      parse_ARP(packet, &offset);
   }    

   pcap_close(pfp);
   return 0;
}
