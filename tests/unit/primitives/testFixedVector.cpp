#include <gtest/gtest.h>
#include "inc/core/primitives/FixedVector.h"

using slr::FixedVector;
 
struct Counter {
    int value = 0;
    static inline int copies = 0;
    static inline int moves = 0;

    Counter() = default;
    Counter(int v) : value(v) {}

    Counter(const Counter& other) : value(other.value) {
        ++copies;
    }

    Counter(Counter&& other) noexcept : value(other.value) {
        ++moves;
    }

    Counter& operator=(const Counter& other) {
        value = other.value;
        ++copies;
        return *this;
    }

    Counter& operator=(Counter&& other) noexcept {
        value = other.value;
        ++moves;
        return *this;
    }

    static void reset() {
        copies = 0;
        moves = 0;
    }
};

TEST(FixedVectorTest, StartsEmpty) {
    FixedVector<int, 4> v;
    EXPECT_EQ(v.size(), 0u);
}

TEST(FixedVectorTest, PushBackIncreasesSize) {
    FixedVector<int, 4> v;
    EXPECT_TRUE(v.push_back(1));
    EXPECT_TRUE(v.push_back(2));
    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
}

TEST(FixedVectorTest, PushBackStopsAtCapacity) {
    FixedVector<int, 2> v;
    EXPECT_TRUE(v.push_back(1));
    EXPECT_TRUE(v.push_back(2));
    EXPECT_FALSE(v.push_back(3));
    EXPECT_EQ(v.size(), 2u);
}

TEST(FixedVectorTest, ClearResetsLogicalSizeOnly) {
    FixedVector<int, 4> v;
    v.push_back(1);
    v.push_back(2);

    v.clear();
    EXPECT_EQ(v.size(), 0u);

    EXPECT_TRUE(v.push_back(3));
    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 3);
}

TEST(FixedVectorTest, ConstAccessWorks) {
    FixedVector<int, 4> v;
    v.push_back(42);

    const auto& cv = v;
    EXPECT_EQ(cv[0], 42);
}

TEST(FixedVectorTest, CopyPushBackUsesCopy) {
    Counter::reset();
    FixedVector<Counter, 4> v;

    Counter c{10};
    v.push_back(c);

    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0].value, 10);
    EXPECT_EQ(Counter::copies, 1);
}

TEST(FixedVectorTest, MovePushBackUsesMove) {
    Counter::reset();
    FixedVector<Counter, 4> v;

    v.push_back(Counter{20});

    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0].value, 20);
    EXPECT_EQ(Counter::moves, 1);
}

TEST(FixedVectorTest, OrderPreserved) {
    FixedVector<int, 5> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);

    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

#ifndef NDEBUG
TEST(FixedVectorTest, OperatorIndexAssertsOnInvalidAccess) {
    FixedVector<int, 2> v;
    EXPECT_DEATH(v[0], "");
}
#endif


int main(int argc, char* argv[]) {
    // loguru::init(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}