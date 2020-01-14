#ifndef HELPER_H
#define HELPER_H

#include <cstdint>
#include <vector>

#define PRINTHEX(x)                                                                                     \
    std::cout << std::hex << std::setfill('0')                                                          \
              << (typeid(x) == typeid(uint16_t)                                                         \
                      ? std::setw(3)                                                                    \
                      : typeid(x) == typeid(uint8_t) ? std::setw(2) : std::setw(1))                     \
              << (uint32_t)x << " | "

class uint4_t {
    std::vector<bool> value;

  public:
    uint4_t() { value = std::vector<bool>(4, false); }

    uint4_t(const uint4_t &other) { value = other.get(); }

    uint4_t(uint8_t input, bool isBigPart = false) {
        value.resize(4);
        int offset = 4 * isBigPart;
        value[0] = (input >> offset) & 1;
        value[1] = (input >> (1 + offset)) & 1;
        value[2] = (input >> (2 + offset)) & 1;
        value[3] = (input >> (3 + offset)) & 1;
    }

    //    uint4_t(uint32_t input) {
    //        value.resize(4);
    //        value[0] = input & 1;
    //        value[1] = (input >> 1) & 1;
    //        value[2] = (input >> 2) & 1;
    //        value[3] = (input >> 3) & 1;
    //    }

    std::vector<bool> get() const { return value; }

    ////////////////////////////////////////////////////////////
    // Comparison operators
    ////////////////////////////////////////////////////////////
    bool operator==(const uint4_t &other) const { return (uint8_t)(*this) == (uint8_t)other; }

    bool operator==(const uint8_t &other) const { return (uint8_t)(*this) == other; }

    bool operator==(const uint32_t &other) const { return (uint32_t)(*this) == other; }

    bool operator==(const int &other) const { return (uint8_t)(*this) == (uint8_t)other; }

    bool operator<(const uint4_t &other) const { return (uint8_t)(*this) < (uint8_t)other; }

    bool operator<(const uint8_t &other) const { return (uint8_t)(*this) < other; }

    bool operator<(const uint32_t &other) const { return (uint32_t)(*this) < other; }

    bool operator<=(const uint4_t &other) const { return (uint8_t)(*this) <= (uint8_t)other; }

    bool operator<=(const uint8_t &other) const { return (uint8_t)(*this) <= other; }

    bool operator<=(const uint32_t &other) const { return (uint32_t)(*this) <= other; }

    bool operator>(const uint4_t &other) const { return (uint8_t)(*this) > (uint8_t)other; }

    bool operator>(const uint8_t &other) const { return (uint8_t)(*this) > other; }

    bool operator>(const uint32_t &other) const { return (uint32_t)(*this) > other; }

    bool operator>=(const uint4_t &other) const { return (uint8_t)(*this) >= (uint8_t)other; }

    bool operator>=(const uint8_t &other) const { return (uint8_t)(*this) >= other; }

    bool operator>=(const uint32_t &other) const { return (uint32_t)(*this) >= other; }

    ////////////////////////////////////////////////////////////
    // Binary operators
    ////////////////////////////////////////////////////////////
    uint4_t operator+(uint4_t &other) { return uint4_t((uint8_t)(*this) + (uint8_t)other, false); }

    uint4_t operator^(uint4_t &other) { return uint4_t((uint8_t)(*this) ^ (uint8_t)other, false); }

    uint8_t operator^(uint8_t &other) { return uint8_t((uint8_t)(*this) ^ other); }

    // uint4_t& operator^=(uint4_t& other) {
    //   *this = uint8_t((uint8_t)(*this) ^ (uint8_t)other);
    //   return *this;
    // }

    // uint4_t& operator^=(uint8_t& other) {
    //   *this = uint8_t((uint8_t)(*this) ^ other);
    //   return *this;
    // }

    uint4_t operator&(uint4_t &other) { return uint4_t((uint8_t)(*this) & (uint8_t)other, false); }

    uint8_t operator&(uint8_t &other) { return uint8_t((uint8_t)(*this) & other); }

    uint4_t operator<<(const int shift) { return uint4_t((uint8_t)(*this) << shift, false); }

    uint4_t operator>>(const int shift) { return uint4_t((uint8_t)(*this) >> shift, false); }

    // uint4_t& operator<<=(const int shift) {
    //   *this = uint4_t((uint8_t)(*this) << shift, false);
    //   return *this;
    // }

    // uint4_t& operator>>=(const int shift) {
    //   *this = uint4_t((uint8_t)(*this) >> shift, false);
    //   return *this;
    // }

    ////////////////////////////////////////////////////////////
    // Typecast operators
    ////////////////////////////////////////////////////////////
    operator uint8_t() const {
        uint8_t out = value[0] | value[1] << 1 | value[2] << 2 | value[3] << 3;
        return out;
    }

    operator uint32_t() {
        uint32_t out = value[0] | value[1] << 1 | value[2] << 2 | value[3] << 3;
        return out;
    }

    ~uint4_t() { value.clear(); }
};

uint8_t gmul(uint8_t a, uint8_t b);

uint4_t gmul(uint4_t a, uint4_t b);

template <typename TYPE> void PrintState(const std::vector<TYPE> &state, bool isString = false) {

    if (isString) {
        for (int i = 0; i < state.size(); ++i)
            PRINTHEX(state[i]);
        return;
    }
    if (state.size() == 16) {
        for (int i = 0; i < 4; ++i) {
            PRINTHEX(state[i]);
            PRINTHEX(state[i + 4]);
            PRINTHEX(state[i + 8]);
            PRINTHEX(state[i + 12]);
            std::cout << std::endl;
        }
        std::cout << "==================";
    } else if (state.size() == 9) {
        for (int i = 0; i < 3; ++i) {
            PRINTHEX(state[i]);
            PRINTHEX(state[i + 3]);
            PRINTHEX(state[i + 6]);
            std::cout << std::endl;
        }
        std::cout << "==========\n";
    } else if (state.size() == 3) {
        PRINTHEX(state[0]);
        PRINTHEX(state[1]);
        PRINTHEX(state[2]);
        std::cout << "\n==========\n";
    }
}

#endif // HELPER_H
