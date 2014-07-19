#include "bitstream.hpp"

#include <utility>
#include <stdexcept>

BitStream::BitStream( const char * const begin, const char* const end )
: m_iterator( begin )
, m_end( end )
{
}

bool BitStream::read( bool& out_bit )
{
  if( eof() ) return false;
  out_bit = ( *m_iterator >> m_curBit ) & 1;
  ++m_curBit;
  if( m_curBit == 8 )
  {
    m_curBit = 0;
    ++m_iterator;
  }
  return true;
}
