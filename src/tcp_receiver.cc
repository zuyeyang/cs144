#include "tcp_receiver.hh"
#include "wrapping_integers.hh"
#include <cstdint>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  if ( message.SYN ) {
    this->zero_point = message.seqno;
    reassembler.insert( 0, message.payload, message.FIN, inbound_stream );
    return;
  }

  if ( this->zero_point.has_value() ) {
    const uint64_t absolute_seqno = message.seqno.unwrap( this->zero_point.value(), inbound_stream.bytes_pushed() );
    reassembler.insert( absolute_seqno - 1, message.payload, message.FIN, inbound_stream );
  }
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  TCPReceiverMessage res;

  res.window_size = ( inbound_stream.available_capacity() >= UINT16_MAX )
                      ? UINT16_MAX
                      : static_cast<uint16_t>( inbound_stream.available_capacity() );

  if ( this->zero_point.has_value() ) {
    res.ackno
      = Wrap32::wrap( inbound_stream.bytes_pushed() + inbound_stream.is_closed() + 1, this->zero_point.value() );
  }

  return res;
}
