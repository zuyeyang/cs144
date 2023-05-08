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
  // Your code here.
  return this->sequence_numbers_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return this->consecutive_retransmission_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  if ( !segment_buffer_.empty() ) {
    const TCPSenderMessage res = segment_buffer_.front();
    segment_buffer_.pop_front();
    return res;
  }
  return {};
}
void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  if ( outbound_stream.has_error() ) {
    return;
  }
  TCPSenderMessage msg;
  if ( next_sequno_ == 0 ) {
    msg.SYN = true;
    SYN_SENT_ = true;
    send_none_empty_message( msg );
    return;
  }

  if ( next_sequno_ == sequence_numbers_in_flight() ) {
    return;
  }

  /* send all string data*/
  /* if window_size_ == 0, then we treat it as 1*/
  uint64_t effective_w_size = window_size_ == 0 ? 1 : window_size_;
  uint64_t reminders = effective_w_size - ( next_sequno_ - ackno_ );
  while ( reminders > 0 ) {
    TCPSenderMessage new_msg;
    if ( outbound_stream.is_finished() && !FIN_SENT_ ) {
      /* send a finish msg*/
      new_msg.FIN = true;
      FIN_SENT_ = true;
      send_none_empty_message( new_msg );
      return;
    }
    if ( outbound_stream.is_finished() ) {
      /* we have sent the FIN msg*/
      return;
    }
    /* load the data into the msg*/
    size_t size = std::min( reminders, TCPConfig::MAX_PAYLOAD_SIZE );
    read( outbound_stream, size, new_msg.payload );
    if ( new_msg.sequence_length() < effective_w_size && outbound_stream.is_finished() ) {
      /* this is a finish data msg*/
      new_msg.FIN = true;
      FIN_SENT_ = true;
    }
    if ( new_msg.sequence_length() == 0 ) {
      return;
    }
    send_none_empty_message( new_msg );
    reminders = effective_w_size - ( next_sequno_ - ackno_ );
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  TCPSenderMessage msg;
  msg.seqno = Wrap32::wrap( next_sequno_, isn_ );
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  if ( !msg.ackno.has_value() ) {
    return;
  }
  uint64_t abs_ackno = msg.ackno.value().unwrap( isn_, next_sequno_ );
  if ( abs_ackno - next_sequno_ > 0 ) {
    return;
  }
  if ( abs_ackno <= ackno_ ) {
    return;
  }
  if ( abs_ackno > 0 && !SYN_ACKED_ ) {
    SYN_ACKED_ = true;
  }

  if ( abs_ackno == next_sequno_ && FIN_SENT_ ) {
    FIN_ACKED_ = true;
  }
  window_size_ = msg.window_size;
  ackno_ = abs_ackno;

  timer_.resetRTO();
  consecutive_retransmission_ = 0;

  /* update the outstanding queue*/
  while ( !segment_outstanding_.empty() ) {
    const auto& sender_msg = segment_outstanding_.front();
    if ( ( ackno_ - sender_msg.seqno.unwrap( isn_, ackno_ ) ) >= sender_msg.sequence_length() ) {
      sequence_numbers_in_flight_ -= sender_msg.sequence_length();
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
    if ( window_size_ > 0 || ( window_size_ == 0 && !SYN_ACKED_ ) ) {
      consecutive_retransmission_++;
      timer_.doubleRTO();
    }
    if ( !timer_.isOpen() ) {
      timer_.start();
    }
  } else {
    /* empty segment outstanding */
    timer_.close();
  }
}

/* Helper Function (Private)*/
void TCPSender::send_none_empty_message( TCPSenderMessage& msg )
{
  msg.seqno = Wrap32::wrap( next_sequno_, isn_ );
  next_sequno_ += msg.sequence_length();
  sequence_numbers_in_flight_ += msg.sequence_length();

  segment_buffer_.push_back( msg );
  segment_outstanding_.push( msg );

  if ( !timer_.isOpen() ) {
    timer_.start();
  }
};