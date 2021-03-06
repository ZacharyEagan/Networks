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
#include "checksum.h"
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
 * prints a decimal formatted address
 **/
void print_IP_address(uint8_t *data, uint8_t size)
{
   int i = 0;
   if (size)
      printf("%d", data[i++]); 
   while (i < size)
   {
       printf(".%d", data[i++]);
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
       arp_l.hrd = ntohs(arp_l.hrd);
   #elif __BYTE_ORDER == __BIG_ENDIAN
   #endif

   /* print out arp type and shtuffs */   
   printf("\tARP Header\n");
   printf("\t\tOpcode: %s\n", arp_l.op == 1 ? "Request" : "Reply");

   /* mallocs */
   sha = malloc(arp_l.hln);
   spa = malloc(arp_l.pln);
   tha = malloc(arp_l.hln);
   tpa = malloc(arp_l.pln);
   
   /* copy the addresses */  
   memcpy(sha, packet + *offset, arp_l.hln);
   *offset += arp_l.hln;
   memcpy(spa, packet + *offset, arp_l.pln);
   *offset += arp_l.pln;
   memcpy(tha, packet + *offset, arp_l.hln);
   *offset += arp_l.hln;
   memcpy(tpa, packet + *offset, arp_l.pln);
   *offset += arp_l.pln;

   printf("\t\tSender MAC: ");
   print_address(sha, arp_l.hln);
   printf("\n");
   

   printf("\t\tSender IP: ");
   print_IP_address(spa, arp_l.pln);
   printf("\n");

   printf("\t\tTarget MAC: ");
   print_address(tha, arp_l.hln);
   printf("\n");
  

   printf("\t\tTarget IP: ");
   print_IP_address(tpa, arp_l.pln);
   printf("\n");


    /* frees */
    free(sha);
    free(spa);
    free(tha);
    free(tpa);
}

/**
 * gets and prints IP header
 * updates offset
 * returns type off next packet
 **/
uint8_t parse_IP(const u_char *packet, size_t *offset, TCP_psudo *psudo)
{
   IP_layer ip;
   /* get the IP header from the packet */
   memcpy(&ip, packet + *offset, sizeof(ip));
   
/*copy relevent data to the psudo header in case of TCP*/
   psudo->src[0] = ip.src[0];
   psudo->src[1] = ip.src[1];
   psudo->src[2] = ip.src[2];
   psudo->src[3] = ip.src[3];
   psudo->dst[0] = ip.dst[0];
   psudo->dst[1] = ip.dst[1];
   psudo->dst[2] = ip.dst[2];
   psudo->dst[3] = ip.dst[3];
   psudo->pad = 0;
   psudo->tcp_prot = ip.prot;
/*endcopy*/

   #if __BYTE_ORDER == __LITTLE_ENDIAN
       ip.tlen = ntohs(ip.tlen);
       ip.id = ntohs(ip.id);
       ip.fragOff = ntohs(ip.fragOff);
       ip.check = ntohs(ip.check);
   #elif __BYTE_ORDER == __BIG_ENDIAN
   #endif
   /* set the offset according to the header length value */
   *offset += (ip.ver_hLen & 0x0F) * 4;
   
   printf("\tIP Header\n");
   printf("\t\tTOS: 0x%x\n", ip.tos);
   printf("\t\tTTL: %d\n", ip.ttl);
   printf("\t\tProtocol: ");
   switch (ip.prot)
   {
      case ICMP: printf("ICMP"); break; 
      case TCP : printf("TCP"); break;
      case UDP : printf("UDP"); break;
      default  : fprintf(stderr, "ip.prot: unknown\n"); break;
   }
   printf("\n");
   
   check_IP(ip);

   printf("\t\tChecksum: %s (0x%x)\n", check_IP(ip) ? "Correct" :"Incorect" , ip.check);
   

   printf("\t\tSender IP: ");
   print_IP_address(ip.src, 4);
   printf("\n");
   printf("\t\tDest IP: ");
   print_IP_address(ip.dst, 4);
   printf("\n\n");

   return ip.prot;
}

/**
 * gets and prints ICMP header
 * updates offset
 * returns type off next packet
 **/
uint8_t parse_ICMP(const u_char *packet, size_t *offset)
{
   ICMP_layer icmp;
   /* get the IP header from the packet */
   memcpy(&icmp, packet + *offset, sizeof(icmp));
   #if __BYTE_ORDER == __LITTLE_ENDIAN
       icmp.check = ntohs(icmp.check);
   #elif __BYTE_ORDER == __BIG_ENDIAN
   #endif
   /* set the offset according to the header length value */
   *offset += sizeof(icmp);
   
   printf("\t\tType: ");
   switch (icmp.typ)
   {
      case ECHO_REPLY: printf("Reply");break;
      case ECHO_REQ  : printf("Request");break;
      default : printf("Unknown %d", icmp.typ);break;
   }
   printf("\n"); 
 
   return icmp.typ;
}

/**
 * gets and prints TCP header
 * updates offset
 * returns type off next packet
 **/
uint8_t parse_TCP(const u_char *packet, size_t *offset, TCP_psudo psudo)
{
   TCP_layer tcp;

   fprintf(stderr, "psudo len = %d\n", psudo.tcp_len);
   memcpy(psudo.rest, packet, psudo.tcp_len);   
   fprintf(stderr, "chksum %d\n", in_cksum((short unsigned int *)&psudo, psudo.tcp_len));

   /* get the TCP header from the packet */
   memcpy(&tcp, packet + *offset, sizeof(tcp));
   #if __BYTE_ORDER == __LITTLE_ENDIAN
       tcp.src = ntohs(tcp.src);
       tcp.dst = ntohs(tcp.dst);
       tcp.sqnc = ntohl(tcp.sqnc);
       tcp.ack = ntohl(tcp.ack);
       tcp.window = ntohs(tcp.window);
       tcp.check = ntohs(tcp.check);
       tcp.urgent = ntohs(tcp.urgent);
   #elif __BYTE_ORDER == __BIG_ENDIAN
   #endif
   /* set the offset according to the header length value */
   *offset += sizeof(tcp);
   
   printf("\t\tSource Port:  %d\n", tcp.src);
   printf("\t\tDest Port:  %d\n", tcp.dst);
   printf("\t\tSequence Number: %lu\n", (long unsigned int)tcp.sqnc);
   printf("\t\tACK Number: %lu\n",(long unsigned int) tcp.ack);
   printf("\t\tSYN Flag: %s\n", tcp.multiple & 0x02 ? "Yes" : "No");
   printf("\t\tRST Flag: %s\n", tcp.multiple & 0x04 ? "Yes" : "No");
   printf("\t\tFIN Flag: %s\n", tcp.multiple & 0x01 ? "Yes" : "No");
   printf("\t\tWindow Size: %d\n", tcp.window);
   printf("\t\tChecksum: %s (0x%x)\n", check_TCP(tcp) ? "Incorrect" : "Incorrect", tcp.check);
   fprintf(stderr, "TCP checksum invalid/not written propperly yet\n");
 
   return tcp.multiple;
}


/**
 * gets and prints UDP header
 * updates offset
 * returns type off next packet
 **/
uint8_t parse_UDP(const u_char *packet, size_t *offset)
{
   UDP_layer udp;
   /* get the IP header from the packet */
   memcpy(&udp, packet + *offset, sizeof(udp));
   #if __BYTE_ORDER == __LITTLE_ENDIAN
       udp.src = ntohs(udp.src);
       udp.dst = ntohs(udp.dst);
       udp.len = ntohs(udp.len);
       udp.check = ntohs(udp.check);
   #elif __BYTE_ORDER == __BIG_ENDIAN
   #endif
   /* set the offset according to the header length value */
   *offset += sizeof(udp);
   
   printf("\t\tSource Port:  %d\n", udp.src);
   printf("\t\tDest Port:  %d\n", udp.dst);
 
   return udp.len;
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

   TCP_psudo psudo;
 
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
     printf("Un-known PDU\n");


   /* get a packet from pcap */
   /* loop for all packets */
   while (pcap_next_ex(pfp, &header, &packet) != -2)
   {
      offset = 0;
      printf("\nPacket number: %d  Packet Len: %d\n\n", pcount++, header->len);
      type = parse_Etho(packet, &offset);
      switch(type)
      {
         case ARP: parse_ARP(packet, &offset); break;

         case IP : switch (parse_IP(packet, &offset, &psudo))
                   {
                     case ICMP: printf("\tICMP Header\n");
                                parse_ICMP(packet, &offset);
                                break;
                     case TCP : psudo.tcp_len = header->len - offset;
                                printf("\tTCP Header\n");
                                parse_TCP(packet, &offset, psudo);
                                break;
                     case UDP : printf("\tUDP Header\n");
                                parse_UDP(packet, &offset);
                                break;
                     default  : break;
                   }
                   break;

         default : fprintf(stderr, "Have not implimented type: %d\n", type);
      }
   }    

   pcap_close(pfp);
   return 0;
}
