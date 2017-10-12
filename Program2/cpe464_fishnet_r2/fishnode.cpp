#include "fish.h"
#include <assert.h>
#include <signal.h>
#include <string.h>

#include "fishnode.h"
#include "checksum.h"


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


/*********  my code ************/

   fish_l2.fishnode_l2_receive = fishnode_l2_receive;

   
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
   //cast header
   l2Header head;
   memcpy (&head, l2frame, sizeof(head));
   head.len = ntohs(head.len); //possible removal warrented
   head.chk = ntohs(head.chk); //possible removal warrented  

   //check validity based on checksum
   if (in_cksum(l2frame, head.len) != head.chk)
      return false;
   
   //check addresses for validity

   

   //return false; // if known fail
   return true; //otherwise
}
