#pragma once

#include "_node.h"
#include <atomic>

template <class T>
class _shared_pool
{
public:
    ~_shared_pool();

    constexpr _node<T>* load() const noexcept;
    constexpr bool try_compare_exchange(_node<T>*& expected, _node<T>* desired) noexcept;
    constexpr void compare_exchange(_node<T>*& expected, _node<T>* desired) noexcept;

private:
    std::atomic<_node<T>*> head = nullptr;
};

template <class T>
inline _shared_pool<T>::~_shared_pool()
{
    _node<T>* p = head.load();
    while (p != nullptr)
    {
        head.compare_exchange_weak(p, p->next);
        ::operator delete(static_cast<void*>(p));
        p = head.load();
    }
}

template <class T>
inline constexpr _node<T>* _shared_pool<T>::load() const noexcept
{
    return head.load();
}

template <class T>
inline constexpr bool _shared_pool<T>::try_compare_exchange(_node<T>*& expected, _node<T>* desired) noexcept
{
    return head.compare_exchange_weak(expected, desired);
}

template <class T>
inline constexpr void _shared_pool<T>::compare_exchange(_node<T>*& expected, _node<T>* desired) noexcept
{
    while (!head.compare_exchange_weak(expected, desired))
    {
    }
}
