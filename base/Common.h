//
// Created by v4kst1z
//

#ifndef CPPNET_COMMON_H
#define CPPNET_COMMON_H

#include <functional>
#include <iostream>
#include <memory>

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  TypeName& operator=(const TypeName&)

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#endif  // CPPNET_COMMON_H