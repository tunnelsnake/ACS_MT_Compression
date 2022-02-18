#ifndef THREAD_SAFE_PRIORITY_QUEUE_H
#define THREAD_SAFE_PRIORITY_QUEUE_H

#include <queue>
#include <mutex>
#include <cassert>
#include <optional>
#include <vector>

// In order to successfully instantiate, one must pass a compare functions
template <class T, class Container = std::vector<T>, class Compare = std::less<typename Container::value_type>>
class ThreadSafePriorityQueue
{
public:
    ThreadSafePriorityQueue(const Compare &compare = Compare()) : _m(), _q(compare) {}

    ~ThreadSafePriorityQueue()
    {
    }

    void push(T elem)
    {
        std::lock_guard<std::mutex> lock(_m);
        _q.push(elem);
    }

    void emplace(T elem)
    {
        std::lock_guard<std::mutex> lock(_m);
        _q.emplace(elem);
    }

    // Attempts to get an element, ptr is null if unsuccessful
    std::optional<T> try_pop_front()
    {
        std::lock_guard<std::mutex> lock(_m);
        if (_q.size() == 0)
        {
            return std::nullopt;
        }
        auto tmp = _q.top();
        _q.pop();
        return tmp;
    }

    unsigned int size()
    {
        std::lock_guard<std::mutex> lock(_m);
        return _q.size();
    }

private:
    mutable std::mutex _m;
    std::priority_queue<T, Container, Compare> _q;
};

#endif //THREAD_SAFE_PRIORITY_QUEUE_H