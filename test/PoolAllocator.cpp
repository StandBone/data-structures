#include "gtest/gtest.h"

#include "PoolAllocator.h"

TEST(PoolAllocator, EmptySharedPool)
{
    _shared_pool<int> pool;

    ASSERT_EQ(pool.load(), nullptr);
}

TEST(PoolAllocator, TrySharedPool)
{
    _shared_pool<int> pool;
    _node<int>* a = static_cast<_node<int>*>(::operator new(sizeof(_node<int>)));
    _node<int>* expected = nullptr;

    a->next = nullptr; pool.try_compare_exchange(expected, a);
    ASSERT_EQ(expected, nullptr);
    ASSERT_EQ(pool.load(), a);
}

TEST(PoolAllocator, SetSharedPool)
{
    _shared_pool<int> pool;
    _node<int>* a = static_cast<_node<int>*>(::operator new(sizeof(_node<int>)));
    _node<int>* expected = nullptr;
    a->next = nullptr; pool.try_compare_exchange(expected, a);

    pool.compare_exchange(expected, nullptr);
    ASSERT_EQ(expected, a);
    ASSERT_EQ(pool.load(), nullptr);

    ::operator delete(static_cast<void*>(a));
}

TEST(PoolAllocator, EmptyLocalPool)
{
    _shared_pool<int> shared;
    _local_pool<int> local(shared);

    ASSERT_EQ(static_cast<_node<int>*>(local), nullptr);
}

TEST(PoolAllocator, UsedLocalPool)
{
    _shared_pool<int> shared;
    _local_pool<int> local(shared);
    _node<int>* a = static_cast<_node<int>*>(::operator new(sizeof(_node<int>)));

    a->next = nullptr; local = a;
    ASSERT_EQ(local, a);
}

TEST(PoolAllocator, UsedSharedPool)
{
    _shared_pool<int> shared;
    _node<int>* a = static_cast<_node<int>*>(::operator new(sizeof(_node<int>)));
    _node<int>* b = static_cast<_node<int>*>(::operator new(sizeof(_node<int>)));
    _node<int>* expected = nullptr;
    a->next = nullptr; shared.try_compare_exchange(expected, a);

    {
        _local_pool<int> local(shared);
        b->next = nullptr; local = b;
    }
    ASSERT_EQ(b, shared.load());
}

TEST(PoolAllocator, Allocate)
{
    _pool_allocator<int> pool;

    int* a = pool.allocate(1);
    ASSERT_NE(a, nullptr);

    pool.deallocate(a, 1);
}

TEST(PoolAllocator, Reuse)
{
    _pool_allocator<int> pool;
    int* a = static_cast<int*>(::operator new(sizeof(_node<int>)));
    pool.add_to_shared_pool(a);

    pool.move_from_shared_to_local_pool();
    int* b = pool.reuse_from_local_pool();
    ASSERT_EQ(b, a);
}

TEST(PoolAllocator, FailReuse)
{
    _pool_allocator<int> pool;

    int* a = pool.reuse_from_local_pool();
    ASSERT_EQ(a, nullptr);
}

TEST(PoolAllocator, FailMoveToLocalPool)
{
    _pool_allocator<int> pool;
    int* a = static_cast<int*>(::operator new(sizeof(_node<int>)));
    int* b = static_cast<int*>(::operator new(sizeof(_node<int>)));
    pool.add_to_shared_pool(a);
    pool.move_from_shared_to_local_pool();
    pool.add_to_shared_pool(b);

    pool.move_from_shared_to_local_pool();
    int* c = pool.reuse_from_local_pool();
    ASSERT_EQ(c, a);
}

TEST(PoolAllocator, MoveBackToSharedPool)
{
    _pool_allocator<int> pool;
    int* a = static_cast<int*>(::operator new(sizeof(_node<int>)));
    int* b = static_cast<int*>(::operator new(sizeof(_node<int>)));
    pool.add_to_shared_pool(a);
    pool.add_to_shared_pool(b);
    pool.move_from_shared_to_local_pool();
    b = pool.reuse_from_local_pool();

    pool.move_from_local_to_shared_pool();
    int* c = pool.reuse_from_local_pool();
    ASSERT_EQ(c, nullptr);
}

TEST(PoolAllocator, FailMoveBackToSharedPool)
{
    _pool_allocator<int> pool;
    int* a = static_cast<int*>(::operator new(sizeof(_node<int>)));
    int* b = static_cast<int*>(::operator new(sizeof(_node<int>)));
    pool.add_to_shared_pool(a);
    pool.move_from_shared_to_local_pool();
    pool.add_to_shared_pool(b);

    pool.move_from_local_to_shared_pool();
    int* c = pool.reuse_from_local_pool();
    ASSERT_EQ(c, a);
}

TEST(PoolAllocator, CopyConstructor)
{
    PoolAllocator<short> s;
    PoolAllocator<int> i = s;

    ASSERT_EQ(s, i);
}

TEST(PoolAllocator, AllocateOneFallback)
{
    PoolAllocator<long> alloc;

    long* p = alloc.allocate(1);
    EXPECT_TRUE(p != nullptr);

    alloc.deallocate(p, 1);
}

TEST(PoolAllocator, AllocateRuse)
{
    PoolAllocator<long> alloc;
    long* p = alloc.allocate(1);
    alloc.deallocate(p, 1);

    p = alloc.allocate(1);
    EXPECT_TRUE(p != nullptr);

    alloc.deallocate(p, 1);
}

TEST(PoolAllocator, AllocateMany)
{
    PoolAllocator<long> alloc;

    long* p = alloc.allocate(42);
    EXPECT_TRUE(p != nullptr);

    alloc.deallocate(p, 42);
}

TEST(PoolAllocator, Comparison)
{
    PoolAllocator<short> s;
    PoolAllocator<int> i;

    ASSERT_EQ(s, i);
}
