#pragma once
#include "bfsfile.hpp"
#include "zipstream.hpp"

class BFSFileCompressed : public BFSFile
{
public:
  BFSFileCompressed( BFSArchive& archive, Info* info );
  virtual ~BFSFileCompressed();
  BFSFileCompressed( const BFSFileCompressed& rhs ) = default;
  BFSFileCompressed& operator=( const BFSFileCompressed& rhs ) = default;
  BFSFileCompressed( BFSFileCompressed&& rhs );
  BFSFileCompressed& operator=( BFSFileCompressed&& rhs );

  virtual BFSFileCompressed* clone() const override { return new BFSFileCompressed( *this ); }

  virtual PHYSFS_sint64 tell() const override { return m_logicalPos; }
  virtual int seek( PHYSFS_uint64 position ) override;

protected:
  virtual PHYSFS_sint64 readImpl( char buf[], const PHYSFS_uint64 len ) override;

private:
  PHYSFS_uint64 m_logicalPos;
  ZipStream m_stream;
};
