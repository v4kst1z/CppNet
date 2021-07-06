//
// Created by v4kst1z
//

#ifndef CPPNET_IOBUFFER_H
#define CPPNET_IOBUFFER_H

#include <vector>
#include <cstddef>
#include <string>

#include <Common.h>

class IOBuffer {
 public:
  explicit IOBuffer(size_t init_size = 2048, int prepend_size = 8);

  const size_t GetReadAbleSize() const;
  const size_t GetWriteAbleSize() const;
  const size_t GetPrependSize() const;

  const char *GetReadAblePtr() const;
  const char *GetWriteAblePtr() const;
  char *GetWriteAblePtr();

  void AppendData(const char *data, size_t len);
  void Append(const std::string &str);

  void ResetId();

  ~IOBuffer() = default;

  IOBuffer &operator=(const IOBuffer &) = delete;
 private:
  void AllocSpace(size_t len);

  std::vector<char> buffer_;
  size_t read_idx_;
  size_t write_idx_;
};

#endif //CPPNET_IOBUFFER_H
