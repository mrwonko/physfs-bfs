# PhysFS FlatOut 2 BFS Support

WIP support for FlatOut 2's .bfs archive format for [PhysicsFS](http://icculus.org/physfs/). Written in C++11, but with a C interface.

## State

* can read file list, so enumeration and stat() works
* both compressed and uncompressed files can be read
* Code itself is fairly clean and cross-platform (using PhysFS's code), but the project is currently only for MSVC2013 (will switch to CMake eventually)
