#include "bfsfile.hpp"
#include "bfsarchive.hpp"

#include <utility>
#include <cassert>

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
    PHYSFS_setErrorCode( code );
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
    PHYSFS_setErrorCode( code );
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
    PHYSFS_setErrorCode( code );
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
    PHYSFS_setErrorCode( code );
    return -1;
  }
}

extern "C" static PHYSFS_Io* duplicate( PHYSFS_Io* io )
{
  try
  {
    BFSFile& file = *static_cast< BFSFile* >( io->opaque );
    return ( new BFSFile( file ) )->getPhysFSInterface();
  }
  catch( PHYSFS_ErrorCode code )
  {
    PHYSFS_setErrorCode( code );
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
    PHYSFS_setErrorCode( code );
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

BFSFile::BFSFile( BFSArchive& archive )
: m_ioInterface( initFileIO( this ) )
, m_archive( archive.getIO().duplicate( &archive.getIO() ) )
{
  // duplicate returned nullptr?
  if( !m_archive )
  {
    // As presumably set by duplicate()
    throw( PHYSFS_getLastErrorCode() );
  }
}

BFSFile::~BFSFile()
{
  if( m_archive ) m_archive->destroy( m_archive );
}

BFSFile::BFSFile( const BFSFile& rhs )
: m_ioInterface( initFileIO( this ) )
, m_archive( rhs.m_archive ? rhs.m_archive->duplicate( rhs.m_archive ) : nullptr )
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

  return *this;
}

BFSFile& BFSFile::operator=( BFSFile&& rhs )
{
  if( this == &rhs ) return *this;

  // Move Archive IO
  if( m_archive ) m_archive->destroy( m_archive );
  m_archive = rhs.m_archive;
  rhs.m_archive = nullptr;

  return *this;
}

PHYSFS_sint64 BFSFile::read( char buf[], const PHYSFS_uint64 len )
{
  if( !m_archive ) return -1;

  // TODO
  ( void )buf;
  ( void )len;
  return -1;
}

int BFSFile::seek( PHYSFS_uint64 position )
{
  if( !m_archive ) return false;

  // TODO
  ( void )position;
  return false;
}

PHYSFS_sint64 BFSFile::tell()
{
  if( !m_archive ) return -1;

  // TODO
  return -1;
}

PHYSFS_sint64 BFSFile::size()
{
  if( !m_archive ) return -1;

  // TODO
  return -1;
}