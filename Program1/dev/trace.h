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
 *  * Enume to define types for ethernet
 *   **/
enum EthoType {ARP = 0x0806, IP = 0x0800, ETHO_LEN = 14};

/**
 * Enume to define types for IP and bellow
 **/
enum IP_Type {ICMP = 1, TCP = 6, UDP = 17};

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
uint8_t parse_IP(const u_char *packet, size_t *offset);
