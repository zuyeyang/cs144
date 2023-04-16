#include "reassembler.hh"
#include "byte_stream.hh"
#include <cstdint>

using namespace std;

void Reassembler::insert( uint64_t first_index, std::string data, bool is_last_substring, Writer& output )
{
  // Handle the end of the stream
  if ( is_last_substring ) {
    is_end_received_ = true;
    closing_bytes_ = first_index + data.size();
  }
  if ( capacity_ == 0 ) {
    capacity_ = output.available_capacity();
  }
  available_capacity_ = output.available_capacity();
  if ( available_capacity_ == 0 ) {
    return;
  }
  first_unpopped_index_ = output.bytes_pushed();
  first_unassembled_index_ = first_unpopped_index_ + bytes_pending();
  first_unacceptable_index_ = first_unpopped_index_ + capacity_;

  // Discard bytes that lie beyond the stream's available capacity
  if ( first_index >= first_unacceptable_index_ ) {
    return;
  }
  if ( first_index + data.size() < first_unpopped_index_ ) {
    return;
  }

  // Truncate data if it extends beyond the available capacity
  if ( first_index + data.size() > first_unacceptable_index_ ) {
    data.resize( first_unacceptable_index_ - first_index );
  }
  if ( first_index < first_unpopped_index_ ) {
    data = data.substr( first_unpopped_index_ - first_index );
    first_index = first_unpopped_index_;
  }

  auto it = pending_bytes_.lower_bound( first_index );
  std::string left = "";
  std::string right = "";
  // Merge with previous data if overlapping or adjacent
  if ( it != pending_bytes_.begin() ) {
    auto prev_it = std::prev( it );
    if ( prev_it->first + prev_it->second.size() >= first_index ) {
      if ( prev_it->first + prev_it->second.size() >= first_index + data.size() ) {
        /*prev_it includes new data*/
        data = prev_it->second;
      } else {
        /* prev_it inlcudes begining part of new data*/
        left = prev_it->second.substr( 0, first_index - prev_it->first );
        data = left + data;
      }
      first_index = prev_it->first;
      it = pending_bytes_.erase( prev_it );
    }
  }

  // Merge with next data if overlapping or adjacent
  while ( it != pending_bytes_.end() && first_index + data.size() >= it->first ) {
    if ( it->first + it->second.size() > first_index + data.size() ) {
      data = data.substr( 0, it->first - first_index ) + it->second;
    }
    it = pending_bytes_.erase( it );
  }

  pending_bytes_[first_index] = data;

  // Write the continuous bytes in the pending bytes to the output
  auto it_push = pending_bytes_.begin();
  while ( it_push != pending_bytes_.end() && it_push->first == next_expected_byte_ ) {
    output.push( it_push->second );
    next_expected_byte_ += it_push->second.size();
    it_push = pending_bytes_.erase( it_push );
  }

  if ( next_expected_byte_ == closing_bytes_ && is_end_received_ && pending_bytes_.empty() ) {
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  uint64_t total_bytes = 0;
  for ( const auto& entry : pending_bytes_ ) {
    total_bytes += entry.second.size();
  }
  return total_bytes;
}

// // Store the new bytes in the pending_bytes_ map and handle overlapping substrings
// for ( size_t i = 0; i < data.size(); ++i ) {
//   uint64_t idx = first_index + i;
//   if ( idx < next_expected_byte_ ) {
//     continue;
//   }
//   pending_bytes_[idx] = data[i];
// }

// // Write the known bytes in order to the output
// while ( pending_bytes_.count( next_expected_byte_ ) && next_expected_byte_ < first_unacceptable_index_
//         && first_unacceptable_index_ != first_unassembled_index_ ) {
//   output.push( std::string( 1, pending_bytes_[next_expected_byte_] ) );
//   pending_bytes_.erase( next_expected_byte_ );
//   ++next_expected_byte_;
// }