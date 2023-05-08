Checkpoint 3 Writeup
====================

My name: Benson Zu

My SUNet ID: zuyeyang

I collaborated with: None

I would like to thank/reward these classmates for their help: None

This checkpoint took me about [10] hours to do. I [did] attend the lab session.

Program Structure and Design of the TCPSender:

TCPSender is responsible for managing the TCP sender's state
and behavior. The key design choices and functionality are as follows:

Variables:
isn_: Initial sequence number.
segment_buffer_:     Buffer for storing TCPSenderMessage objects to be sent.
SYN_SENT_, SYN_ACKED_, FIN_SENT_, FIN_ACKED_: State flags.
next_sequno_:                Next sequence number to be used.
window_size_:                Current window size.
abs_ackno_:                  Absolute acknowledged sequence number.
zero_window_:                Indicates if the window size is zero.
timer_:                      The time clock used for RTO
consecutive_retransmission_: Counter for consecutive retransmissions.
segment_outstanding_:        Queue for TCPSenderMessage that are unacknoed.

When a reader is read or the receiver message is arrived, I will first
update the state flags, and push all the information fitting to the window_size
to the segment_buffer_ deque. Besides, all segment that has been sent from
the segment_buffer_ but has not been acnkonwledged will be stored in the 
segment_outstanding_ queue. 

I also defined a helper class for the timer: TCPTimer is responsible for 
managing the TCP retransmission timer. The key design choices and 
functionality are as follows:

Variables:
base_RTO_:      Initial RTO value.
RTO_:           Upper bounds of RTO.
curr_RTO_:      Current RTO.
open_:          Indicates if the timer is running.
ticking():      Update the timer state based on the elapsed time since the 
                last tick, and check if a retransmission is required.

All private attributes have their public API to update their values, 
I choose to encapsulate the timer class to separate its funcitonality
from the sender. 



Implementation Challenges:
[The RTO aspect presents the greatest challenge. It encompasses a multitude
of interdependent principles, requiring substantial organizational effort.
Furthermore, there are numerous edge cases that demand consideration, such as
the possibility of receiving acknowledgment prior to sending a SYNC message.
To surmount these obstacles, I utilized both test programming and design
programming to guarantee the effectiveness of my approach and account for
all potential edge cases.]

Remaining Bugs:
[None]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
