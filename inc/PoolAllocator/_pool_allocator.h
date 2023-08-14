#pragma once

#include "_node.h"
#include "_local_pool.h"
#include "_shared_pool.h"

template <class T>
class _pool_allocator
{
public:
    constexpr T*   allocate(size_t n);
    constexpr void add_to_shared_pool(T* p);
    constexpr void move_from_shared_to_local_pool() noexcept;
    constexpr T*   reuse_from_local_pool();
    constexpr void move_from_local_to_shared_pool() noexcept;
    constexpr void deallocate(T* p, size_t n);

private:
    static _shared_pool<T> shared_pool;
    static thread_local _local_pool<T> local_pool;
};

template <class T>
inline constexpr T* _pool_allocator<T>::allocate(size_t n)
{
    return static_cast<T*>(::operator new(n * sizeof(_node<T>)));
}

template <class T>
inline constexpr void _pool_allocator<T>::add_to_shared_pool(T* p)
{
    _node<T>* node = reinterpret_cast<_node<T>*>(p);
    node->next = shared_pool.load();
    shared_pool.compare_exchange(node->next, node);
}

template <class T>
inline constexpr void _pool_allocator<T>::move_from_shared_to_local_pool() noexcept
{
    if (local_pool == nullptr)
    {
        local_pool = shared_pool.load();
        shared_pool.compare_exchange(local_pool, nullptr);
    }
}

template <class T>
inline constexpr T* _pool_allocator<T>::reuse_from_local_pool()
{
    _node<T>* p = local_pool;
    if (p != nullptr)
    {
        local_pool = p->next;
    }
    return reinterpret_cast<T*>(p);
}

template <class T>
inline constexpr void _pool_allocator<T>::move_from_local_to_shared_pool() noexcept
{
    _node<T>* null_ptr = nullptr;
    if (shared_pool.try_compare_exchange(null_ptr, local_pool))
    {
        local_pool = nullptr;
    }
}

template <class T>
inline constexpr void _pool_allocator<T>::deallocate(T* p, size_t n)
{
    ::operator delete(static_cast<void*>(p), n * sizeof(_node<T>));
}

template <class T>
_shared_pool<T> _pool_allocator<T>::shared_pool;

template <class T>
thread_local _local_pool<T> _pool_allocator<T>::local_pool(shared_pool);
