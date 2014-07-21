#include "bfsarchiver.h"

#include <utility>
#include <cassert>

#include <physfs.h>

#include <iostream>
#include <string>
#include <fstream>

void recursiveDir( const std::string& dir, unsigned indent )
{
  PHYSFS_enumerateFilesCallback( dir.c_str(),
    []( void* data, const char *origdir, const char *filename )
  {
    unsigned indent = *reinterpret_cast< unsigned int* >( data );
    std::cout << std::string( indent, ' ' ) << filename;

    PHYSFS_Stat stat;
    std::string fullname = origdir;
    fullname += '/';
    fullname += filename;
    if( PHYSFS_stat( fullname.c_str(), &stat ) )
    {
      switch( stat.filetype )
      {
      case PHYSFS_FILETYPE_REGULAR:
        std::cout << " <file, " << stat.filesize << " bytes>" << std::endl;
        break;
      case PHYSFS_FILETYPE_DIRECTORY:
        std::cout << " <directory>" << std::endl;
        recursiveDir( fullname, indent + 1 );
        break;
      case PHYSFS_FILETYPE_SYMLINK:
        std::cout << " <symlink>" << std::endl;
        break;
      case PHYSFS_FILETYPE_OTHER:
        std::cout << " <other>" << std::endl;
        break;
      default:
        std::cout << " <invalid>" << std::endl;
        break;
      }
    }
    else
    {
      std::cout << " <unknown>" << std::endl;
    }
  }, &indent );
}

int main( int argc, char** argv )
{

  if( !PHYSFS_init( argv[ 0 ] ) )
  {
    std::cerr << "Could not init PhysFS: " << PHYSFS_getLastError() << std::endl;
    return 1;
  }
  struct PhysFSCloser
  {
    ~PhysFSCloser() { PHYSFS_deinit(); }
  } physFSCloser;

  if( !registerBfsArchiver() )
  {
    std::cerr << "Could not init BFS Archiver! " << PHYSFS_getLastError() << std::endl;
    return 1;
  }
  std::string mountFile = "patch1.bfs";
  if( argc > 1 ) mountFile = argv[ 1 ];

  if( !PHYSFS_mount( mountFile.c_str(), "/", true ) )
  {
    std::cerr << "Could not mount " << mountFile << "! " << PHYSFS_getLastError() << std::endl;
    return 1;
  }

  if( argc > 2 )
  {
    auto file = PHYSFS_openRead( argv[ 2 ] );
    if( !file )
    {
      std::cerr << "Error opening " << argv[ 2 ] << ": " << PHYSFS_getLastError() << std::endl;
    }
    else
    {
      std::string outFilename( argv[ 2 ] );
      for( char& c : outFilename )
      {
        if( c == '/' || c == '\\' ) c = ',';
      }
      std::ofstream outFile( outFilename.c_str(), std::ios::binary );
      if( !outFile.fail() )
      {
        PHYSFS_sint64 bytesRead = 0;
        char buffer[ 1024 ];
        bool error = false;
        while( ( bytesRead = PHYSFS_readBytes( file, buffer, sizeof( buffer ) ) ) > 0 )
        {
          if( bytesRead < sizeof( buffer ) && !PHYSFS_eof( file ) )
          {
            std::cerr << "Error reading from " << argv[ 2 ] << ": " << PHYSFS_getLastError() << std::endl;
            error = true;
            break;
          }
          outFile.write( buffer, bytesRead );
          if( outFile.fail() )
          {
            std::cerr << "Error writing to " << outFilename << "!" << std::endl;
            error = true;
            break;
          }
        }
        if( !error )
        {
          std::cout << "Unpacked " << argv[ 2 ] << " to " << outFilename << std::endl;
        }
      }
      PHYSFS_close( file );
    }
  }
  else
  {
    std::cout << "Files in " << mountFile << ":" << std::endl;
    recursiveDir( "", 1 );
  }

  return 0;
}
