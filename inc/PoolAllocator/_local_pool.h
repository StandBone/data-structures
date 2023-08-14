#pragma once

#include "_node.h"
#include "_shared_pool.h"

template <class T>
class _local_pool
{
public:
    _local_pool(_shared_pool<T>& shared);
    ~_local_pool();
    constexpr _local_pool& operator=(_node<T>* p) noexcept;
    constexpr operator _node<T>*&() noexcept;
    constexpr operator const _node<T>*() const noexcept;

private:
    _shared_pool<T>& shared;
    _node<T>* head = nullptr;
};

template <class T>
inline _local_pool<T>::_local_pool(_shared_pool<T>& shared) : shared(shared)
{
}

template <class T>
inline _local_pool<T>::~_local_pool()
{
    _node<T>* null_ptr = nullptr;
    if (!shared.try_compare_exchange(null_ptr, head))
    {
        while (head != nullptr)
        {
            _node<T>* p = head;
            head = p->next;
            p->next = shared.load();
            shared.compare_exchange(p->next, p);
        }
    }
}

template <class T>
inline constexpr _local_pool<T>& _local_pool<T>::operator=(_node<T>* p) noexcept
{
    head = p;
    return *this;
}

template <class T>
inline constexpr _local_pool<T>::operator _node<T>*&() noexcept
{
    return head;
}

template <class T>
inline constexpr _local_pool<T>::operator const _node<T>*() const noexcept
{
    return head;
}
