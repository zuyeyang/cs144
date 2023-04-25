Checkpoint 2 Writeup
====================

My name: Benson Zu  

My SUNet ID: zuyeyang

I collaborated with: none

I would like to thank/reward these classmates for their help: rqzhang

This lab took me about [2] hours to do. I [did/did not] attend the lab session.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
The TCPReceiver class contains the following methods:
* private attribute: std::optional<Wrap32> zero_point {};
    * used to record the initial sequence number of the stream.
* public function receive: 
    * Processes the received TCPSenderMessage and updates the reassembler and 
    inbound_stream accordingly. It first checks if the message has the SYN flag
    set, and if so, sets the initial sequence number (zero_point). If 
    zero_point has a value, it calculates the absolute sequence number and 
    updates the reassembler.
* public function send: 
    * Constructs and returns a TCPReceiverMessage containing the receiver's
    window size and acknowledgement number (ackno), based on the current state
    of the inbound_stream. The window size is set to the minimum of UINT16_MAX
    or the available capacity of the inbound_stream, and the ackno is
    calculated using the wrapping arithmetic operations provided by the
    Wrap32 class.

The Wrap32 class:
* public function wrap: 
    * It returns a new Wrap32 object, which is the result of adding n to 
    zero_point using wrapping arithmetic. This operation is used for converting
    an absolute sequence number to a wrapped value relative to a zero_point.
* public function  unwrap: 
    * This method takes a Wrap32 value zero_point and a uint64_t value 
    checkpoint as arguments. It calculates the difference between the 
    current Wrap32 object's raw_value_ and the wrapped value of checkpoint
    with respect to zero_point. The method checks the edge case that 
    difference is negative but adding the difference to uint64_t checkpoint
    results in a value greater than checkpoint (for example, casting 
    int32_t diff = -1 to uint64_t will leads to (INT64MAX-1). If this 
    condition is met, the difference is cast to a uint32_t before being 
    added to checkpoint. The method returns the unwrapped value, which is 
    the result of adding the difference to checkpoint. 


Implementation Challenges:
Conversion between uint64_t and uint32_t is challenging.

Remaining Bugs:
None

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I made an extra test I think will be helpful in catching bugs: [describe where to find]
