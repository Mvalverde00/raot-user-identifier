#include "byte_reader.h"

#include <iostream>

byte ByteReader::ReadByte() {
  return buffer_[position_++];
}

byte ByteReader::PeakByte() {
  return buffer_[position_];
}

uint16_t ByteReader::ReadUInt16() {
  uint16_t ret = 0;
  ret |= ReadByte();
  ret |= (uint16_t)(ReadByte() << 8);
  return ret;
}

uint32_t ByteReader::ReadUInt32() {
  uint32_t ret = 0;
  ret |= ReadByte();
  ret |= (ReadByte() << 8);
  ret |= (ReadByte() << 16);
  ret |= (ReadByte() << 24);
  return ret;
}
int ByteReader::ReadInt32() {
  return (int)ReadUInt32();
}
int ByteReader::PeakInt32() const{
  int ret = 0;
  ret |= buffer_[position_];
  ret |= (buffer_[position_ + 1] << 8);
  ret |= (buffer_[position_ + 2] << 16);
  ret |= (buffer_[position_ + 3] << 24);
  return ret;
}


// Zigzag decoding
int ByteReader::ReadPackedInt32() {
  int data = ReadPackedUInt32();
  return (int)((data >> 1) ^ -(data & 1));
}
uint32_t ByteReader::ReadPackedUInt32() {
  uint64_t data = ReadPackedUInt64();
  if (data >= 0x100000000) {
    std::cerr << "Possible overflow when reading packed uint32: " << data << "\n";
  }

  return (uint32_t)data;
}


// Zigzag decoding
int64_t ByteReader::ReadPackedInt64() {
  int64_t data = ReadPackedUInt64();
  return ((int64_t)(data >> 1)) ^ -((int64_t)data & 1);
}

uint64_t ByteReader::ReadPackedUInt64() {
  byte a0 = ReadByte();
  if (a0 < 241)
  {
    return a0;
  }

  byte a1 = ReadByte();
  if (a0 >= 241 && a0 <= 248)
  {
    return 240 + ((a0 - (uint64_t)241) << 8) + a1;
  }

  byte a2 = ReadByte();
  if (a0 == 249)
  {
    return 2288 + ((uint64_t)a1 << 8) + a2;
  }

  byte a3 = ReadByte();
  if (a0 == 250)
  {
    return a1 + (((uint64_t)a2) << 8) + (((uint64_t)a3) << 16);
  }

  byte a4 = ReadByte();
  if (a0 == 251)
  {
    return a1 + (((uint64_t)a2) << 8) + (((uint64_t)a3) << 16) + (((uint64_t)a4) << 24);
  }

  byte a5 = ReadByte();
  if (a0 == 252)
  {
    return a1 + (((uint64_t)a2) << 8) + (((uint64_t)a3) << 16) + (((uint64_t)a4) << 24) + (((uint64_t)a5) << 32);
  }

  byte a6 = ReadByte();
  if (a0 == 253)
  {
    return a1 + (((uint64_t)a2) << 8) + (((uint64_t)a3) << 16) + (((uint64_t)a4) << 24) + (((uint64_t)a5) << 32) + (((uint64_t)a6) << 40);
  }

  byte a7 = ReadByte();
  if (a0 == 254)
  {
    return a1 + (((uint64_t)a2) << 8) + (((uint64_t)a3) << 16) + (((uint64_t)a4) << 24) + (((uint64_t)a5) << 32) + (((uint64_t)a6) << 40) + (((uint64_t)a7) << 48);
  }

  byte a8 = ReadByte();
  if (a0 == 255)
  {
    return a1 + (((uint64_t)a2) << 8) + (((uint64_t)a3) << 16) + (((uint64_t)a4) << 24) + (((uint64_t)a5) << 32) + (((uint64_t)a6) << 40) + (((uint64_t)a7) << 48) + (((uint64_t)a8) << 56);
  }

  std::cerr << "FATAL ERROR attempting to read packed ulong\n";
}


std::vector<byte> ByteReader::ReadBytes(int n_bytes) {
  if (position_ + n_bytes > length_) {
    std::cerr << "FATAL ERROR reading " << n_bytes << " bytes from reader \n";
  }

  std::vector<byte> ret(n_bytes);
  for (int i = 0; i < n_bytes; i++) {
    ret[i] = buffer_[position_ + i];
  }
  position_ += n_bytes;
  return ret;
}

std::string ByteReader::ReadString() {
  uint16_t size = ReadUInt16();

  if (size == 0) return "";

  int realsize = size - 1;
  std::vector<byte> string_bytes = ReadBytes(realsize);

  std::string msg(reinterpret_cast<char const*>(string_bytes.data()), realsize);
  return msg;
}


int ByteReader::Size() {
  return length_ - position_;
}
