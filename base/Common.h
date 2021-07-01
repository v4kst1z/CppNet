//
// Created by v4kst1z
//

#ifndef CPPNET_COMMON_H
#define CPPNET_COMMON_H

#include <memory>
#include <iostream>

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName&);             \
TypeName& operator=(const TypeName&)

#ifndef DEBUG
#define DEBUG 1 // set debug mode
#endif

#define LOG(x) do { \
if (DEBUG) { std::cerr << x << std::endl; } \
} while (0)

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}




#endif //CPPNET_COMMON_H

