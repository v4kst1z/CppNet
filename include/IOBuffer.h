//
// Created by v4kst1z
//

#ifndef CPPNET_NET_IOBUFFER_H
#define CPPNET_NET_IOBUFFER_H

#include <cstddef>
#include <string>
#include <vector>

#include "Common.h"

class IOBuffer {
 public:
  explicit IOBuffer(size_t init_size = 2048, int prepend_size = 8);

  size_t GetReadAbleSize() const;
  size_t GetWriteAbleSize() const;
  size_t GetPrependSize() const;

  const char *GetReadAblePtr() const;
  const char *GetWriteAblePtr() const;
  char *GetWriteAblePtr();

  void AppendData(const char *data, size_t len);
  void Append(const std::string &str);

  void AddReadIdx(size_t);
  void AddWriteIdx(size_t);

  void ResetId();

  ~IOBuffer() = default;

  IOBuffer &operator=(const IOBuffer &) = delete;

 private:
  void AllocSpace(size_t len);

  std::vector<char> buffer_;
  size_t read_idx_;
  size_t write_idx_;
};

#endif  // CPPNET_NET_IOBUFFER_H
