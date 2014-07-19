#include "huffmann.hpp"
#include "bitstream.hpp"

#include <iostream>

#include <cassert>

static const char * s_iterator;

Huffmann::Huffmann( const char * const begin, const char * const end )
{
  const char * iterator = begin;
  deserialize( iterator, end );
}

void Huffmann::deserialize( const char *& cur, const char * const end )
{
  if( cur == end ) throw( UnexpectedEnfOfInput() );
  m_leaf = *( cur++ );
  if( cur == end ) throw( UnexpectedEnfOfInput() );
  const unsigned char flags = *( cur++ );
  if( !( flags & 0x80 ) )
  {
    m_child1.reset( new Huffmann() );
    m_child1->deserialize( cur, end );
    m_child0.reset( new Huffmann() );
    m_child0->deserialize( cur, end );
  }
}


bool Huffmann::decode( BitStream& stream, char& out_result ) const
{
  assert( !m_child0 == !m_child1 );

  if( !m_child0 )
  {
    out_result = m_leaf;
    return true;
  }
  bool bit;
  if( !stream.read( bit ) ) return false;

  return ( bit ? m_child1 : m_child0 )->decode( stream, out_result );
}
