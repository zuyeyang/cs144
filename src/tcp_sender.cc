#include "tcp_sender.hh"
#include "tcp_config.hh"
#include "tcp_sender_message.hh"
#include "wrapping_integers.hh"

#include <cstddef>
#include <cstdint>
#include <random>
#include <sys/types.h>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) )
  , initial_RTO_ms_( initial_RTO_ms )
  , timer_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return next_sequno_ - abs_ackno_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return this->consecutive_retransmission_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  if ( !segment_buffer_.empty() ) {
    const TCPSenderMessage res = segment_buffer_.front();
    segment_buffer_.pop_front();
    return res;
  }
  return {};
}
void TCPSender::push( Reader& outbound_stream )
{
  if ( outbound_stream.has_error() || FIN_SENT_ ) {
    return;
  }
  const uint64_t effective_w_size = window_size_ == 0 ? 1 : window_size_;
  while ( sequence_numbers_in_flight() < effective_w_size ) {
    TCPSenderMessage msg = TCPSenderMessage();
    if ( !SYN_SENT_ ) {
      msg.SYN = true;
      msg.seqno = isn_;
      SYN_SENT_ = true;
    }
    const uint64_t len
      = min( effective_w_size - sequence_numbers_in_flight() - msg.sequence_length(), TCPConfig::MAX_PAYLOAD_SIZE );
    msg.seqno = Wrap32::wrap( next_sequno_, isn_ );
    read( outbound_stream, len, msg.payload );
    if ( !FIN_SENT_ && outbound_stream.is_finished() ) {
      if ( sequence_numbers_in_flight() + msg.sequence_length() < effective_w_size ) {
        msg.FIN = true;
        FIN_SENT_ = true;
      }
    }
    if ( msg.sequence_length() > 0 ) {
      segment_buffer_.push_back( msg );
      segment_outstanding_.push( msg );
      next_sequno_ += msg.sequence_length();

      if ( !timer_.isOpen() ) {
        timer_.start();
      }
    } else {
      break;
    }
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  TCPSenderMessage msg;
  msg.seqno = Wrap32::wrap( next_sequno_, isn_ );
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  if ( msg.window_size == 0 ) {
    window_size_ = 1;
    zero_window_ = true;
  } else {
    window_size_ = msg.window_size;
    zero_window_ = false;
  }
  if ( !SYN_SENT_ || !msg.ackno.has_value() ) {
    return;
  }
  const uint64_t msg_abs_ackno = msg.ackno.value().unwrap( isn_, abs_ackno_ );
  if ( msg_abs_ackno > abs_ackno_ && msg_abs_ackno <= next_sequno_ ) {
    abs_ackno_ = msg_abs_ackno;
  } else {
    return;
  }

  if ( !SYN_ACKED_ && SYN_SENT_ && abs_ackno_ > 0 ) {
    SYN_ACKED_ = true;
  }
  if ( !FIN_ACKED_ && FIN_SENT_ && msg_abs_ackno == next_sequno_ ) {
    FIN_ACKED_ = true;
  }

  timer_.resetRTO();
  consecutive_retransmission_ = 0;
  while ( !segment_outstanding_.empty() ) {

    const auto& sender_msg = segment_outstanding_.front();

    if ( ( abs_ackno_ - sender_msg.seqno.unwrap( isn_, abs_ackno_ ) ) >= sender_msg.sequence_length() ) {
      segment_outstanding_.pop();
    } else {
      break;
    }
  }
  if ( segment_outstanding_.empty() ) {
    timer_.close();
  } else {
    timer_.start();
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick )
{
  if ( !timer_.ticking( ms_since_last_tick ) ) {
    return;
  }

  if ( !segment_outstanding_.empty() ) {
    segment_buffer_.push_front( segment_outstanding_.front() );
    if ( !zero_window_ ) {
      consecutive_retransmission_++;
      timer_.doubleRTO();
    }
    if ( !timer_.isOpen() ) {
      timer_.start();
    }
  } else {
    timer_.close();
  }
}
