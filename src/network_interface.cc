#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"
#include "ethernet_header.hh"
#include "ipv4_datagram.hh"
#include "parser.hh"
#include <optional>
using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  EthernetFrame res;

  const uint32_t next_hop_ip_addr = next_hop.ipv4_numeric();
  const auto& arp_e = arp_table_.find( next_hop_ip_addr );
  if ( arp_e != arp_table_.end() ) {

    res.header = {
      .dst = arp_e->second.ethernet_address_,
      .src = ethernet_address_,
      .type = EthernetHeader::TYPE_IPv4,
    };
    ;
    res.payload = serialize( dgram );
    buffer_.push( res );
  } else {
    /* send a ARP request*/
    if ( pending_ip_address_table_.find( next_hop_ip_addr ) == pending_ip_address_table_.end() ) {
      ARPMessage msg;
      msg.opcode = ARPMessage::OPCODE_REQUEST;
      msg.sender_ethernet_address = ethernet_address_;
      msg.sender_ip_address = ip_address_.ipv4_numeric();
      msg.target_ethernet_address = {};
      msg.target_ip_address = next_hop_ip_addr;

      res.header = {
        .dst = ETHERNET_BROADCAST,
        .src = ethernet_address_,
        .type = EthernetHeader::TYPE_ARP,
      };
      res.payload = serialize( msg );
      buffer_.push( res );
      pending_ip_address_table_[next_hop_ip_addr] = lifespan_pending_ip_address_;
    }
    /* if the same ip request has been sent */
    pending_ID_pair_table_.emplace_back( next_hop, dgram );
  }
}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  /* ignore any frames not destined for the network interface*/
  if ( frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST ) {
    return nullopt;
  }

  /* send the datagram right away if it's a IP address*/
  if ( frame.header.type == EthernetHeader::TYPE_IPv4 ) {
    InternetDatagram res_ID;
    if ( !parse( res_ID, frame.payload ) ) {
      return nullopt;
    }
    return res_ID;
  }
  /* send datarame to APR*/
  if ( frame.header.type == EthernetHeader::TYPE_ARP ) {
    ARPMessage msg;
    if ( !parse( msg, frame.payload ) ) {
      return nullopt;
    }
    bool isRequest
      = msg.opcode == ARPMessage::OPCODE_REQUEST && msg.target_ip_address == ip_address_.ipv4_numeric();
    bool isReply = msg.opcode == ARPMessage::OPCODE_REPLY && msg.target_ethernet_address == ethernet_address_;
    if ( isRequest ) {
      ARPMessage reply;
      reply.opcode = ARPMessage::OPCODE_REPLY;
      reply.sender_ethernet_address = ethernet_address_;
      reply.sender_ip_address = ip_address_.ipv4_numeric();
      reply.target_ethernet_address = msg.sender_ethernet_address;
      reply.target_ip_address = msg.sender_ip_address;

      EthernetFrame res_EF;
      res_EF.header = {
        .dst = msg.sender_ethernet_address,
        .src = ethernet_address_,
        .type = EthernetHeader::TYPE_ARP,
      };
      res_EF.payload = serialize( reply );
      buffer_.push( res_EF );
      extract_info_from_ARPMessage( msg );
    }
    if ( isReply ) {
      extract_info_from_ARPMessage( msg );
    }
  }

  return nullopt;
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  /* update the arp table*/
  for ( auto i = arp_table_.begin(); i != arp_table_.end(); ) {
    if ( i->second.TTL > ms_since_last_tick ) {
      i->second.TTL -= ms_since_last_tick;
      i++;
    } else {
      i = arp_table_.erase( i );
    }
  }
  /* update the pending tables */
  for ( auto j = pending_ip_address_table_.begin(); j != pending_ip_address_table_.end(); ) {
    if ( j->second > ms_since_last_tick ) {
      j->second -= ms_since_last_tick;
      j++;
    } else {
      /* resend a msg*/
      ARPMessage msg;
      EthernetFrame res;
      msg.opcode = ARPMessage::OPCODE_REQUEST;
      msg.sender_ethernet_address = ethernet_address_;
      msg.sender_ip_address = ip_address_.ipv4_numeric();
      msg.target_ethernet_address = {};
      msg.target_ip_address = j->first;

      res.header = {
        .dst = ETHERNET_BROADCAST,
        .src = ethernet_address_,
        .type = EthernetHeader::TYPE_ARP,
      };
      res.payload = serialize( msg );
      buffer_.push( res );
      j->second = lifespan_pending_ip_address_;
    }
  }
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if ( !buffer_.empty() ) {
    const EthernetFrame res = buffer_.front();
    buffer_.pop();
    return res;
  }
  return nullopt;
}

/* Helper Functions*/
void NetworkInterface::extract_info_from_ARPMessage( ARPMessage& msg )
{
  arp_table_[msg.sender_ip_address] = { msg.sender_ethernet_address, lifespan_arp_entry_ };
  for ( auto i = pending_ID_pair_table_.begin(); i != pending_ID_pair_table_.end(); ) {
    if ( i->first.ipv4_numeric() == msg.sender_ip_address ) {
      send_datagram( i->second, i->first );
      i = pending_ID_pair_table_.erase( i );
    } else {
      i++;
    }
  }
  pending_ip_address_table_.erase( msg.sender_ip_address );
}