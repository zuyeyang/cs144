#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ),
                                              bytes_pushed_(0),
                                              bytes_popped_(0),
                                              is_closed_(false),
                                              has_error_(false),
                                              buffer_() {}

void Writer::push( string data )
{
    if (!is_closed_ && !has_error_) {
        uint64_t available = available_capacity();
        uint64_t to_push = std::min(available, static_cast<uint64_t>(data.size()));
        // for (uint64_t i = 0; i < to_push; i++) {
        //     buffer_.push_back(data[i]);
        // }
        buffer_.append(data, 0, to_push);
        bytes_pushed_ += to_push;
    }
}

void Writer::close()
{
    is_closed_ = true;
}

void Writer::set_error()
{
    has_error_ = true;
}

bool Writer::is_closed() const
{
    return is_closed_;
}

uint64_t Writer::available_capacity() const
{
    return capacity_ - buffer_.size();
}

uint64_t Writer::bytes_pushed() const
{
    return bytes_pushed_;
}

std::string_view Reader::peek() const
{
    // string data= std::string(buffer_.begin(), buffer_.end());
  // std::string_view res = std::string_view{&buffer_.front(), 1};
  std::string_view res = std::string_view{&buffer_[0], buffer_.size()};
  return res;
}

bool Reader::is_finished() const
{
    return is_closed_ && buffer_.empty();
}

bool Reader::has_error() const
{
    return has_error_;
}

void Reader::pop( uint64_t len )
{
  uint64_t to_pop = std::min(len, static_cast<uint64_t>(buffer_.size()));
  // for (uint64_t i = 0; i < to_pop; i++) {
  //     buffer_.pop_front();
  // }
  buffer_.erase(0, to_pop);
  bytes_popped_ += to_pop;
}

uint64_t Reader::bytes_buffered() const
{
    return buffer_.size();
}

uint64_t Reader::bytes_popped() const
{
    return bytes_popped_;
}

