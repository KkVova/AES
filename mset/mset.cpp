#include <gtest/gtest.h>

#include "mset.h"
#include <bitset>

TEST(MSET, Representation) {
    uint64_t val = 0x0F0AF005A3824270;
    Pair pair = MSET::makePair(val);
    std::cout << std::hex << std::right << std::setfill('0') << std::setw(15) << (uint64_t)val
              << std::endl;
    MSET::PrintPair(pair);
    uint64_t test = MSET::makeData(pair);
    std::cout << std::hex << std::right << std::setfill('0') << std::setw(15) << test << std::endl;
}

TEST(MSET, Positive_Representation) {
    for (uint64_t val = 0x0F000005A3824270; val < 0x0F000005A3924270; val++) {
        Pair pair = MSET::makePair(val);
        uint64_t test = MSET::makeData(pair);

        vector<uint8_t> counterFirst(0x10, 0);
        vector<uint8_t> counterSecond(0x10, 0);

        for (int shift = 0; shift < 64; shift += 4) {
            uint8_t halfByte = (val >> shift) & 0x0F;
            ++counterFirst[halfByte];
        }

        for (int shift = 0; shift < 64; shift += 4) {
            uint8_t halfByte = (test >> shift) & 0x0F;
            ++counterSecond[halfByte];
        }

        EXPECT_EQ(counterFirst, counterSecond);
    }
}