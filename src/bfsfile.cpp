#include "bfsfile.hpp"
#include "bfsarchive.hpp"

#include <utility>
#include <cassert>
#include <algorithm>

//    PhysFS Callbacks

extern "C" static PHYSFS_sint64 read( PHYSFS_Io* io, void *buf, PHYSFS_uint64 len )
{
  try
  {
    BFSFile& file = *static_cast< BFSFile* >( io->opaque );
    return file.read( static_cast< char* >( buf ), len );
  }
  catch( PHYSFS_ErrorCode code )
  {
    if( code ) PHYSFS_setErrorCode( code );
    return -1;
  }
}

extern "C" static int seek( PHYSFS_Io* io, PHYSFS_uint64 position )
{
  try
  {
    BFSFile& file = *static_cast< BFSFile* >( io->opaque );
    return file.seek( position );

  }
  catch( PHYSFS_ErrorCode code )
  {
    if( code ) PHYSFS_setErrorCode( code );
    return false;
  }
}

extern "C" static PHYSFS_sint64 tell( PHYSFS_Io* io )
{
  try
  {
    BFSFile& file = *static_cast< BFSFile* >( io->opaque );
    return file.tell();
  }
  catch( PHYSFS_ErrorCode code )
  {
    if( code ) PHYSFS_setErrorCode( code );
    return -1;
  }
}

extern "C" static PHYSFS_sint64 length( PHYSFS_Io* io )
{
  try
  {
    BFSFile& file = *static_cast< BFSFile* >( io->opaque );
    return file.size();

  }
  catch( PHYSFS_ErrorCode code )
  {
    if( code ) PHYSFS_setErrorCode( code );
    return -1;
  }
}

extern "C" static PHYSFS_Io* duplicate( PHYSFS_Io* io )
{
  try
  {
    BFSFile& file = *static_cast< BFSFile* >( io->opaque );
    return file.clone()->getPhysFSInterface();
  }
  catch( PHYSFS_ErrorCode code )
  {
    if( code ) PHYSFS_setErrorCode( code );
    return nullptr;
  }
}

extern "C" static void destroy( PHYSFS_Io* io )
{
  try
  {
    BFSFile* file = static_cast< BFSFile* >( io->opaque );
    delete file;
  }
  catch( PHYSFS_ErrorCode code )
  {
    if( code ) PHYSFS_setErrorCode( code );
  }
}

static inline PHYSFS_Io initFileIO( BFSFile* self )
{
  PHYSFS_Io io = {
    0,
    self,
    read,
    nullptr, // no write()
    seek,
    tell,
    length,
    duplicate,
    nullptr, // no flush()
    destroy
  };
  return io;
}

//    BFSFile Class Implementation

BFSFile::BFSFile( BFSArchive& archive, Info* info )
: m_ioInterface( initFileIO( this ) )
, m_archive( archive.getIO().duplicate( &archive.getIO() ) )
, m_info( info )
{
  // duplicate returned nullptr?
  if( !m_archive )
  {
    // As presumably set by duplicate()
    throw( PHYSFS_getLastErrorCode() );
  }
  // Bail without changing error code on failure
  if( !seek( 0 ) ) throw PHYSFS_ERR_OK;
}

BFSFile::~BFSFile()
{
  if( m_archive ) m_archive->destroy( m_archive );
}

BFSFile::BFSFile( const BFSFile& rhs )
: m_ioInterface( initFileIO( this ) )
, m_archive( rhs.m_archive ? rhs.m_archive->duplicate( rhs.m_archive ) : nullptr )
, m_info( rhs.m_info )
, m_phyiscalPos( rhs.m_phyiscalPos )
{
  // duplicate returned nullptr?
  if( !m_archive && !rhs.m_archive )
  {
    // As presumably set by duplicate()
    throw( PHYSFS_getLastErrorCode() );
  }
}

BFSFile::BFSFile( BFSFile&& rhs )
: m_ioInterface( initFileIO( this ) )
, m_archive( rhs.m_archive )
, m_info( rhs.m_info )
, m_phyiscalPos( rhs.m_phyiscalPos )
{
  rhs.m_archive = nullptr;
}

BFSFile& BFSFile::operator=( const BFSFile& rhs )
{
  if( this == &rhs ) return *this;

  // Copy Archive IO
  if( m_archive ) m_archive->destroy( m_archive );
  if( rhs.m_archive )
  {
    m_archive = rhs.m_archive->duplicate( rhs.m_archive );
    if( !m_archive )
    {
      // As presumably set by duplicate()
      throw( PHYSFS_getLastErrorCode() );
    }
  }
  else
  {
    m_archive = nullptr;
  }

  m_info = rhs.m_info;
  m_phyiscalPos = rhs.m_phyiscalPos;

  return *this;
}

BFSFile& BFSFile::operator=( BFSFile&& rhs )
{
  if( this == &rhs ) return *this;

  // Move Archive IO
  if( m_archive ) m_archive->destroy( m_archive );
  m_archive = rhs.m_archive;
  rhs.m_archive = nullptr;

  m_info = rhs.m_info;
  m_phyiscalPos = rhs.m_phyiscalPos;

  return *this;
}

PHYSFS_sint64 BFSFile::read( char buf[], const PHYSFS_uint64 len )
{
  if( !m_archive ) return -1;

  auto bytesRead = readImpl( buf, std::min< PHYSFS_uint64 >( m_info->uncompressedSize - m_phyiscalPos, len ) );
  if( bytesRead > 0 ) m_phyiscalPos += bytesRead;
  return bytesRead;
}

PHYSFS_sint64 BFSFile::readImpl( char buf[], const PHYSFS_uint64 len )
{
  return m_archive->read( m_archive, buf, len );
}

int BFSFile::seek( PHYSFS_uint64 position )
{
  if( position >= m_info->compressedSize ) throw PHYSFS_ERR_PAST_EOF;
  if( m_archive->seek( m_archive, m_info->offset + position ) )
  {
    m_phyiscalPos = position;
    return true;
  }
  else
  {
    m_phyiscalPos = m_archive->tell( m_archive );
    return false;
  }
}
