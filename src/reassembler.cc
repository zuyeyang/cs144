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

  // Store the new bytes in the pending_bytes_ map and handle overlapping substrings
  for ( size_t i = 0; i < data.size(); ++i ) {
    uint64_t idx = first_index + i;
    if ( idx < next_expected_byte_ ) {
      continue;
    }
    pending_bytes_[idx] = data[i];
  }

  // Write the known bytes in order to the output
  while ( pending_bytes_.count( next_expected_byte_ ) && next_expected_byte_ < first_unacceptable_index_
          && first_unacceptable_index_ != first_unassembled_index_ ) {
    output.push( std::string( 1, pending_bytes_[next_expected_byte_] ) );
    pending_bytes_.erase( next_expected_byte_ );
    ++next_expected_byte_;
  }
  if ( next_expected_byte_ == closing_bytes_ && is_end_received_ && pending_bytes_.empty() ) {
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return pending_bytes_.size();
}