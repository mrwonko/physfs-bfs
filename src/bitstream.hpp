#pragma once

#include <vector>

class BitStream
{
public:
  BitStream( const char * const begin, const char* const end );
  ~BitStream() = default;
  BitStream( const BitStream& rhs ) = default;
  BitStream& operator=( const BitStream& rhs ) = delete;

  /**
  @param out_bit 0 iff next bit is 0
  @return if already eof(), thus no reading possible
  **/
  bool read( bool& out_bit );
  bool eof() const { return m_iterator == m_end; }

private:
  const char * m_iterator;
  const char * const m_end;
  unsigned char m_curBit = 0;
};
