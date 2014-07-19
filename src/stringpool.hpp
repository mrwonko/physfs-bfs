#pragma once

#include <string>
#include <vector>

struct PHYSFS_Io;

class StringPool
{
public:
  StringPool() = default;
  ~StringPool() = default;
  StringPool( const StringPool& rhs ) = default;
  StringPool& operator=( const StringPool& rhs ) = default;
  StringPool( StringPool&& rhs );
  StringPool& operator=( StringPool&& rhs );

  /**
  @param io File to read from; i/o position will be moved past end of pool.
  @param pos i/o position of string pool start
  @return i/o position past end of pool.
  **/
  int read( PHYSFS_Io& io, unsigned int pos );

  const std::string& at( unsigned int index ) const { return m_pool.at( index ); }
  unsigned int size() const { return m_pool.size(); }

private:
  std::vector< std::string > m_pool;

};

