#pragma once

#include "byte_stream.hh"

#include <cstdint>
#include <map>
#include <string>
class Reassembler
{
private:
  /*Helper Function*/
  std::map<uint64_t, std::string> buffer_ {}; /* buffer for reassembler*/
  uint64_t first_unacceptable_index_ { 0 };   /* upper bound of the buffer idx*/
  uint64_t first_unassembled_index_ { 0 };    /* end of buffered bytes*/
  uint64_t bytes_pushed { 0 };                /* next index to be popped*/
  uint64_t capacity_ { 0 };                   /* size of buffer*/
  uint64_t closing_bytes_ { 0 };              /* the end of the eof file*/
  bool is_end_received_ { false };            /* If end of file is received*/

public:
  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring, Writer& output );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;
};
