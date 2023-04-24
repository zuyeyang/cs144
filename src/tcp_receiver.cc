#include "tcp_receiver.hh"
#include <cstdint>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  if ( !zero_point.has_value() ) {
    if ( message.SYN ) {
      zero_point = message.seqno;
      reassembler.insert( 0, message.payload, message.FIN, inbound_stream );
      ackno = Wrap32::wrap( message.sequence_length(), zero_point.value() );
      window_size = inbound_stream.available_capacity() - reassembler.bytes_pending();
    }
    return;
  }

  uint64_t first_index = message.seqno.unwrap( zero_point.value(), reassembler.bytes_pending() );
  reassembler.insert( first_index, message.payload, message.FIN, inbound_stream );
  ackno = ackno.value() + message.sequence_length();
  window_size = inbound_stream.available_capacity() - reassembler.bytes_pending();
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  struct TCPReceiverMessage res;
  res.ackno = ackno;
  res.window_size = inbound_stream.available_capacity();
  return res;
}
