#include "stringpool.hpp"
#include "bfsformat.hpp"
#include "bitstream.hpp"
#include "huffmann.hpp"

#include <physfs.h>

#include <utility>
#include <set>
#include <cassert>
#include <algorithm>
#include <cstdint>

static BFSStringPoolHeader readHeader( PHYSFS_Io& io, unsigned int pos )
{
  BFSStringPoolHeader header;
  if( !io.seek( &io, pos ) || io.read( &io, &header, sizeof( header ) ) != sizeof( header ) )
  {
    throw UnexpectedEnfOfInput();
  }
  // Byte swap
  header.end = PHYSFS_swapULE32( header.end );
  header.offsetsOffset = PHYSFS_swapULE32( header.offsetsOffset );
  header.uncompressedSizesOffset = PHYSFS_swapULE32( header.uncompressedSizesOffset );
  header.huffmannTreeOffset = PHYSFS_swapULE32( header.huffmannTreeOffset );
  header.compressedStringsOffset = PHYSFS_swapULE32( header.compressedStringsOffset );
  // Adjust offsets
  header.end += pos;
  header.offsetsOffset += pos;
  header.uncompressedSizesOffset += pos;
  header.huffmannTreeOffset += pos;
  header.compressedStringsOffset += pos;
  return header;
}

StringPool::StringPool( StringPool&& rhs )
: m_pool( std::move( rhs.m_pool ) )
{
}

StringPool& StringPool::operator=( StringPool&& rhs )
{
  m_pool = std::move( rhs.m_pool );
  return *this;
}

int StringPool::read( PHYSFS_Io& io, unsigned int pos )
{
  try
  {
    m_pool.clear();

    // Read header
    BFSStringPoolHeader header = readHeader( io, pos );

    unsigned int huffmanTreeSize;
    unsigned int offsetsSize;
    unsigned int compressedStringsSize;
    unsigned int uncompressedSizesSize;
    {
      std::set< unsigned int > offsets{
      header.huffmannTreeOffset,
      header.offsetsOffset,
      header.compressedStringsOffset,
      header.end,
      header.uncompressedSizesOffset
    };
      // Total size ought to be the largest value
      if( *offsets.rbegin() != header.end ) return -1;
      // Calculate sizes
      auto getSize = [ &offsets ]( unsigned int offset )
      {
        auto nextOffset = ++offsets.find( offset );
        assert( nextOffset != offsets.end() );
        return *nextOffset - offset;
      };
      huffmanTreeSize = getSize( header.huffmannTreeOffset );
      offsetsSize = getSize( header.offsetsOffset );
      compressedStringsSize = getSize( header.compressedStringsOffset );
      uncompressedSizesSize = getSize( header.uncompressedSizesOffset );
    }

    // Read Huffmann Tree
    std::vector< char > huffmanTreeData( huffmanTreeSize );
    if( !io.seek( &io, header.huffmannTreeOffset ) ) return -1;
    if( io.read( &io, huffmanTreeData.data(), huffmanTreeSize ) != huffmanTreeSize ) return -1;
    Huffmann huffmannTree( huffmanTreeData.data(), huffmanTreeData.data() + huffmanTreeData.size() );
    huffmanTreeData.clear();

    // Read unpacked sizes
    std::vector< std::uint16_t > uncompressedSizes( uncompressedSizesSize / 2 );
    if( !io.seek( &io, header.uncompressedSizesOffset ) ) return -1;
    if( io.read( &io, uncompressedSizes.data(), uncompressedSizesSize ) != uncompressedSizesSize ) return -1;

    // Read offsets
    std::vector< std::uint32_t > offsets( offsetsSize / 4 );
    if( !io.seek( &io, header.offsetsOffset ) ) return -1;
    if( io.read( &io, offsets.data(), offsetsSize ) != offsetsSize ) return -1;

    // Read packed strings
    std::vector< char > compressedStrings( compressedStringsSize );
    if( !io.seek( &io, header.compressedStringsOffset ) ) return -1;
    if( io.read( &io, compressedStrings.data(), compressedStringsSize ) != compressedStringsSize ) return -1;

    const unsigned int stringCount = std::min( uncompressedSizes.size(), offsets.size() );

    m_pool.reserve( stringCount );
    for( unsigned int strIndex = 0; strIndex < stringCount; ++strIndex )
    {
      const char * const begin = compressedStrings.data() + PHYSFS_swapULE32( offsets[ strIndex ] );
      const char * const end = compressedStrings.data() + compressedStrings.size();
      BitStream stream( begin, end );

      std::string str;
      std::uint16_t uncompressedSize = PHYSFS_swapULE16( uncompressedSizes[ strIndex ] );
      str.reserve( uncompressedSize );

      for( unsigned int charIndex = 0; charIndex < uncompressedSize; ++charIndex )
      {
        char curChar;
        if( !huffmannTree.decode( stream, curChar ) )
        {
          m_pool.clear();
          return -1;
        }
        str.push_back( curChar );
      }
      m_pool.emplace_back( std::move( str ) );
    }
    return header.end;
  }
  catch( UnexpectedEnfOfInput )
  {
    return -1;
  }
}
