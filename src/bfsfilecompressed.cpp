#include "bfsfilecompressed.hpp"

#include <cassert>
#include <algorithm>

BFSFileCompressed::BFSFileCompressed( BFSArchive& archive, Info* info )
: BFSFile( archive, info )
, m_logicalPos( 0 )
{
}


BFSFileCompressed::~BFSFileCompressed()
{
}

BFSFileCompressed::BFSFileCompressed( BFSFileCompressed&& rhs )
: BFSFile( std::move( rhs ) )
, m_logicalPos( rhs.m_logicalPos )
, m_stream( std::move( rhs.m_stream ) )
{
}

BFSFileCompressed& BFSFileCompressed::operator=( BFSFileCompressed&& rhs )
{
  BFSFile::operator=( std::move( rhs ) );
  m_logicalPos = rhs.m_logicalPos;
  m_stream = std::move( rhs.m_stream );
  return *this;
}

PHYSFS_sint64 BFSFileCompressed::readImpl( char buf[], const PHYSFS_uint64 len )
{
  auto read = m_stream.read( buf, len,
    [ this ]( char buf[], const PHYSFS_uint64 len )
  {
    return BFSFile::readImpl( buf, len );
  } );
  m_logicalPos += read;
  return read;
}

int BFSFileCompressed::seek( PHYSFS_uint64 position )
{
  if( position >= m_info->uncompressedSize ) throw PHYSFS_ERR_PAST_EOF;

  // need to go back? then start over.
  if( position < m_logicalPos )
  {
    m_stream = ZipStream();
    if( !BFSFile::seek( 0 ) )
    {
      // FIXME (how?): we are now probably in a broken state
      return false;
    }
    m_logicalPos = 0;
  }
  while( m_logicalPos < position )
  {
    char buffer[ 512 ];
    PHYSFS_uint64 toRead{ std::min< PHYSFS_uint64 >( sizeof( buffer ), position - m_logicalPos ) };
    auto read = BFSFileCompressed::readImpl( buffer, toRead );
    if( read <= 0 ) return false;
    m_logicalPos += read;
  }
  return true;
}