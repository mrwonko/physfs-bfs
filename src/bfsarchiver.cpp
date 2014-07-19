#include "bfsarchiver.h"
#include "bfsarchive.hpp"
#include "bfsfile.hpp"

#include <physfs.h>

extern "C" static void* openArchive( PHYSFS_Io* io, const char* name, int forWrite )
{
  ( void )name;
  if( forWrite ) return nullptr;
  try
  {
    BFSArchive* archive = new BFSArchive( *io );
    return archive;
  }
  catch( PHYSFS_ErrorCode code )
  {
    PHYSFS_setErrorCode( code );
    return nullptr;
  }
}

extern "C" static void closeArchive( void* opaque )
{
  try
  {
    BFSArchive* archive = reinterpret_cast< BFSArchive* >( opaque );
    delete archive;
  }
  catch( PHYSFS_ErrorCode code )
  {
    PHYSFS_setErrorCode( code );
  }
}

extern "C" static void enumerateFiles( void* opaque, const char* dirname, PHYSFS_EnumFilesCallback cb, const char* origdir, void* callbackdata )
{
  try
  {
    BFSArchive* archive = reinterpret_cast< BFSArchive* >( opaque );
    archive->enumerateFiles( dirname, cb, origdir, callbackdata );
  }
  catch( PHYSFS_ErrorCode code )
  {
    PHYSFS_setErrorCode( code );
  }
}

extern "C" static PHYSFS_Io* openRead( void* opaque, const char* filename )
{
  try
  {
    BFSArchive* archive = reinterpret_cast< BFSArchive* >( opaque );
    BFSFile* file = archive->openRead( filename );
    return file ? file->getPhysFSInterface() : nullptr;
  }
  catch( PHYSFS_ErrorCode code )
  {
    PHYSFS_setErrorCode( code );
    return nullptr;
  }
}

// openWrite/openAppend are unsupported
extern "C" static PHYSFS_Io* unsupportedOpen( void* opaque, const char* filename )
{
  ( void )opaque;
  ( void )filename;
  PHYSFS_setErrorCode( PHYSFS_ERR_UNSUPPORTED );
  return nullptr;
}

// mkdir/remove are unsupported
extern "C" static int unsupportedOperation( void* opaque, const char* filename )
{
  ( void )opaque;
  ( void )filename;
  PHYSFS_setErrorCode( PHYSFS_ERR_UNSUPPORTED );
  return -1;
}

extern "C" static int stat( void* opaque, const char* filename, PHYSFS_Stat* stat )
{
  try
  {
    BFSArchive* archive = reinterpret_cast< BFSArchive* >( opaque );
    return archive->stat( filename, *stat );
  }
  catch( PHYSFS_ErrorCode code )
  {
    PHYSFS_setErrorCode( code );
    return 0;
  }
}

static const PHYSFS_Archiver s_bfsArchiver
{
  0, // Version
  {
    "bfs",
    "FlatOut 2 BFS Archive",
    "Willi Schinmeyer",
    "http://github.com/mrwonko/physfs-bfs",
    0 // no support for symbolic links
  },
  openArchive,
  enumerateFiles,
  openRead,
  unsupportedOpen,
  unsupportedOpen,
  unsupportedOperation,
  unsupportedOperation,
  stat,
  closeArchive
};

extern "C" int registerBfsArchiver()
{
  return PHYSFS_registerArchiver( &s_bfsArchiver );
}
