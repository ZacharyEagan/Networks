#ifndef FISHNODE_DEF
#define FISHNODE_DEF

int fishnode_l2_receive(void *l2frame);

int fish_l2_send(void *l3frame, fnaddr_t next_hop, int len);
void arp_received(void *l2frame);
void send_arp_request(fnaddr_t l3addr);
void add_arp_entry(fn_l2addr_t l2addr, fnaddr_t addr, int timeout);
void resolve_fnaddr(fnaddr_t addr, arp_resolution_cb cb, void *param);



#endif
