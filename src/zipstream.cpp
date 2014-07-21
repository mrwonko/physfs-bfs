#include "zipstream.hpp"

#include <physfs.h>
extern "C"
{
#include "physfs_miniz.hpp"
}

#include <utility>
#include <cstdlib>
#include <cstring>

// zlib's inflateCopy not supplied by physfs_miniz.hpp
static int inflateCopy( z_stream* dest, const z_stream* src )
{
  if( !dest || !src ) return MZ_PARAM_ERROR;
  auto prevMsg = dest->msg;
  std::memcpy( dest, src, sizeof( z_stream ) );
  dest->msg = prevMsg;

  // copy stream state, if any
  if( dest->state )
  {
    if( !dest->zalloc ) return Z_STREAM_ERROR;
    inflate_state* state = static_cast< inflate_state* >( dest->zalloc( dest->opaque, 1, sizeof( inflate_state ) ) );
    if( !state )
    {
      dest->state = nullptr;
      return Z_MEM_ERROR;
    }
    std::memcpy( state, src->state, sizeof( inflate_state ) );
    dest->state = reinterpret_cast< mz_internal_state* >( state );
  }
  return Z_OK;
}

static void* alloc_func( void *opaque, unsigned int items, unsigned int size )
{
  ( void )opaque;
  return std::malloc( items * size );
}

static void free_func( void *opaque, void *address )
{
  ( void )opaque;
  std::free( address );
}

ZipStream::ZipStream()
: m_stream( new z_stream{} ) // zero-initialize
, m_buffer( BUFFERSIZE )
{
  m_stream->zalloc = alloc_func;
  m_stream->zfree = free_func;
  switch( inflateInit2( m_stream, MAX_WBITS ) )
  {
  case Z_OK:
    break;
  case Z_MEM_ERROR:
    delete m_stream;
    m_stream = nullptr;
    throw PHYSFS_ERR_OUT_OF_MEMORY;
  default:
    delete m_stream;
    m_stream = nullptr;
    throw PHYSFS_ERR_OTHER_ERROR;
  }
}

ZipStream::~ZipStream()
{
  if( m_stream )
  {
    inflateEnd( m_stream );
    delete m_stream;
  }
}

ZipStream::ZipStream( const ZipStream& rhs )
: m_stream( new z_stream{} ) // zero-initialize
, m_buffer( BUFFERSIZE )
{
  copyStream( rhs );
}

ZipStream& ZipStream::operator=( const ZipStream& rhs )
{
  if( m_stream ) inflateEnd( m_stream );
  copyStream( rhs );
  return *this;
}

void ZipStream::copyStream( const ZipStream& rhs )
{
  if( !rhs.m_stream )
  {
    delete m_stream;
    m_stream = nullptr;
    return;
  }
  if( !m_stream ) m_stream = new z_stream{};
  switch( inflateCopy( m_stream, rhs.m_stream ) )
  {
  case Z_OK:
    break;
  case Z_MEM_ERROR:
    delete m_stream;
    m_stream = nullptr;
    throw PHYSFS_ERR_OUT_OF_MEMORY;
  default:
    delete m_stream;
    m_stream = nullptr;
    throw PHYSFS_ERR_OTHER_ERROR;
  }
  // copy input buffer if necessary
  if( rhs.m_stream->avail_in > 0 )
  {
    m_buffer = rhs.m_buffer;
    m_stream->next_in = m_buffer.data() + ( rhs.m_stream->next_in - rhs.m_buffer.data() );
  }
}

ZipStream::ZipStream( ZipStream&& rhs )
: m_stream( rhs.m_stream )
, m_buffer( std::move( rhs.m_buffer ) )
{
  rhs.m_stream = nullptr;
}

ZipStream& ZipStream::operator=( ZipStream&& rhs )
{
  if( m_stream )
  {
    inflateEnd( m_stream );
    delete m_stream;
  }
  m_stream = rhs.m_stream;
  rhs.m_stream = nullptr;
  m_buffer = std::move( rhs.m_buffer );
  return *this;
}


std::int64_t ZipStream::read( char buf[], const std::uint64_t len, std::function< std::int64_t( char buf[], const std::uint64_t len ) > readInput )
{
  if( !m_stream )
  {
    PHYSFS_setErrorCode( PHYSFS_ERR_OTHER_ERROR );
    return -1;
  }
  std::int64_t read = 0;
  m_stream->avail_out = len;
  m_stream->next_out = reinterpret_cast< unsigned char* >( buf );
  while( read < len )
  {
    // Fill input buffer if necessary
    if( m_stream->avail_in == 0 )
    {
      m_stream->avail_in = readInput( reinterpret_cast< char* >( m_buffer.data() ), m_buffer.size() );
      if( m_stream->avail_in < 0 )
      {
        // readInput() should've set an error code.
        return -1;
      }
      if( m_stream->avail_in == 0 )
      {
        PHYSFS_setErrorCode( PHYSFS_ERR_PAST_EOF );
        break;
      }
      m_stream->next_in = m_buffer.data();
    }
    auto previouslyRead = m_stream->total_out;
    auto retVal = inflate( m_stream, Z_SYNC_FLUSH );
    read += m_stream->total_out - previouslyRead;
    switch( retVal )
    {
    case Z_OK:
      // Read something, not done yet
      break;
    case Z_STREAM_END:
      // Read all there is
      return read;
      break;

      // Error cases
    case Z_ERRNO:
      PHYSFS_setErrorCode( PHYSFS_ERR_IO );
      return -1;
    case Z_MEM_ERROR:
      PHYSFS_setErrorCode( PHYSFS_ERR_OUT_OF_MEMORY );
      return -1;
    default:
      PHYSFS_setErrorCode( PHYSFS_ERR_CORRUPT );
      return -1;
    }
  }
  return read;
}
