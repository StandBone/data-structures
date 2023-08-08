#pragma once

#include <atomic>
#include <type_traits>
#include <cstddef>

/**
 * @brief   TODO
 * 
 * TODO contrast to default allocator
 */
template <class T>
class PoolAllocator
{
public:
    /**
     * @brief   Element type
     */
    using value_type = T;

    /**
     * @brief   Quantities of elements
     */
    using size_type = size_t;

    /**
     * @brief   Difference between two pointers
     */
    using difference_type = ptrdiff_t;

    /**
     * @brief   Indicates  that  the  PoolAllocator  shall  propagate  when  the
     *          container is move-assigned
     */
    using propagate_on_container_move_assignment = std::true_type;

    /**
     * @brief   Construct PoolAllocator object
     */
    constexpr PoolAllocator() noexcept = default;

    /**
     * @brief   Copy-construct PoolAllocator object
     */
    constexpr PoolAllocator(const PoolAllocator& x) noexcept = default;

    /**
     * @copydoc PoolAllocator(const PoolAllocator&)
     */
    template <class U>
    constexpr PoolAllocator(const PoolAllocator<U>& x) noexcept;

    /**
     * @brief    Destructs the PoolAllocator object
     */
    constexpr ~PoolAllocator() = default;

    /**
     * @brief   Allocate block of storage
     *
     * Attempts  to  allocate  a  block of storage with a size  large  enough to
     * contain  n elements of member  type value_type,  and returns a pointer to
     * the first element.
     *
     * The storage is aligned appropriately for objects of type value_type,  but
     * they are not constructed.
     *
     * TODO contrast to default allocator
     *
     * @param   n   Number of elements to be allocated.
     *
     * @return   A pointer to the first element in the block of storage.
     *
     * @throws  std::bad_alloc   if the amount of storage requested could not be
     *                           allocated.
     */
    [[nodiscard]] constexpr value_type* allocate(size_type n);

    /**
     * @brief   Release block of storage
     *
     * Releases a block of storage previously allocated with member allocate.
     *
     * The elements in the  array  are  not destroyed by a  call to this  member
     * function.
     *
     * TODO contrast to default allocator
     *
     * @param   p   Pointer  to  a  block of storage  previously  allocated with
     *              PoolAllocator::allocate(size_type).
     * @param   n   Number    of   elements   allocated    on    the   call   to
     *              PoolAllocator::allocate(size_type)   for   this   block   of
     *              storage.
     */
    constexpr void deallocate(value_type* p, size_type n);

    /**
     * @brief   Compares the PoolAllocators lhs and rhs
     *
     * Performs a  comparison operation between  the PoolAllocator lhs  and rhs.
     * Always evaluates to true.
     *
     * @param   lhs,rhs   PoolAllocators to be compared
     *
     * @return  true
     */
    template <class T1, class T2>
    friend constexpr bool operator==(const PoolAllocator<T1>&, const PoolAllocator<T2>&) noexcept;

private:
    union Node
    {
        Node* next;
        value_type value;
    };

    class LocalPool
    {
    public:
        Node* pool = nullptr;
        ~LocalPool();
    };

    class SharedPool
    {
    public:
        std::atomic<Node*> pool = nullptr;
        ~SharedPool();
    };

    static SharedPool shared;
    static thread_local LocalPool local;
};

template <class T>
template <class U>
inline constexpr PoolAllocator<T>::PoolAllocator(const PoolAllocator<U>& x) noexcept
{
}

template <class T>
inline constexpr typename PoolAllocator<T>::value_type* PoolAllocator<T>::allocate(size_type n)
{
    if (n == 1)
    {
        if (local.pool == nullptr)
        {
            // move nodes from shared.pool to local.pool
            local.pool = shared.pool.load();
            while (!shared.pool.compare_exchange_weak(local.pool, nullptr))
            {
            }
        }

        if (local.pool != nullptr)
        {
            // allocate from local.pool
            Node* p = local.pool;
            local.pool = p->next;

            // move nodes from local.pool back to shared.pool
            Node* null_ptr = nullptr;
            if (shared.pool.compare_exchange_weak(null_ptr, local.pool))
            {
                local.pool = nullptr;
            }

            return reinterpret_cast<value_type*>(p);
        }
    }

    return static_cast<value_type*>(::operator new(n * sizeof(value_type)));
}

template <class T>
inline constexpr void PoolAllocator<T>::deallocate(value_type* p, size_type n)
{
    if (n == 1)
    {
        // add node to shared.pool
        Node* node = reinterpret_cast<Node*>(p);
        node->next = shared.pool.load();
        while (!shared.pool.compare_exchange_weak(node->next, node))
        {
        }
    }
    else
    {
        ::operator delete(static_cast<void*>(p));
    }
}

template <class T1, class T2>
inline constexpr bool operator==(const PoolAllocator<T1>&, const PoolAllocator<T2>&) noexcept
{
    return true;
}

template <class T>
inline PoolAllocator<T>::LocalPool::~LocalPool()
{
    Node* null_ptr = nullptr;
    if (!shared.pool.compare_exchange_weak(null_ptr, local.pool))
    {
        PoolAllocator alloc;
        while (pool != nullptr)
        {
            Node* p = pool;
            pool = p->next;
            alloc.deallocate(reinterpret_cast<T*>(p), 1);
        }
    }
}

template <class T>
inline PoolAllocator<T>::SharedPool::~SharedPool()
{
    Node* p = pool.load();
    while (p != nullptr)
    {
        pool.compare_exchange_weak(p, p->next);
        delete p;
        p = pool.load();
    }
}

template <class T>
typename PoolAllocator<T>::SharedPool PoolAllocator<T>::shared;

template <class T>
thread_local typename PoolAllocator<T>::LocalPool PoolAllocator<T>::local;
