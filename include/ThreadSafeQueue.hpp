#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <cassert>

template <class T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue()
    {
    }

    ~ThreadSafeQueue()
    {
    }

    void push_front(T elem)
    {
        std::lock_guard<std::mutex> lock(_m);
        _q.push(elem);
    }

    void emplace(T elem) {
        std::lock_guard<std::mutex> lock(_m);
        _q.emplace(elem);
    }

    T pop_back()
    {
        std::lock_guard<std::mutex> lock(_m);
        assert(_q.size() != 0); // Todo: replace with runtime error
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