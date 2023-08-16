#include "gtest/gtest.h"

#include "AlignedAllocator.h"
#include <cstddef>
#include <cstdint>

TEST(AlignedAllocator, CopyConstructor)
{
    AlignedAllocator<short, 4096> s;
    AlignedAllocator<int, 4096> i(s);

    ASSERT_EQ(s, i);
}

TEST(AlignedAllocator, AllocateOne)
{
    AlignedAllocator<int, 4096> alloc;

    int* p = alloc.allocate(1);
    ASSERT_TRUE(p);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(p) % 4096, 0);

    alloc.deallocate(p, 1);
}

TEST(AlignedAllocator, AllocateMany)
{
    AlignedAllocator<int, 4096> alloc;

    int* p = alloc.allocate(42);
    ASSERT_TRUE(p);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(p) % 4096, 0);

    alloc.deallocate(p, 1);
}

TEST(AlignedAllocator, Comparison)
{
    AlignedAllocator<char, 512> c;
    AlignedAllocator<short, 4096> s;
    AlignedAllocator<int, 4096> i;

    ASSERT_NE(c, i);
    ASSERT_EQ(s, i);
}
