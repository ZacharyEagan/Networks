#include "fish.h"
#include <assert.h>
#include <signal.h>
#include <string.h>

#include "fishnode.h"

//#define DEBUG
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

   //printf("Setting pointer!\n");
//   fish_l2.fishnode_l2_receive = fishnode_l2_receive;
 //  fish_l2.fish_l2_send = fish_l2_send;
 //  fish_arp.arp_received = arp_received;
 //  fish_arp.send_arp_request = send_arp_request;

///Base functionality
fish_l3.fishnode_l3_receive = fishnode_l3_receive;

fish_l3.fish_l3_send = fish_l3_send;

fish_l3.fish_l3_forward = fish_l3_forward;

///Full Functionality
//fish_fwd.add_fwtable_entry = add_fwtable_entry;
//fish_fwd.remove_fwtable_entry = remove_fwtable_entry;
//fish_fwd.update_fwtable_metric = update_fwtable_metric;
//fish_fwd.longest_prefix_match = longest_prefix_match;

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

   /** \ingroup L3
       \brief Receives a new layer 3 frame from FishnetL3Funcs::fish_l3_receive.
          Decapsulates the frame and forwards it or passes it the L4 as needed.
       \arg \c l3frame A pointer to the received L3 frame.  The only part of
          the frame that can be modified is the TTL header field.  The caller
          will free the memory for the frame as necessary.
       \arg \c len The length of the layer 3 frame, in host byte order
       \return false if the receive is known to have failed and true otherwise.

       This function is responsible for correctly directing packets to the
       higher network layers or to the forwarding engine.  This requires
       following general steps:

       \li If the L3 destination is the node's L3 address, remove the L3 header
       and pass the frame to the L4 code.
       \li If the L3 destination is the broadcast address AND the frame was
       received by this node previously (use the compination of src L3 address
       and packet ID to determine this), drop the frame without sending an FCMP
       message.
       \li Otherwise, if the L3 destination is the broadcast address, the frame
       must be both passed up the network stack and forwarded back out over the
       fishnet with a decremented TTL.
       \li If the L3 addresses isn't the broadcast address and the L3 address
       isn't the node's address, the TTL is decremented and the frame is
       forwarded back over the fishnet.

       This is also the function that calls your implementation of fishnet L3
       protocols, such as DV routing.
      */
int fishnode_l3_receive(void *l3frame, int len)
{
   static uint32_t size = 20;
   static uint32_t *src = NULL;    
   static uint32_t *pid = NULL;   
   static uint32_t count = 0;
   
   l3Header *head3 = (l3Header *)l3frame;
   fnaddr_t mine = fish_getaddress();
   void *l4frame = (void *)((char *)l3frame + sizeof(l3Header));
   int answer = 1;

   if (src == NULL)
   {
      src = (uint32_t *)malloc(size * sizeof(uint32_t));
   }
   if (pid == NULL)
   {
      pid = (uint32_t *)malloc(size * sizeof(uint32_t));
   }

   int repete = 0;
   for (uint32_t cur = count; cur--;)
   {
      if (src[cur] == head3->src && pid[cur] == head3->pid)
      {
         repete = 1;
         break;
      }
   }
   if (!repete)
   {
      if (count > size / 2)
      {
         size *= 2;
         src = (uint32_t *)realloc(src, size * sizeof(uint32_t));
         pid = (uint32_t *)realloc(pid, size * sizeof(uint32_t));
      }
      src[count] = head3->src;
      pid[count] = head3->pid;
      count++;
   }


//-------------------------
//   printf("----------------Recieve-----------------\n");
//   printf("size: %d, count: %d\n", size, count);
//   printf("repete: %d\n", repete);  
//   printf("src: %d, dst: %d, mine: %d, pid: %d, prot: %d, ttl: %d, len: %d\n",head3->src, head3->dst, mine, head3->pid, head3->prot, head3->ttl, len);



   if (head3->dst == mine)
   {  
//      printf("pass up to fish_l4_recieve\n");
//      printf("________________________________________\n\n");
      return fish_l4.fish_l4_receive(l4frame,len - sizeof(l3Header), head3->prot, head3->src);
   }
   else
   {
      if (head3->dst == ALL_NEIGHBORS)
      {
         if (!repete)
         {
//            printf("decriment ttl, forward on and pass up to fish_l4_recieve\n");
            //do stuff
            answer = fish_l4.fish_l4_receive(l4frame,len - sizeof(l3Header), head3->prot, head3->src);
//            printf("________________________________________\n\n");
            head3->ttl--;
            //if (head3->src == mine || head3->ttl > 0)
            if (head3->ttl > 0)
               fish_l3_forward(l3frame, len);
            return answer;             
         }
         else
         {
            //printf("drop it low\n");
            //printf("________________________________________\n\n");
            return false;
         }
      } 
      else
      {
//         printf("decriment ttl forward on\n");
//         printf("________________________________________\n\n");
         head3->ttl--;
          if (head3->ttl > 0) //this is probably the problem if somthing is going wrong
            answer = fish_l3_forward(l3frame, len);
         return answer;       
      }
   }

//   printf("missed somthing returning true\n");
//   printf("________________________________________\n\n");
   return true;	
}



 /** \ingroup L3
       \brief Receives a new L4 frame to be sent over the network.
       \arg \c l4frame A pointer to the L4 frame.  The original frame memory
       must not be modified.  The caller is responsible for freeing any memory
       after this function has completed.
      \arg \c len The length of the L4 frame
      \arg \c dst_addr The L3 address of the final destination for this frame.
      \arg \c proto The fishnet protocol number of the L4 frame
      \arg \c ttl The TTL for the frame, or 0 to use the maximum TTL.  If the TTL is greater than the maximum TTL, set it to the maximum TTL.
       \return false if the send is known to have failed and true otherwise.
      
      This functoin is responsible for encapsulating a L4 frame in a L3 header
      and then forwarding the packet through the fishnet.  Forwarding is
      accomplished by calling the fish_l3_forward() function.
    */
int fish_l3_send(void *l4frame, int len, fnaddr_t dst_addr, 
                  uint8_t proto, uint8_t ttl)
{  
   int ret;
   int len3 = sizeof(l3Header) + len * sizeof(char);
   l3Header *head;
   char *l3Frame = (char *)calloc(1, len3);

   head = (l3Header *)l3Frame;
   memcpy(l3Frame + sizeof(l3Header), l4frame, sizeof(char) * len);

   head->ttl = ttl > MAX_TTL || ttl == 0 ? MAX_TTL : ttl;
   head->prot = proto;

   head->dst = dst_addr;
   head->src = fish_getaddress();
   head->pid = htonl(fish_next_pktid());

//   printf("-------------------------SEND------------------\n");
//   printf("len: %d, ttl: %d, prot: %d, src %d, dst %d, pid %d\n",len, head->ttl, head->prot, head->src, head->dst, head->pid);
//   printf("-----------------------------------------------\n");

   ret = fish_l3.fish_l3_forward(l3Frame, len3);

   free(l3Frame);
   return ret;
}



   /** \ingroup L3
       \brief Takes an already-encapsulated L3 frame, looks up the destination
        in the forwarding table, and passes the frame off to L2.
        \arg \c l3frame A pointer to the L3 frame.  Note that this is different
        than fish_l3_send which takes a L4 frame.  The original frame memory
        must not be modified.  The caller is responsible for freeing any
        memory after this function has completed.
        \arg \c len The length of the L3 frame.
       \return false if the send is known to have failed (e.g., TTL exceeded)
         and true otherwise.
     
     fish_l3_forward is given a L3 encapsulated frame and is responsible for
     delivering it. This requires the following steps:
       \li If the TTL is 0 and the destination is not local, drop the packet and
       generate the correct FCMP error message.
       \li Lookup the L3 destination in the forwarding table.
       \li If there is no route to the destination, drop the frame and generate
       the correct FCMP error.
       \li Use fish_l2_send to send the frame to the next-hop neighbor indicated
       by the forwarding table.


     This function may be called from a number of other places in the code,
     including fish_l3_send and fishnode_l3_receive.
     */
int fish_l3_forward(void *l3frame, int len)
{
	l3Header *head = (l3Header *)l3frame;
   uint32_t src = ntohl(head->src);
   uint32_t dst = ntohl(head->dst);
//   uint32_t pid = ntohl(head->pid);

//printf("--------------------Forward--------------------------\n");
//printf("len: %d, TTL: %d, Prot: %d, PID: %d, SRC: %d, DST: %d\n", len, head->ttl, head->prot, 
//            pid, src, dst);

   //ttl exceded and not a broadcast and not for me and not an fcmp message
   if (head->ttl == 0 && dst != ntohl(fish_getaddress()) && dst != ALL_NEIGHBORS && head->prot != 8)
   {
//printf("Broadcast fcmp 1\n___________________________________________\n\n");
	   fish_fcmp.send_fcmp_response(l3frame, len, 1); //send ttl exceded
      return false;
   }
	fnaddr_t next_hop = fish_fwd.longest_prefix_match(head->dst);
   if (!next_hop && head->prot != 8 && src != ALL_NEIGHBORS && dst != ALL_NEIGHBORS)
   {
//printf("Broadcast fcmp 0\n___________________________________________\n\n");
	   fish_fcmp.send_fcmp_response(l3frame, len, 2);
      return false;
   }
//printf("src: %d, fishaddress: %d, dst: %d, allneigh: %d\n", src, ntohl(fish_getaddress()), dst, ALL_NEIGHBORS);
   if (src == ntohl(fish_getaddress()) && dst == ALL_NEIGHBORS && head->ttl != 0)
   {
//printf("Send to dst: %d\n______________________________________\n\n", ntohl(dst));
      return fish_l2.fish_l2_send(l3frame, head->dst, len);
   }
   if (!next_hop)
   {
//printf("Drop It LOW\n___________________________________________\n\n");
      return false;
   }
//printf("Send to dst: %d\n______________________________________\n\n", ntohl(next_hop));
	return fish_l2.fish_l2_send(l3frame, next_hop, len); 

}







