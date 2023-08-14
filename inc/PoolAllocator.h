#pragma once

#include "PoolAllocator/_pool_allocator.h"
#include <type_traits>
#include <cstddef>

/**
 * @brief   TODO
 *
 * TODO General allocator description
 *
 * Unlike  std::allocator,   PoolAllocator::deallocate does not free the memory,
 * but stores it into a memory  pool and recycles it on  a  subsequent  call  to
 * PoolAllocator::allocate.
 *
 * @tparam  T   Type of the elements allocated by PoolAllocator
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
     *
     * @param   x   Another PoolAllocator to construct from
     */
    constexpr PoolAllocator(const PoolAllocator& x) noexcept = default;

    /**
     * @brief   Copy-construct PoolAllocator object
     *
     * @tparam  U   Element type of x
     *
     * @param   x   Another PoolAllocator to construct from
     */
    template <class U>
    constexpr PoolAllocator(const PoolAllocator<U>& x) noexcept;

    /**
     * @brief   Destructs the PoolAllocator object
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
     * If there have been prior calls to PoolAllocator::deallocate, these blocks
     * of storage may be reused.
     *
     * @param   n   Number of elements to be allocated.
     *
     * @return  A pointer to the first element in the block of storage.
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
     * Unlike std::allocator::deallocate, the memory is stored and may be reused
     * by PoolAllocator::allocate.
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
     * @tparam  T1   Element type of lhs
     * @tparam  T2   Element type of rhs
     *
     * @param   lhs,rhs   PoolAllocators to be compared
     *
     * @return  true
     */
    template <class T1, class T2>
    friend constexpr bool operator==(const PoolAllocator<T1>& lhs, const PoolAllocator<T2>& rhs) noexcept;

private:
    _pool_allocator<T> pool;
};

template <class T>
template <class U>
inline constexpr PoolAllocator<T>::PoolAllocator(const PoolAllocator<U>&) noexcept
{
}

template <class T>
inline constexpr typename PoolAllocator<T>::value_type* PoolAllocator<T>::allocate(size_type n)
{
    if (n == 1)
    {
        pool.move_from_shared_to_local_pool();
        value_type* p = pool.reuse_from_local_pool();
        pool.move_from_local_to_shared_pool();
        if (p != nullptr)
        {
            return p;
        }
    }

    return pool.allocate(n);
}

template <class T>
inline constexpr void PoolAllocator<T>::deallocate(value_type* p, size_type n)
{
    if (n == 1)
    {
        pool.add_to_shared_pool(p);
    }
    else
    {
        pool.deallocate(p, n);
    }
}

template <class T1, class T2>
inline constexpr bool operator==(const PoolAllocator<T1>&, const PoolAllocator<T2>&) noexcept
{
    return true;
}
