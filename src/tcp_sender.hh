#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include "wrapping_integers.hh"
#include <cstddef>
#include <cstdint>
#include <vector>
class TCPTimer
{
private:
  unsigned int base_RTO_;    /* Init RTO*/
  unsigned int RTO_;         /* Track the upper bounds of RTO*/
  unsigned int curr_RTO_ {}; /* current RTO*/
  bool open_ {};

public:
  //! Initialize a TCP retransmission timer
  explicit TCPTimer( const uint64_t initial_RTO_ms )
    : base_RTO_( initial_RTO_ms ), RTO_( initial_RTO_ms ), open_( true )
  {}

  bool isOpen() const { return open_; }
  unsigned int RTO() const { return RTO_; }
  void doubleRTO() { RTO_ *= 2; }
  void resetRTO()
  {
    RTO_ = base_RTO_;
    curr_RTO_ = 0;
  }
  void start()
  {
    open_ = true;
    curr_RTO_ = 0;
  }
  void close()
  {
    open_ = false;
    curr_RTO_ = 0;
  }
  bool ticking( const size_t& ms_since_last_tick )
  {
    if ( isOpen() ) {
      if ( ms_since_last_tick > RTO_ - curr_RTO_ ) { /* retransmit */
        curr_RTO_ = RTO_;                            /* reset curr_RTO_*/
      } else {                                       /* update curr_RTO_*/
        curr_RTO_ += ms_since_last_tick;
      }
      if ( curr_RTO_ >= RTO_ && open_ ) {
        curr_RTO_ = 0;
        return true;
      }
    }
    return false;
  }
};

class TCPSender
{
private:
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

  /* storage of TCPSenderMessage*/
  std::deque<TCPSenderMessage> mutable segment_buffer_ {}; /* TCPSender is going to send*/

  /* State check*/
  bool SYN_SENT_ { false };
  bool SYN_ACKED_ { false };
  bool FIN_SENT_ { false };
  bool FIN_ACKED_ { false };

  /* Absolute sequece number*/
  uint64_t next_sequno_ { 0 };
  uint64_t window_size_ { 0 };
  uint64_t abs_ackno_ { 0 };
  bool zero_window_ { false };

  /* RTO states */
  TCPTimer timer_;
  size_t consecutive_retransmission_ { 0 };
  std::queue<TCPSenderMessage> segment_outstanding_ {}; /*TCPSender may resend*/

public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};
