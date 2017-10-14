#include "fish.h"
#include <assert.h>
#include <signal.h>
#include <string.h>

#include "fishnode.h"

#define DEBUG
static int noprompt = 0;

void sigint_handler(int sig)
{
   if (SIGINT == sig)
      fish_main_exit();
}

static void keyboard_callback(char *line)
{
   if (0 == strcasecmp("show neighbors", line))
      fish_print_neighbor_table();
   else if (0 == strcasecmp("show arp", line))
      fish_print_arp_table();
   else if (0 == strcasecmp("show route", line))
      fish_print_forwarding_table();
   else if (0 == strcasecmp("show dv", line))
      fish_print_dv_state();
   else if (0 == strcasecmp("quit", line) || 0 == strcasecmp("exit", line))
      fish_main_exit();
   else if (0 == strcasecmp("show topo", line))
      fish_print_lsa_topo();
   else if (0 == strcasecmp("help", line) || 0 == strcasecmp("?", line)) {
      printf("Available commands are:\n"
             "    exit                         Quit the fishnode\n"
             "    help                         Display this message\n"
             "    quit                         Quit the fishnode\n"
             "    show arp                     Display the ARP table\n"
             "    show dv                      Display the dv routing state\n"
             "    show neighbors               Display the neighbor table\n"
             "    show route                   Display the forwarding table\n"
             "    show topo                    Display the link-state routing\n"
             "                                 algorithm's view of the network\n"
             "                                 topology\n"
             "    ?                            Display this message\n"
            );
   }
   else if (line[0] != 0)
      printf("Type 'help' or '?' for a list of available commands.  "
             "Unknown command: %s\n", line);

   if (!noprompt)
      printf("> ");

   fflush(stdout);
}




int main(int argc, char **argv)
{
   printf("Top\n");

   struct sigaction sa;
   int arg_offset = 1;

   /* Verify and parse the command line parameters */
   if (argc != 2 && argc != 3 && argc != 4)
   {
      printf("Usage: %s [-noprompt] <fishhead address> [<fn address>]\n", argv[0]);
      return 1;
   }

   if (0 == strcasecmp(argv[arg_offset], "-noprompt")) {
      noprompt = 1;
      arg_offset++;
   }

   /* Install the signal handler */
   sa.sa_handler = sigint_handler;
   sigfillset(&sa.sa_mask);
   sa.sa_flags = 0;
   if (-1 == sigaction(SIGINT, &sa, NULL))
   {
      perror("Couldn't set signal handler for SIGINT");
      return 2;
   }


/******  my code insertion *****/

   printf("Setting pointer!\n");
   fish_l2.fishnode_l2_receive = fishnode_l2_receive;
   fish_l2.fish_l2_send = fish_l2_send;
   
/********* end my code ********/




   /* Set up debugging output */
#ifdef DEBUG
   fish_setdebuglevel(FISH_DEBUG_ALL);
#else
   fish_setdebuglevel(FISH_DEBUG_NONE);
#endif
   fish_setdebugfile(stderr);

   /* Join the fishnet */
   if (argc-arg_offset == 1)
      fish_joinnetwork(argv[arg_offset]);
   else
      fish_joinnetwork_addr(argv[arg_offset], fn_aton(argv[arg_offset+1]));

   /* Install the command line parsing callback */
   fish_keybhook(keyboard_callback);
   if (!noprompt)
      printf("> ");
   fflush(stdout);

   /* Enable the built-in neighbor protocol implementation.  This will discover
    * one-hop routes in your fishnet.  The link-state routing protocol requires
    * the neighbor protocol to be working, whereas it is redundant with DV.
    * Running them both doesn't break the fishnode, but will cause extra routing
    * overhead */
   fish_enable_neighbor_builtin( 0
         | NEIGHBOR_USE_LIBFISH_NEIGHBOR_DOWN
      );

   /* Enable the link-state routing protocol.  This requires the neighbor
    * protocol to be enabled. */
   fish_enable_lsarouting_builtin(0);

   /* Full-featured DV routing.  I suggest NOT using this until you have some
    * reasonable expectation that your code works.  This generates a lot of
    * routing traffic in fishnet */

   fish_enable_dvrouting_builtin( 0
         | DVROUTING_WITHDRAW_ROUTES
         // | DVROUTING_TRIGGERED_UPDATES
         | RVROUTING_USE_LIBFISH_NEIGHBOR_DOWN
         | DVROUTING_SPLIT_HOR_POISON_REV
         | DVROUTING_KEEP_ROUTE_HISTORY
    );

   /* Execute the libfish event loop */
   fish_main();

   /* Clean up and exit */
   if (!noprompt)
      printf("\n");

   printf("Fishnode exiting cleanly.\n");

   return 0;
}













































  /** \ingroup L2
     \brief Receives a new L3 frame to be sent over the network.
       \arg \c l3frame A pointer to the L3 frame.  The original frame memory
       must not be modified.  The caller is responsible for freeing any memory
       after this function has completed.
      \arg \c next_hop The L3 address of the neighbor this frame should be sent
      to.
      \arg \c len The length of the L3 frame
       \return false if the send is known to have failed and true otherwise.
      
     This function takes to following steps to transmit \c l3frame:
     \li Adds an L2 header to the frame
     \li Uses the ARP cache to resolve the L3 address to L2 (see \ref ARP)
     \li Calls fish_l1_send() to transmit the frame
    */

void callback(fn_l2addr_t addr, void *l2frame)
{
   l2Header *head; //for easy access to dst

   if (FNL2_VALID(addr))  //check address 
   {
      printf("CallBack: Valid Address\n");
      head = (l2Header *)l2frame; //copy dst the easy way
      head->dst[0] = addr.l2addr[0]; 
      head->dst[1] = addr.l2addr[1]; 
      head->dst[2] = addr.l2addr[2]; 
      head->dst[3] = addr.l2addr[3]; 
      head->dst[4] = addr.l2addr[4]; 
      head->dst[5] = addr.l2addr[5]; 
 
   //compute the l2 checksum
      head->chk = in_cksum(l2frame, ntohs(head->len));
printf("headp->len = %d\n", head->len);
printf("chk before ntohs = %x\n", head->chk);
printf("in_cksum before = %d\n", in_cksum(l2frame, head->len));
      //head->chk = ntohs(head->chk);
printf("chk after ntohs = %x\n", head->chk);
printf("in_cksum after = %d\n\n", in_cksum(l2frame, head->len));
	
      fish_l1_send(l2frame); //pas completed frame downstream
   }

   free(l2frame); //free the l2 frame alocated in fish_l2_send
}


//not sure how to know if send fails
int fish_l2_send(void *l3frame, fnaddr_t next_hop, int len)
{
   l2Header head;
   
   fn_l2addr_t carp; //l2 address for src
   carp = fish_getl2address(); //store src
   
   head.src[0] = carp.l2addr[0]; //copy src the easy way
   head.src[1] = carp.l2addr[1];
   head.src[2] = carp.l2addr[2];
   head.src[3] = carp.l2addr[3];
   head.src[4] = carp.l2addr[4];
   head.src[5] = carp.l2addr[5];
	
   head.len = len + sizeof(head);
   head.len = ntohs(head.len);
   
   head.chk = 0x0000;

   void *l2frame = malloc(len + sizeof(head));//allocate mem fo l2
   //freed in call back
   
   //combine the l2 header with the l3 frame
   memcpy(l2frame, &head, sizeof(head));
   memcpy((char *)l2frame + sizeof(head), l3frame, len);
   

   //start the arp cache lookup
   fish_arp.resolve_fnaddr(next_hop, callback, l2frame);
   
   return true;
}
















 /** \ingroup L2
       \brief Receives a new layer 2 frame from the layer 1 code in libfish.
          Decapsulates the frame and passes it up the stack as needed.
       \arg \c l2frame A pointer to the received L2 frame.  The frame must not
       be modified.  The caller will free the memory for the frame as necessary.
       \return false if the receive is known to have failed and true otherwise.

       This function is responsible for correctly directing valid packets to the
       higher network layers.  This requires
       following general steps:
       \li Dropping frames with invalid checksums
       \li Dropping frames that are not destined for this node (verifies the L2
       address.  doesn't consider the L3 address).
       \li Decapsulating frame and passing up the stack
       (fish_l3::fish_l3_receive).

       This is also the function that calls your implementation of fishnet L2
       protocols, such as ARP.
    */
int fishnode_l2_receive(void *l2frame)
{
   printf("pointer called\n");
   //cast header
   l2Header head;
   fn_l2addr_t src, dst;
   fn_l2addr_t zeros;
   fn_l2addr_t ones;
   fn_l2addr_t carp;
   
   memcpy (&head, l2frame, sizeof(head));

   for (int i = 0; i < 6; i++)
   {
      zeros.l2addr[i] = 0;
      ones.l2addr[i] = 0xFF;

      src.l2addr[i] = head.src[i];
      dst.l2addr[i] = head.dst[i];
   }
   
   head.len = ntohs(head.len); //possible removal warrented
   head.chk = ntohs(head.chk); //possible removal warrented  
      

   carp = fish_getl2address();

   
   printf("carp: %x:%x:%x:%x:%x:%x\n", carp.l2addr[0], carp.l2addr[1], 
                                       carp.l2addr[2], carp.l2addr[3],
                                       carp.l2addr[4], carp.l2addr[5]);
                                       
    
   printf("src: %x:%x:%x:%x:%x:%x, ", head.src[0], head.src[1], 
                                head.src[2], head.src[3],
                                 head.src[4], head.src[5]);
   printf("dst: %x:%x:%x:%x:%x:%x, len: %d, chk: %d\n",
            head.dst[0], head.dst[1], head.dst[2], 
            head.dst[3], head.dst[4], head.dst[5],
            head.len, head.chk);
   
   

   //check validity based on checksum
   if (in_cksum(l2frame, head.len))
   {
      return false;
   }
   
   //check addresses for validity
   if (FNL2_EQ(src, zeros)) //src all 0 reserved
   {
      printf("src zero\n");
      return false;
   }
   if (FNL2_EQ(dst, zeros)) //dst all 0 reserved
   {
      printf("dst zero\n");
      return false;
   }
   if (FNL2_EQ(src, ones)) //src all ones reserved
   {
      printf("src ones\n");
      return false;
   }
   

   if (FNL2_EQ(dst, carp) || FNL2_EQ(dst, ones)) //check pack relevent
   {
      printf("for me!\n");  
      fish_l3.fish_l3_receive((void *)((char*)l2frame + sizeof(head)),
                               head.len - sizeof(head));
   }
    

   //return false; // if known fail
   return true; //otherwise
}
