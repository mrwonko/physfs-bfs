#pragma once

#include <physfs.h>

#include <string>

class BFSArchive;

/**
@brief Access to an uncompressed file in a BFS Archive
**/
class BFSFile
{
public:
  struct Info
  {
    PHYSFS_uint32 offset;
    PHYSFS_uint32 compressedSize;
    PHYSFS_uint32 uncompressedSize;
    bool compressed;
  };

public:
  BFSFile( BFSArchive& archive, Info* info );
  virtual ~BFSFile();
  BFSFile( const BFSFile& rhs );
  BFSFile( BFSFile&& rhs );
  BFSFile& operator=( const BFSFile& rhs );
  BFSFile& operator=( BFSFile&& rhs );

  PHYSFS_Io* getPhysFSInterface() { return &m_ioInterface; }

  PHYSFS_sint64 read( char buf[], const PHYSFS_uint64 len );
  virtual int seek( PHYSFS_uint64 position );
  virtual PHYSFS_sint64 tell() const { return m_phyiscalPos; }
  PHYSFS_sint64 size() const { return m_info->uncompressedSize; };

  virtual BFSFile* clone() const { return new BFSFile( *this ); }

protected:
  virtual PHYSFS_sint64 readImpl( char buf[], const PHYSFS_uint64 len );

private:
  /// PhysFS Interface to this File
  PHYSFS_Io m_ioInterface;
protected:
  /// IO of the Archive this file is part of (duplicate owned by us)
  PHYSFS_Io* m_archive;
  /// I/o position in archive where this file starts
  Info* m_info;
  /// Physical position in file
  PHYSFS_sint64 m_phyiscalPos;
};
