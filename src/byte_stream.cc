#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity )
  , bytes_pushed_( 0 )
  , bytes_popped_( 0 )
  , is_closed_( false )
  , has_error_( false )
  , buffer_()
{}

void Writer::push( string data )
{
  if ( !is_closed_ && !has_error_ ) { /* make sure it's not closed or in error*/
    uint64_t available = available_capacity();
    uint64_t to_push = std::min( available, static_cast<uint64_t>( data.size() ) );
    if ( to_push > 0 ) { /* only push the dataset for a positive to_push size*/
                         /* only push the data string with upper bounds to_push sizes*/
      buffer_.push_back( data.substr( 0, to_push ) );
      bytes_pushed_ += to_push;
    }
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
  return capacity_ - bytes_pushed_ + bytes_popped_;
}

uint64_t Writer::bytes_pushed() const
{
  return bytes_pushed_;
}

std::string_view Reader::peek() const
{
  /* take a peek at the first string element of our buffer*/
  std::string_view res = std::string_view { &buffer_.front()[0], buffer_.front().size() };
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
  while ( len > 0 && !buffer_.empty() ) {
    uint64_t string_size = static_cast<uint64_t>( buffer_.front().size() );
    uint64_t to_pop = std::min( len, string_size );
    /* if the to_pop size is greater than an element string, pop the whole string
    otherwise, erasing the corrsponding amount of bytes from the string and keep
    the element*/
    if ( to_pop < string_size ) {
      buffer_.front().erase( 0, to_pop );
    } else {
      buffer_.pop_front();
    }

    len -= to_pop;
    bytes_popped_ += to_pop;
  }
}

uint64_t Reader::bytes_buffered() const
{

  return bytes_pushed_ - bytes_popped_;
}

uint64_t Reader::bytes_popped() const
{
  return bytes_popped_;
}
