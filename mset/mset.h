#ifndef MSET_H
#define MSET_H

#include <vector>

using std::vector;

typedef struct Pair {
    uint16_t nums;    // b 0:15
    uint16_t divides; // v

    Pair(uint16_t n, uint16_t d) : nums(n), divides(d) {}
} Pair;

class MSET {
  public:
    static Pair makePair(const uint64_t &data) {
        // (2^4)^2^4-1 possible various for polydimentional diff
        assert((data & 0xF000000000000000) == 0);

        uint8_t counter[16] = {0};
        uint16_t nums = 0;
        uint16_t divides = 0;

        for (int shift = 0; shift < 64; shift += 4) {
            uint8_t halfByte = (data >> shift) & 0x0F;
            ++counter[halfByte];
        }

        uint8_t agg = 0;
        for (int i = 0; i < 16; i++) {
            if (counter[i] > 0) {
                nums = nums | (0x01 << i);
                agg += counter[i];
                divides = divides | (0x01 << agg);
            }
        }

        return Pair(nums, divides);
    }

    static uint64_t makeData(const Pair &pair) {
        uint64_t result = 0;
        uint8_t prev = 0;
        uint8_t next = 0;

        for (uint8_t i = 0; i < 0x10; ++i) {
            if (pair.nums & (0x01 << i)) {
                for (; next < 16; ++next) {
                    if (pair.divides & (0x01 << next)) {
                        break;
                    }
                }

                for (int num = 0; num < next - prev; ++num) {
                    result = (result << 4) | (uint64_t)(i & 0x0F);
                }
                prev = next;
                ++next;
            }
        }

        return result;
    }

    static void PrintPair(const Pair &pair) {
        for (uint32_t i = 0; i < 0x10; ++i) {
            std::cout << std::hex << i;
        }
        printf("\n");
        for (uint32_t i = 1; i < 0x10000; i *= 2) {
            if (pair.nums & i)
                std::cout << "1";
            else
                std::cout << "0";
        }
        printf("  - nums\n");
        for (uint32_t i = 1; i < 0x10000; i *= 2) {
            if (pair.divides & i)
                std::cout << "1";
            else
                std::cout << "0";
        }
        printf("  - divides\n");
    }

  private:
    vector<Pair> mset;
};

#endif // MSET_H