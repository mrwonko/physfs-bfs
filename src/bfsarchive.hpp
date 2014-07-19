#pragma once

#include <physfs.h>

#include <string>
#include <map>
#include <cstdint>
#include <memory>
#include <utility>

class BFSFile;

class BFSArchive
{
public:
  BFSArchive( PHYSFS_Io& io );
  ~BFSArchive();
  BFSArchive( const BFSArchive& ) = delete;
  BFSArchive( BFSArchive&& ) = delete;
  BFSArchive& operator=( const BFSArchive& ) = delete;
  BFSArchive& operator=( BFSArchive&& ) = delete;

  void enumerateFiles( std::string dirname, PHYSFS_EnumFilesCallback cb, const char* origdir, void* callbackdata );
  BFSFile* openRead( const std::string& filename );
  bool stat( const std::string& filename, PHYSFS_Stat& stat );

  PHYSFS_Io& getIO() { return m_io; }

private:
  struct FileInfo
  {
    std::uint32_t offset;
    std::uint32_t compressedSize;
    std::uint32_t uncompressedSize;
    bool compressed;
  };

  struct Directory
  {
    std::map< std::string, std::unique_ptr< Directory > > dirs;
    std::map< std::string, FileInfo > files;

    void insert( std::string&& filename, FileInfo&& file );
  };

private:
  std::pair< Directory*, FileInfo* > lookup( std::string filename );

private:
  PHYSFS_Io& m_io;
  Directory m_root;
};
