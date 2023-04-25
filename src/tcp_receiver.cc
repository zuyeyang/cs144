#include "tcp_receiver.hh"
#include "wrapping_integers.hh"
#include <cstdint>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  if ( message.SYN && this->zero_point.has_value() ) {
    return;
  }
  if ( message.SYN ) {
    this->zero_point = message.seqno;
    reassembler.insert( 0, message.payload, message.FIN, inbound_stream );
    return;
  }
  uint64_t absolute_seqno = 0;
  if ( this->zero_point.has_value() ) {
    absolute_seqno = message.seqno.unwrap( this->zero_point.value(), inbound_stream.bytes_pushed() );
    reassembler.insert( absolute_seqno - 1, message.payload, message.FIN, inbound_stream );
  }
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  TCPReceiverMessage res;
  if ( inbound_stream.available_capacity() >= UINT16_MAX ) {
    res.window_size = UINT16_MAX;
  } else {
    res.window_size = uint16_t( inbound_stream.available_capacity() );
  }

  if ( this->zero_point.has_value() ) {
    res.ackno
      = Wrap32::wrap( inbound_stream.bytes_pushed() + inbound_stream.is_closed() + 1, this->zero_point.value() );
  }
  return res;
}
