//
// Created by v4kst1z on 2021/6/26.
//

#include "IOBuffer.h"

IOBuffer::IOBuffer(size_t init_size, int prepend_size)  :
    buffer_(prepend_size + init_size),
    read_idx_(prepend_size),
    write_idx_(prepend_size) {}

size_t IOBuffer::GetReadAbleSize() {
    return write_idx_ - read_idx_;
}

size_t IOBuffer::GetWriteAbleSize() {
    return buffer_.size() - write_idx_;
}

size_t IOBuffer::GetPrependSize() {
    return read_idx_;
}

const char *IOBuffer::GetReadAblePtr() const {
    return &*buffer_.begin() + read_idx_;
}

const char *IOBuffer::GetWriteAblePtr() const {
    return &*buffer_.begin() + write_idx_;
}

char *IOBuffer::GetWriteAblePtr() {
    return &*buffer_.begin() + write_idx_;
}

void IOBuffer::AllocSpace(size_t len) {
    if(GetWriteAbleSize() + GetPrependSize() - 8 < len)
        buffer_.resize(write_idx_ + len);
    else {
        size_t read_size = GetReadAbleSize();
        std::copy(&*buffer_.begin() + read_idx_, &*buffer_.begin() + write_idx_, &*buffer_.begin() + 8);
        read_idx_ = 8;
        write_idx_ = read_size + 8;
    }
}

void IOBuffer::AppendData(const char *data, size_t len) {
    if(GetWriteAbleSize() < len) {
        AllocSpace(len);
    }
    std::copy(data, data + len, GetWriteAblePtr());
    write_idx_ += len;
}

void IOBuffer::Append(const std::string &str) {
    AppendData(str.data(), str.size());
}

void IOBuffer::ResetId() {
    read_idx_ = 8;
    write_idx_ = 8;
}


