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


