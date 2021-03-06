#pragma once

#define BROADCAST_ID    0x00
#define MY_ID           0x0f
#define NEXT_ID         0x04
#define OTHER_ID        0x09

#define RETURNED        1
#define MY_BROADCAST    2
#define BROADCAST       3
#define MY_MSG          4
#define OTHER_MSG       5

/*! \brief  This function implements checking source and destination addresses from a given packet. 
  * \brief  The return values are separated into 5 cases.
  * \return RETURNED, MY_BROADCAST, BROADCAST, MY_MSG, OTHER_MSG
  * \details Case 1. RETURNED
  * : Received the message that you sent has returned to you
  * \details Case 2. MY_BROADCAST
  * : Received the broadcast message that you sent has returned to you
  * \details Case 3. BROADCAST
  * : Received a broadcast message that someone sent
  * \details Case 4. MY_MSG
  * : Received a message that someone sent exactly to you
  * \details Case 5. OTHER_MSG
  * : Received a message that someone sent to another */
uint8_t checkAddress(const frame_t* frame);
