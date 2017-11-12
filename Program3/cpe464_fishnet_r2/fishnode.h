#ifndef FISHNODE_DEF
#define FISHNODE_DEF

#define ARP_TIMEOUT 180

///Base functionality
// fish_l3.fishnode_l3_receive
//fish_l3.fish_l3_send
//fish_l3.fish_l3_forward
int fishnode_l3_receive(void *l3frame, int len);
int fish_l3_send(void * l4frame, int len, fnaddr_t dst_addr, uint8_t proto, uint8_t ttl);
int fish_l3_forward(void *l3frame, int len);

///Full Functionality
//fish_fwd.add_fwtable_entry
//fish_fwd.remove_fwtable_entry
//fish_fwd.update_fwtable_metric
//fish_fwd.longest_prefix_match

///Extended Functionality
//Forwarding Table 
//Advanced DV Routing

typedef struct pair
{
   fnaddr_t src;
   fnaddr_t pid;
} pair;

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
   uint32_t pid;
   fnaddr_t src;
   fnaddr_t dst;
} __attribute__((packed)) l3Header;

typedef struct arp_header
{
   uint32_t type;
   fnaddr_t l3;
   fn_l2addr_t l2;
} __attribute__((packed)) arp_header;


#endif
