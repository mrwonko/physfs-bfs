#include "bfsarchive.hpp"
#include "bfsformat.hpp"
#include "stringpool.hpp"
#include "bfsfilecompressed.hpp"

#include <vector>
#include <cassert>
#include <memory>
#include <algorithm>
#include <iostream>

static BFSHeader readHeader( PHYSFS_Io& io )
{
  io.seek( &io, 0 );
  BFSHeader header;
  if( io.read( &io, &header, sizeof( header ) ) != sizeof( header )
    || std::string( header.fileId, 4 ) != std::string( "bfs1", 4 ) )
  {
    throw PHYSFS_ERR_CORRUPT;
  }
  // Byte swap
  header.fileCount = PHYSFS_swapSLE32( header.fileCount );
  header.flagAndHeaderSize = PHYSFS_swapSLE32( header.flagAndHeaderSize );
  header.hashSize = PHYSFS_swapSLE32( header.hashSize );

  return header;
}

BFSArchive::BFSArchive( PHYSFS_Io& io )
: m_io( io )
{
  // Read Header
  BFSHeader header = readHeader( io );
  if( header.hashSize != BFSHeader::HASH_SIZE )
  {
    std::cerr << "Invalid Hash Size" << std::endl;
    throw PHYSFS_ERR_CORRUPT;
  }

  StringPool stringPool;
  unsigned int stringPoolEnd = stringPool.read( io, BFSHeader::HEADER_SIZE );
  if( stringPoolEnd == -1 )
  {
    std::cerr << "Error: Failed to read string pool!" << std::endl;
    throw PHYSFS_ERR_CORRUPT;
  }

  unsigned int fileCount = std::min( header.fileCount, stringPool.size() );
  if( !io.seek( &io, stringPoolEnd ) )
  {
    std::cerr << "Error: Failed to seek File Info" << std::endl;
    throw PHYSFS_ERR_CORRUPT;
  }

  std::vector< BFSFileInfo > fileInfos( fileCount );
  if( io.read( &io, fileInfos.data(), fileCount * sizeof( BFSFileInfo ) ) != fileCount * sizeof( BFSFileInfo ) )
  {
    std::cerr << "Error: Failed to read File Info" << std::endl;
    throw PHYSFS_ERR_CORRUPT;
  }

  for( const auto& fileInfo : fileInfos )
  {
    const auto compressedSize = PHYSFS_swapULE32( fileInfo.compressedSize );
    const auto uncompressedSize = PHYSFS_swapULE32( fileInfo.uncompressedSize );
    const auto offset = PHYSFS_swapULE32( fileInfo.offset );
    const auto compressionType = PHYSFS_swapULE32( fileInfo.compressionType );
    const bool compressed = compressionType == 5;
    const auto dir = PHYSFS_swapULE16( fileInfo.dirStringIndex );
    const auto file = PHYSFS_swapULE16( fileInfo.fileStringIndex );
    std::string filename = stringPool.at( dir ) + '/' + stringPool.at( file );

    if( !compressed && compressionType != 4 )
    {
      std::cerr << "Warning: Ignoring file '" << filename << "' with unsupported compression type " << fileInfo.compressionType << "!" << std::endl;
      continue;
    }

    if( !compressed ) std::cout << filename << " is uncompressed." << std::endl;

    m_root.insert( std::move( filename ), {
      offset,
      compressedSize,
      uncompressedSize,
      compressed
    } );
  }
}

void BFSArchive::Directory::insert( std::string&& filename, BFSFile::Info&& file )
{
  auto slashPos = filename.find( '/' );
  if( slashPos == std::string::npos )
  {
    files.emplace( std::make_pair( filename, file ) );
  }
  else
  {
    std::string dirname = filename.substr( 0, slashPos );
    std::string tail = filename.substr( slashPos + 1 );
    auto entry = dirs.find( dirname );
    if( entry == dirs.end() )
    {
      entry = dirs.emplace( std::make_pair( std::move( dirname ), std::unique_ptr< Directory >( new Directory() ) ) ).first;
    }
    entry->second->insert( std::move( tail ), std::move( file ) );
  }
}

BFSArchive::~BFSArchive()
{
  m_io.destroy( &m_io );
}

void BFSArchive::enumerateFiles( std::string dirname, PHYSFS_EnumFilesCallback cb, const char* origdir, void* callbackdata )
{
  Directory* dir = lookup( dirname ).first;
  if( !dir ) return;
  for( auto& entry : dir->dirs )
  {
    cb( callbackdata, origdir, entry.first.c_str() );
  }
  for( auto& entry : dir->files )
  {
    cb( callbackdata, origdir, entry.first.c_str() );
  }
}

BFSFile* BFSArchive::openRead( const std::string& filename )
{
  BFSFile::Info* info = lookup( filename ).second;
  if( !info )
  {
    PHYSFS_setErrorCode( PHYSFS_ERR_NOT_FOUND );
    return nullptr;
  }
  return info->compressed ? new BFSFileCompressed( *this, info ) : new BFSFile( *this, info );
}

bool BFSArchive::stat( const std::string& filename, PHYSFS_Stat& stat )
{
  stat.modtime = -1;
  stat.createtime = -1;
  stat.accesstime = -1;
  stat.readonly = 1;
  auto entry = lookup( filename );
  if( entry.first )
  {
    stat.filesize = -1;
    stat.filetype = PHYSFS_FILETYPE_DIRECTORY;
    return true;
  }
  if( entry.second )
  {
    stat.filesize = entry.second->uncompressedSize;
    stat.filetype = PHYSFS_FILETYPE_REGULAR;
    return true;
  }
  return false;
}

std::pair< BFSArchive::Directory*, BFSFile::Info* > BFSArchive::lookup( std::string filename )
{
  Directory* targetDir = &m_root;
  if( !filename.empty() && filename != "/" ) // root dir
  {
    std::string::size_type slashPos;
    while( ( slashPos = filename.find( '/' ) ) != std::string::npos )
    {
      const std::string dir = filename.substr( 0, slashPos );
      filename.erase( 0, slashPos + 1 );
      auto entry = targetDir->dirs.find( dir );
      // missing directory
      if( entry == targetDir->dirs.end() ) return std::make_pair( nullptr, nullptr );
      targetDir = entry->second.get();
    }
  }
  if( filename.empty() ) // trailing slash
  {
    return std::make_pair( targetDir, nullptr );
  }
  std::pair< Directory*, BFSFile::Info* > result{ nullptr, nullptr };
  // Look for Directory
  {
    auto entry = targetDir->dirs.find( filename );
    if( entry != targetDir->dirs.end( ) )
    {
      result.first = entry->second.get( );
    }
  }
  // Look for File
  {
    auto entry = targetDir->files.find( filename );
    if( entry != targetDir->files.end( ) )
    {
      result.second = &entry->second;
    }
  }
  return result;
}
