#pragma once

#include <memory>
#include <stdexcept>

#include <iostream>

class BitStream;

class UnexpectedEnfOfInput : public std::runtime_error
{
public:
  UnexpectedEnfOfInput() : std::runtime_error( "Unexpected end of input" ) {}
};

class Huffmann
{
public:
  Huffmann( const char * const begin, const char * const end );
  ~Huffmann() = default;
  Huffmann( const Huffmann& rhs ) = delete;
  Huffmann& operator=( const Huffmann& rhs ) = delete;

  /**
  @return Whether eof was hit during read
  **/
  bool decode( BitStream& stream, char& out_result ) const;

private:
  Huffmann() = default;
  void deserialize( const char *& cur, const char * const end );

private:
  std::unique_ptr< Huffmann > m_child0;
  std::unique_ptr< Huffmann > m_child1;
  char m_leaf;
};
