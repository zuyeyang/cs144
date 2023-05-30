#include "router.hh"
#include "address.hh"

#include <cstdint>
#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";
  router_table_.push_back( { route_prefix, prefix_length, next_hop, interface_num } );
}

void Router::route()
{
  // Go through all the interfaces, and route every incoming datagram
  // to the best outgoing interface.
  for ( auto& interface : interfaces_ ) {
    while ( true ) {
      auto datagram = interface.maybe_receive();
      if ( datagram.has_value() ) {
        route_one_datagram( datagram.value() );
      } else {
        break;
      }
    }
  }
}

void Router::route_one_datagram( InternetDatagram& dgram )
{

  auto target_entry = router_table_.end();
  for ( auto i = router_table_.begin(); i != router_table_.end(); i++ ) {
    bool isDefault = i->prefix_length == 0;
    if ( isDefault || ( i->route_prefix ^ dgram.header.dst ) >> ( 32 - i->prefix_length ) == 0 ) {
      /* select the best option*/
      if ( target_entry == router_table_.end() || i->prefix_length > target_entry->prefix_length ) {
        target_entry = i;
      }
    }
  }
  if ( target_entry != router_table_.end() ) {
    if ( dgram.header.ttl <= 1 ) {
      return; // drop
    }
    dgram.header.ttl--;
    dgram.header.compute_checksum();
    const optional<Address> next_hop = target_entry->next_hop;
    AsyncNetworkInterface& interface = interfaces_[target_entry->interface_num];
    if ( next_hop.has_value() ) {
      interface.send_datagram( dgram, next_hop.value() );
    } else {
      interface.send_datagram( dgram, Address::from_ipv4_numeric( dgram.header.dst ) );
    }
  }
}
