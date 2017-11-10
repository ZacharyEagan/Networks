#ifndef FISHNODE_DEF
#define FISHNODE_DEF

#define ARP_TIMEOUT 180

///Base functionality
//fish_l3.fishnode_l3_receive
//fish_l3.fish_l3_send
//fish_l3.fish_l3_forward

///Full Functionality
//fish_fwd.add_fwtable_entry
//fish_fwd.remove_fwtable_entry
//fish_fwd.update_fwtable_metric
//fish_fwd.longest_prefix_match

///Extended Functionality
//Forwarding Table 
//Advanced DV Routing


typedef struct l2Header
{
   uint8_t dst[6];
   uint8_t src[6];
   uint16_t chk;
   uint16_t len;
} __attribute__((packed)) l2Header;

typedef struct l3Header
{
   uint8_t ttl;
   uint8_t prot;
   uint8_t pid[4];
   uint8_t src[4];
   uint8_t dst[4];
} __attribute__((packed)) l3Header;

typedef struct arp_header
{
   uint32_t type;
   fnaddr_t l3;
   fn_l2addr_t l2;
} __attribute__((packed)) arp_header;


#endif
