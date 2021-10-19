#ifndef __BYTE_READER_H__
#define __BYTE_READER_H__

#include <gsl/span>
#include <string>
#include <vector>

typedef uint8_t byte;
typedef gsl::span<byte> byte_span;

class ByteReader {

public:
  ByteReader() : buffer_(), position_(0), length_(0) {};

  ByteReader(std::vector<byte>& bytes) : buffer_(bytes.data(), bytes.size()), position_(0), length_(bytes.size()) {};

  ByteReader(const ByteReader& other) : buffer_(other.buffer_.data(), other.buffer_.size()), position_(other.position_), length_(other.length_) {};


  byte ReadByte();
  byte PeakByte();

  uint16_t ReadUInt16();

  uint32_t ReadUInt32();
  int ReadInt32();
  int PeakInt32() const;

  uint32_t ReadPackedUInt32();
  int ReadPackedInt32();

  uint64_t ReadPackedUInt64();
  int64_t ReadPackedInt64();

  std::vector<byte> ReadBytes(int n_bytes);

  std::string ReadString();

  int Size();


private:
  byte_span buffer_;

  int position_;
  int length_;
};
#endif
