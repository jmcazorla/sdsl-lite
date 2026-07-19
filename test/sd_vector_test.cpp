#include "sdsl/sd_vector.hpp"
#include "sdsl/bit_vectors.hpp"
#include "gtest/gtest.h"

using namespace sdsl;
using namespace std;

namespace
{

const size_t BV_SIZE = 1000000;

template<class T>
class sd_vector_test : public ::testing::Test { };

using testing::Types;

typedef Types<
sd_vector<>,
          sd_vector<rrr_vector<63>>
          > Implementations;

TYPED_TEST_CASE(sd_vector_test, Implementations);

TYPED_TEST(sd_vector_test, iterator_constructor)
{
    std::vector<uint64_t> pos;
    bit_vector bv(BV_SIZE);
    std::mt19937_64 rng;
    std::uniform_int_distribution<uint64_t> distribution(0, 9);
    auto dice = bind(distribution, rng);
    for (size_t i=0; i < bv.size(); ++i) {
        if (0 == dice()) {
            pos.emplace_back(i);
            bv[i] = 1;
        }
    }
    TypeParam sdv(pos.begin(),pos.end());
    for (size_t i=0; i < bv.size(); ++i) {
        ASSERT_EQ((bool)sdv[i],(bool)bv[i]);
    }
}

TYPED_TEST(sd_vector_test, builder_constructor)
{
    std::vector<uint64_t> pos;
    bit_vector bv(BV_SIZE);
    std::mt19937_64 rng;
    std::uniform_int_distribution<uint64_t> distribution(0, 9);
    auto dice = bind(distribution, rng);
    size_t ones = 0;
    for (size_t i=0; i < bv.size(); ++i) {
        if (0 == dice()) {
            pos.emplace_back(i);
            bv[i] = 1;
            ones++;
        }
    }
    sd_vector_builder builder(BV_SIZE, ones);
    for (auto i : pos) {
        builder.set(i);
    }
    TypeParam sdv(builder);
    for (size_t i=0; i < bv.size(); ++i) {
        ASSERT_EQ((bool)sdv[i],(bool)bv[i]);
    }
}

TYPED_TEST(sd_vector_test, get_uint64_128_256_fast)
{
    bit_vector bv(256, 0);
    const std::vector<size_t> ones = {0, 1, 2, 63, 64, 65, 66, 127, 128, 129, 130, 131, 132, 198, 199, 200, 255};
    for (auto pos : ones) {
        bv[pos] = 1;
    }
    TypeParam sdv(bv);

    auto build_value = [](const bit_vector& bits, size_t start, size_t len) {
        uint64_t value = 0;
        for (size_t i = 0; i < len; ++i) {
            if (start + i < bits.size() && bits[start + i]) {
                value |= (uint64_t(1) << i);
            }
        }
        return value;
    };

    const uint64_t expected_64 = build_value(bv, 0, 64);
    const uint64_t expected_64_hi = build_value(bv, 64, 64);
    const uint64_t expected_64_hi_hi = build_value(bv, 128, 64);
    const uint64_t expected_64_hi_hi_hi = build_value(bv, 192, 64);

    const uint128_t expected_128 = static_cast<uint128_t>(expected_64) | (static_cast<uint128_t>(expected_64_hi) << 64);
    const uint128_t expected_256_high = static_cast<uint128_t>(expected_64_hi_hi) | (static_cast<uint128_t>(expected_64_hi_hi_hi) << 64);
    const uint256_t expected_256(expected_64, expected_64_hi, expected_256_high);

    ASSERT_EQ(expected_64, sdv.get_uint64(0));
    ASSERT_EQ(expected_128, sdv.get_uint128(0));
    ASSERT_EQ(expected_256, sdv.get_uint256(0));
}

TYPED_TEST(sd_vector_test, builder_empty_constructor)
{
    sd_vector_builder builder(BV_SIZE, 0UL);
    TypeParam sdv(builder);
    for (size_t i=0; i < BV_SIZE; ++i) {
        ASSERT_FALSE((bool)sdv[i]);
    }
}

} // end namespace

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

