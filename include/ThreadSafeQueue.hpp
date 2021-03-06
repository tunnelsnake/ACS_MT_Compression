#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <cassert>
#include <optional>


template <class T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue() : _m(), _q() {}

    ~ThreadSafeQueue()
    {
    }

    void push_front(T elem)
    {
        std::lock_guard<std::mutex> lock(_m);
        _q.push(elem);
    }

    void emplace(T elem)
    {
        std::lock_guard<std::mutex> lock(_m);
        _q.emplace(elem);
    }

    T pop_front()
    {
        std::lock_guard<std::mutex> lock(_m);
        assert(_q.size() != 0); // Todo: replace with runtime error
        auto tmp = _q.front();
        _q.pop();
        return tmp;
    }

    // Attempts to get an element, ptr is null if unsuccessful
    std::optional<T> try_pop_front()
    {
        std::lock_guard<std::mutex> lock(_m);
        if(_q.size() == 0) {
            return std::nullopt;
        }
        auto tmp = _q.front();
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
    std::queue<T> _q;
};

#endif //THREAD_SAFE_QUEUE_H