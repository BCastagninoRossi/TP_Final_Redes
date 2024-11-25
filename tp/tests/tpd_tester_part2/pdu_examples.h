#include "macros.h" 

/* Buffer array for TCP read mockup */

char *test_buffers_arr[CASES][ARRAY_SIZE] = {
  {
    /* write 0 : correct PDU in one write */
    "client_Amy" "\x02" "2024-11-05 20:08:01" "\x02" "Hi, I need some help" "\x04",
    /* write 1 : invalid ASCII on user */
    "agent_Pauló" "\x02" "2024-11-05 20:08:21" "\x02" "Hi Amy, what can I do for you?""\x04",
    /* write 2:  2 pdu in one write */
    "client_Amy" "\x02" "2024-11-05 20:08:01" "\x02" "There is an issue with my paid subscription service of Netflix. I can't play any content on any device" "\x04" "agent_Paul" "\x02" "2024-11-05 20:08:21" "\x02" "Let me take a look. Please wait a minute, be right back\x04"
  },
  {
    /* write 0 : incomplete PDU in one write */
    "client_Inc" "\x02" "2024-11-05 20:08:",
    /* write 1 : remaining PDU bytes  of a correct PDU*/
    "01""\x02""Hi, I need some help""\x04",
    /* write 3 : one a and half PDU*/
     "agent_Paul" "\x02" "2024-11-05 20:08:21" "\x02" "Hi Amy, what can I do for you?""\x04"  "client_Amy" "\x02" "2024-11-05 20:08:01" "\x02" "Houston, I have a"
  },
  {
    /* write 0 : empty username */
    "\x02" "2024-11-05 20:08:01" "\x02" "Hi, I need some help" "\x04",
    /* write 1 : bad timestamp, not YYYY-mm-dd HH:mm:ss */
    "agent_Paul" "\x02" "24-11-05 20:08:21" "\x02" "Hi Amy, what can I do for you?""\x04",
    /* write 2 :  no fields  */
    "\x04"
  }

};
