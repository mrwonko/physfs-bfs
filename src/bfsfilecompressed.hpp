#pragma once
#include "bfsfile.hpp"

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

private:
  virtual PHYSFS_sint64 readImpl( char buf[], const PHYSFS_uint64 len ) override;
  virtual int seekImpl( PHYSFS_uint64 position ) override;
};
