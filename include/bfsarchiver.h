#pragma once

#if defined(_WIN32)
# define PHYSFS_BFS_DLLEXPORT __declspec(dllexport)
# define PHYSFS_BFS_DLLIMPORT __declspec(dllimport)
#else
# if __GNUC__ >= 4
#   define PHYSFS_BFS_DLLEXPORT __attribute__ ((visibility ("default")))
#   define PHYSFS_BFS_DLLIMPORT __attribute__ ((visibility ("default")))
# else
#  define PHYSFS_BFS_DLLEXPORT
#  define PHYSFS_BFS_DLLIMPORT
# endif
#endif

#ifdef PHYSFS_BFS_INTERNAL
# define PHYSFS_BFS_API PHYSFS_BFS_DLLEXPORT
#else 
# define PHYSFS_BFS_API PHYSFS_BFS_DLLIMPORT
#endif

#ifdef __cplusplus
extern "C"
{
#endif

  /**
  @return 0 on error, non-0 on success
  **/
  PHYSFS_BFS_API int registerBfsArchiver( );

#ifdef __cplusplus
}
#endif
