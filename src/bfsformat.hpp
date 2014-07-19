#pragma once

#include <cstdint>

struct BFSHeader
{
  enum
  {
    HASH_SIZE = 0x3e5,
    HEADER_SIZE = 0x14 + HASH_SIZE * 8,
  };

  char fileId[ 4 ]; // "bfs1"
  char unused[ 4 ];
  std::uint32_t flagAndHeaderSize;
  std::uint32_t fileCount;
  std::uint32_t hashSize;

  bool flag() const { return ( flagAndHeaderSize >> 31 ) & 1 ; }
  std::uint32_t headerSize( ) const { return flagAndHeaderSize & ~( 1 << 31 ); }
};

struct BFSStringPoolHeader
{
  std::uint32_t end; // Size of the stringpool block
  std::uint32_t offsetsOffset; // location of the offsets into the packed strings
  std::uint32_t uncompressedSizesOffset; // location of unpacked sizes of strings
  std::uint32_t huffmannTreeOffset; // location of serialized huffmann tree for decoding
  std::uint32_t compressedStringsOffset; // location of packed string data
};

struct BFSFileInfo
{
  std::uint32_t compressionType;
  std::uint32_t offset;
  std::uint32_t uncompressedSize;
  std::uint32_t compressedSize;
  char ignored[ 4 ];
  std::uint16_t dirStringIndex;
  std::uint16_t fileStringIndex;
};
