#include "reassembler.hh"
#include "byte_stream.hh"
#include <cstdint>

using namespace std;

void Reassembler::insert( uint64_t first_index, std::string data, bool is_last_substring, Writer& output )
{
  /*Handle the end of the stream*/
  if ( is_last_substring ) {
    is_end_received_ = true;
    closing_bytes_ = first_index + data.size();
  }
  /* empty space writer is not allowed*/
  if ( output.available_capacity() == 0 ) {
    return;
  }
  /*Initialize the fixed capacity*/
  if ( capacity_ == 0 ) {
    capacity_ = output.available_capacity();
  }

  /* Update the three boundary index*/
  first_unassembled_index_ = first_unpopped_index_ + bytes_pending();
  first_unacceptable_index_ = first_unpopped_index_ + capacity_;

  /* If the input is out of boundary or it's already pushed, return*/
  if ( first_index >= first_unacceptable_index_ || first_index + data.size() < first_unpopped_index_ ) {
    return;
  }

  /*Truncate data if it extends beyond the available capacity*/
  if ( first_index + data.size() > first_unacceptable_index_ ) {
    data.resize( first_unacceptable_index_ - first_index );
  }
  if ( first_index < first_unpopped_index_ ) {
    data = data.substr( first_unpopped_index_ - first_index );
    first_index = first_unpopped_index_;
  }
  /* If overlap happens, merge with existing buffer element*/
  auto it = buffer_.lower_bound( first_index );
  std::string left;
  std::string right;
  /*Merge with previous data if overlapping or adjacent*/
  if ( it != buffer_.begin() ) {
    auto prev_it = std::prev( it );
    uint64_t prev_it_end_idx = prev_it->first + prev_it->second.size();
    if ( prev_it_end_idx >= first_index ) {
      if ( prev_it_end_idx >= first_index + data.size() ) { /*prev_it includes new data*/
        data = prev_it->second;
      } else {
        /* prev_it inlcudes begining part of new data*/
        left = prev_it->second.substr( 0, first_index - prev_it->first );
        data = left + data;
      }
      /* Merge by updating new data index*/
      first_index = prev_it->first;
      it = buffer_.erase( prev_it );
    }
  }

  /* Merge all available element with next data if overlapping or adjacent */
  while ( it != buffer_.end() && first_index + data.size() >= it->first ) {
    if ( it->first + it->second.size() > first_index + data.size() ) {
      data = data.substr( 0, it->first - first_index ) + it->second;
    }
    it = buffer_.erase( it );
  }
  /* Assign the data*/
  buffer_[first_index] = data;

  /*Write the continuous bytes in the pending bytes to the output*/
  auto it_push = buffer_.begin();
  while ( it_push != buffer_.end() && it_push->first == first_unpopped_index_ ) {
    output.push( it_push->second );
    first_unpopped_index_ += it_push->second.size();
    it_push = buffer_.erase( it_push );
  }
  /* close the bytestream if finish all the push including eof*/
  if ( first_unpopped_index_ == closing_bytes_ && is_end_received_ && buffer_.empty() ) {
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  uint64_t total_bytes = 0;
  for ( const auto& entry : buffer_ ) {
    total_bytes += entry.second.size();
  }
  return total_bytes;
}