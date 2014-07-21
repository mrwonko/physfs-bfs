#pragma once

#include <functional>
#include <vector>
#include <cstdint>

typedef struct mz_stream_s mz_stream;

class ZipStream
{
  enum {
    BUFFERSIZE = 16 * 1024,
  };
public:
  /**
  @throw PHYSFS_ErrorCode on error
  **/
  ZipStream();
  ~ZipStream();
  /**
  @throw PHYSFS_ErrorCode on error
  **/
  ZipStream( const ZipStream& rhs );
  /**
  @throw PHYSFS_ErrorCode on error
  **/
  ZipStream& operator=( const ZipStream& rhs );
  ZipStream( ZipStream&& rhs );
  ZipStream& operator=( ZipStream&& rhs );

  /**
  Try uncompressing len bytes into buf.
  Retrieve new compressed data using readInput.
  @param readInput function for retrieving compressed data, should return -1 on error and set PhysFS Error Code, otherwise returns number of bytes read.
  @return Number of bytes read, or -1 on error. Use PHYSFS_getError() to get error details.
  **/
  std::int64_t read( char buf[], const std::uint64_t len, std::function< std::int64_t( char buf[], const std::uint64_t len ) > readInput );

private:
  void copyStream( const ZipStream& rhs );

private:
  mz_stream* m_stream;
  std::vector< unsigned char > m_buffer;
};

