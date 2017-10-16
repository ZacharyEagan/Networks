#ifndef FISHNODE_DEF
#define FISHNODE_DEF

#define ARP_TIMEOUT 180


int fishnode_l2_receive(void *l2frame);
int fish_l2_send(void *l3frame, fnaddr_t next_hop, int len);
void send_arp_request(fnaddr_t l3addr);

void arp_received(void *l2frame);
void add_arp_entry(fn_l2addr_t l2addr, fnaddr_t addr, int timeout);
//void resolve_fnaddr(fnaddr_t addr, arp_resolution_cb cb, void *param);

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
