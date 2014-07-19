#pragma once

#include <physfs.h>

#include <string>

class BFSArchive;

class BFSFile
{
public:
  BFSFile( BFSArchive& archive );
  ~BFSFile();
  BFSFile( const BFSFile& rhs );
  BFSFile( BFSFile&& rhs );
  BFSFile& operator=( const BFSFile& rhs );
  BFSFile& operator=( BFSFile&& rhs );

  PHYSFS_Io* getPhysFSInterface() { return &m_ioInterface; }

  PHYSFS_sint64 read( char buf[], const PHYSFS_uint64 len );
  int seek( PHYSFS_uint64 position );
  PHYSFS_sint64 tell();
  PHYSFS_sint64 size();

private:
  /// PhysFS Interface to this File
  PHYSFS_Io m_ioInterface;
  /// IO of the Archive this file is part of (duplicate owned by us)
  PHYSFS_Io* m_archive;
};
