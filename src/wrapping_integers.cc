#include "wrapping_integers.hh"
#include <cstdint>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return zero_point + n;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  const Wrap32 wrapCheckpoint = wrap( checkpoint, zero_point );
  const int32_t diff = this->raw_value_ - wrapCheckpoint.raw_value_;
  if ( diff < 0 && ( checkpoint + diff > checkpoint ) ) {
    return checkpoint + static_cast<uint32_t>( diff );
  }
  return checkpoint + diff;
}
