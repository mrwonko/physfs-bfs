#include "bfsfilecompressed.hpp"

#include <cassert>

BFSFileCompressed::BFSFileCompressed( BFSArchive& archive, Info* info )
: BFSFile( archive, info )
{
}


BFSFileCompressed::~BFSFileCompressed()
{
}

BFSFileCompressed::BFSFileCompressed( BFSFileCompressed&& rhs )
: BFSFile( std::move( rhs ) )
{
}

BFSFileCompressed& BFSFileCompressed::operator=( BFSFileCompressed&& rhs )
{
  BFSFile::operator=( std::move( rhs ) );
  return *this;
}

PHYSFS_sint64 BFSFileCompressed::readImpl( char buf[], const PHYSFS_uint64 len )
{
  assert( m_info->compressed );

  // TODO
  ( void )buf;
  ( void )len;
  return -1;
}

int BFSFileCompressed::seekImpl( PHYSFS_uint64 position )
{
  assert( m_info->compressed );

  // TODO
  ( void )position;
  return false;
}