//
// Created by v4kst1z
//

#ifndef CPPNET_PRIORITYQUEUE_H
#define CPPNET_PRIORITYQUEUE_H

#include <queue>
#include <algorithm>

template<
        typename T,
        class C = std::vector<T>,
        class Cmp = std::less<typename C::value_type>
>
class PriorityQueue : public std::priority_queue<T, C, Cmp> {
public:
    bool erase(const T& value) {
        auto it = std::find(this->c.begin(), this->c.end(), value);
        if (it != this->c.end()) {
            this->c.erase(it);
            std::make_heap(this->c.begin(), this->c.end(), this->comp);
            return true;
        }
        else {
            return false;
        }
    }
};
#endif //CPPNET_PRIORITYQUEUE_H

