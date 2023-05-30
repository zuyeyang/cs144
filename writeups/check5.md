Checkpoint 5 Writeup
====================

My name: Benson Zu

My SUNet ID: zuyeyang

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: ChatGPT

This checkpoint took me about [3] hours to do. I [did not] attend the lab session.

Program Structure and Design of the Router:
[
1. add_route() Method
We record new routing rule (a router_entry) to the router_table_.
It takes in four parameters: the route_prefix, prefix_length, next_hop,
and interface_num, and creates a new router_entry with these values
which is then appended to the router_table_.

2.route() Method
For each interface, it uses the maybe_receive() method of 
AsyncNetworkInterface class to consume every incoming datagram
and send it on one of the interfaces to the correct next hop.
The outbound interface and next-hop are chosen based on the longest
prefix match in the router_table_. (Runtime O(n))

The route() method uses the private route_one_datagram() method to
handle the routing of individual datagrams. This method first selects
the best routing rule from the router_table_ based on the longest prefix
match, then it decreases the TTL (time to live) of the datagram and
recomputes its checksum. After this, it sends the datagram to the next hop through the selected interface.]

Implementation Challenges:
[Used to forget update the checksum of the datagram's header 
after updating it, resulting in bad IPV4 datagram]

Remaining Bugs:
[Nothing]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
