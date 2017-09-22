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


typedef struct TCP_psudo
{
   uint8_t src[4];
   uint8_t dst[4];
   uint8_t pad;
   uint8_t tcp_prot;
   uint16_t tcp_len;
   uint8_t rest[1522];
} __attribute__((packed)) TCP_psudo;




/**
 *  * Enume to define types for ethernet
 *   **/
enum EthoType {ARP = 0x0806, IP = 0x0800, ETHO_LEN = 14};

/**
 * Enume to define types for IP and bellow
 **/
enum IP_Type {ICMP = 1, TCP = 6, UDP = 17};

enum UDP_Type {ORANGES = 0}; 
/**
 * UDP header with packing
 **/
typedef struct UDP_layer
{
   uint16_t src;
   uint16_t dst;
   uint16_t len;
   uint16_t check;
   //ignoring rest of header cause don't need it
} __attribute__((packed)) UDP_layer;


enum TCP_Type {APPLES = 0}; 
/**
 * TCP header with packing
 **/
typedef struct TCP_layer
{
   uint16_t src;
   uint16_t dst;
   uint32_t sqnc;
   uint32_t ack;
   uint8_t  off_res_ns;
   uint8_t  multiple;
   uint16_t window;
   uint16_t check;
   uint16_t urgent;
   //ignoring rest of header cause don't need it
} __attribute__((packed)) TCP_layer;


/**
 * Checksum checker for TCP header
 **/
int check_TCP(TCP_layer tcp)
{
   return 0;
}

enum ICMP_Type {ECHO_REPLY = 0, ECHO_REQ = 8}; 
/**
 * ICMP header with packing
 **/
typedef struct ICMP_layer
{
   uint8_t typ;
   uint8_t code;
   uint16_t check; 
   uint32_t rest;
   //ignoring rest of header cause don't need it
} __attribute__((packed)) ICMP_layer;




/**
 * IP Header with packing
 *   **/
typedef struct IP_layer
{
    uint8_t ver_hLen;
    uint8_t tos;
    uint16_t tlen;
    uint16_t id;
    uint16_t fragOff;
    uint8_t ttl;
    uint8_t prot;
    uint16_t check;
    uint8_t src[4];
    uint8_t dst[4];
    //note: does not include variable IP optional
} __attribute__((packed)) IP_layer;

/**
 * Checksum checker for IP header
 **/
int check_IP(IP_layer ip)
{
   uint32_t sum = (ip.ver_hLen << 8) + ip.tos;
   sum += ip.tlen;
   sum += ip.id;
   sum += ip.fragOff;
   sum += ((uint32_t)ip.ttl << 8);
   sum += ip.prot;
   sum += ip.check;
   sum += (ip.src[0] << 8) + ip.src[1];
   sum += (ip.src[2] << 8) + ip.src[3];
   sum += (ip.dst[0] << 8) + ip.dst[1];
   sum += (ip.dst[2] << 8) + ip.dst[3];  
   sum = (sum & 0x0000ffff) + (sum >> 16);

   return sum ^ 0xFFFFFFFF;
}

/**
 *  * Ethernet header with packing 
 *   **/
typedef struct Ethernet
{
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t typ;
} __attribute__((packed)) Ethernet;


/**
 *  * ARP header with packing 
 *   **/
typedef struct Arp_layer
{
    uint16_t hrd; /*hrdwr typ*/
    uint16_t pro; /*pro typ */
    uint8_t hln; /* hardware adr len def 6 */
    uint8_t pln;/* prot adr len def 4 (ipv4)*/
    uint16_t op; /*opcode*/
    /* need easy way to stor the variable len
 *      * fields for the addresses */
} __attribute__((packed)) Arp_layer;

/**
 * prints a hexedecimal formatted address
 **/
void print_address(uint8_t *data, uint8_t size);

/**
 * prints a hexedecimal formatted address
 **/
void print_IP_address(uint8_t *data, uint8_t size);

/**
 * gets and prints the ethernet packet
 * updates offset
 * returns type of next packet
 **/
uint16_t parse_Etho(const u_char *packet, size_t *offset);

/**
 * prints data from the arp header
 **/
void parse_ARP(const u_char *packet, size_t *offset);

/**
 * gets and prints IP header
 * updates offset
 * returns type off next packet
 **/
uint8_t parse_IP(const u_char *packet, size_t *offset, TCP_psudo *psudo);


/**
 * gets and prints ICMP header
 * updates offset
 * returns type off next packet
 **/
uint8_t parse_ICMP(const u_char *packet, size_t *offset);

/**
 * gets and prints TCP header
 * updates offset
 * returns type off next packet
 **/
uint8_t parse_TCP(const u_char *packet, size_t *offset, TCP_psudo psudo);


/**
 * gets and prints UDP header
 * updates offset
 * returns type off next packet
 **/
uint8_t parse_UDP(const u_char *packet, size_t *offset);
