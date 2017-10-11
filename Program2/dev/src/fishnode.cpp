#include "fishnode.h"


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



int fishnode_l2_recieve(void *l2frame)
{
	   

   return false; // if known fail
   return true; //otherwise
}
