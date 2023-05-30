Checkpoint 4 Writeup
====================

My name: Benson Zu

My SUNet ID: zuyeyang

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: CHATGPT

This checkpoint took me about [4] hours to do. I did not attend the lab session.

Program Structure and Design of the NetworkInterface:
[
## Private Attributes
1. ARP_entry struct and arp_table_: 
The ARP table is a mapping of IP addresses to ARP entries. Each ARP entry 
contains an Ethernet address and a Time To Live value. This table is used
for resolving IP addresses to Ethernet addresses for frame transmission.
2. pending_ip_address_table_: 
This table stores the IP addresses for which an ARP request has been sent
but no reply has been received yet. Each entry in this table also has a 
lifespan.
3. pending_ID_pair_table_: 
This list stores pairs of IP addresses and datagrams that are waiting to be
sent once the Ethernet address is resolved.
4. buffer_: 
This is a queue of Ethernet frames that are ready to be transmitted. All
request will be pushed to this buffer_, and every call of maybe_send()
will extract a single EthernetFrame out.

## Public Methods:
1. send_datagram(): 
First checks the ARP table to see if the Ethernet address for the next
hop is known. If it is, the datagram is encapsulated in an Ethernet frame
and added to the buffer for transmission. If the address is not known,
an ARP request is sent.
2. recv_frame(): 
Depending on the type of frame (IPv4 or ARP), it either returns the
contained datagram or processes the ARP message to update the ARP
table or send an ARP reply.
3. tick(): 
Remove or updates of entries in the ARP table and the pending IP 
address table depending on whether ms_since_last_tick is greater
or smaller than the TTL of each element.
]

Implementation Challenges:
[Distinguishing the ARPMessage as reply or request is challenging, since 
every single hop could be receiver or sender, which lead bugs easily]

Remaining Bugs:
[No Bugs left]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
