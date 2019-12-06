#include <gtest/gtest.h>

#include <helper.h>

uint4_t gmul(uint4_t a, uint4_t b) {
    uint4_t p = 0;
    uint8_t counter;
    uint4_t hi_bit_set;
    for (counter = 0; counter < 4; counter++) {
        if (b & 1)
            p = p ^ a;
        hi_bit_set = (a & 0x08);
        a = a << 1;
        if (hi_bit_set)
            a = a ^ 0x13; /* x^4 + x + 1 */
        b = b >> 1;
    }
    return p;
}

uint8_t gmul(uint8_t a, uint8_t b) {
    uint8_t p = 0;
    uint8_t counter;
    uint8_t hi_bit_set;
    for (counter = 0; counter < 8; counter++) {
        if (b & 1)
            p ^= a;
        hi_bit_set = (a & 0x80);
        a <<= 1;
        if (hi_bit_set)
            a ^= 0x1b; /* x^8 + x^4 + x^3 + x + 1 */
        b >>= 1;
    }
    return p;
}

//////////////////////////////////////////////////////////////////////////
// Test cases uint4_t
//////////////////////////////////////////////////////////////////////////

TEST(uint4_t, Positive_Constructor) {
    for (int i = 0; i < 256; ++i) {
        uint8_t value = i;
        uint4_t bigPart(value, true);
        EXPECT_EQ(value & 0xF0, (uint8_t)bigPart << 4);

        uint4_t littlePart(value);
        EXPECT_EQ(value & 0x0F, (uint8_t)littlePart);
    }
}

TEST(uint4_t, Positive_BinaryOperator_Summary) {
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            uint4_t first(i, false);
            uint4_t second(j, false);

            EXPECT_EQ((i + j) & 0x0F, (uint8_t)(first + second));
            EXPECT_EQ(i & 0x0F, (uint8_t)first);
            EXPECT_EQ(j & 0x0F, (uint8_t)second);
        }
    }
}

TEST(uint4_t, Positive_BinaryOperator_AND) {
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            uint4_t first(i, false);
            uint4_t second(j, false);

            EXPECT_EQ((i & j) & 0x0F, (uint8_t)(first & second));

            EXPECT_EQ(i & 0x0F, (uint8_t)first);
            EXPECT_EQ(j & 0x0F, (uint8_t)second);
        }
    }

    for (int i = 16; i < 256; ++i) {
        for (int j = 16; j < 256; ++j) {
            uint4_t first(i, false);
            uint4_t second(j, false);

            EXPECT_EQ(i & (j & 0x0F), i & second);
            EXPECT_EQ((i & 0x0F) & j, first & j);

            EXPECT_EQ(i & 0x0F, (uint8_t)first);
            EXPECT_EQ(j & 0x0F, (uint8_t)second);
        }
    }
}

TEST(uint4_t, Positive_BinaryOperator_XOR) {
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            uint4_t first(i, false);
            uint4_t second(j, false);

            EXPECT_EQ((i ^ j) & 0x0F, (uint8_t)(first ^ second));

            EXPECT_EQ(i & 0x0F, (uint8_t)first);
            EXPECT_EQ(j & 0x0F, (uint8_t)second);
        }
    }

    for (int i = 16; i < 256; ++i) {
        for (int j = 16; j < 256; ++j) {
            uint4_t first(i, false);
            uint4_t second(j, false);

            EXPECT_EQ(i ^ (j & 0x0F), i ^ second);
            EXPECT_EQ((i & 0x0F) ^ j, first ^ j);

            EXPECT_EQ(i & 0x0F, (uint8_t)first);
            EXPECT_EQ(j & 0x0F, (uint8_t)second);
        }
    }
}

TEST(uint4_t, Positive_Operator_RightShift) {
    for (int i = 0; i < 16; ++i) {
        for (int shift = 0; shift < 5; ++shift) {
            uint4_t result(i, false);

            EXPECT_EQ((i >> shift) & 0x0F, (uint8_t)(result >> shift));
            EXPECT_EQ(i & 0x0F, (uint8_t)result);
        }
    }
}

TEST(uint4_t, Positive_Operator_LeftShift) {
    for (int i = 0; i < 16; ++i) {
        for (int shift = 0; shift < 5; ++shift) {
            uint4_t result(i, false);

            EXPECT_EQ((i << shift) & 0x0F, (uint8_t)(result << shift));
            EXPECT_EQ(i & 0x0F, (uint8_t)result);
        }
    }
}

// TEST(uint4_t, GF_16_MatrixMul) {
//   printf("    ");
//   for (int i = 0; i < 16; ++i)
//     PRINTHEX(uint4_t(i));
//   printf("\n");
//   for (int i = 0; i < 16; ++i) {
//     PRINTHEX(uint4_t(i));
//     for (int j = 0; j < 16; ++j) {
//       uint4_t first(i, false);
//       uint4_t second(j, false);
//       uint4_t result = gmul(first, second);
//       PRINTHEX(result);
//     }
//     printf("\n");
//   }
// }

// TEST(uint4_t, GF_16_Sbox) {
//     uint8_t mulMatrix[4] = {0x02, 0x0A, 0x0C, 0x05};

//     std::vector<uint4_t> table;
//     std::cout << std::left << std::setfill(' ') << std::setw(20);
//     std::cout << "Z:";
//     for (int i = 0; i < 16; ++i)
//         PRINTHEX(uint4_t(i));
//     printf("\n");
//     std::cout << std::left << std::setfill(' ') << std::setw(20);
//     std::cout << "Z^(-1)";
//     printf("0 | ");
//     table.push_back(0x00);
//     for (int i = 0; i < 16; ++i) {
//         for (int j = 0; j < 16; ++j) {
//             uint4_t first(i, false);
//             uint4_t second(j, false);
//             uint4_t result = gmul(first, second);
//             if (result == 0x01) {
//                 table.push_back(second);
//                 PRINTHEX(second);
//                 break;
//             }
//         }
//     }
//     printf("\n");

//     std::vector<uint4_t> mul;
//     uint4_t tmp;
//     for (int j = 0; j < 16; ++j) {
//         uint8_t element = 0;
//         for (int i = 0; i < 4; ++i) {
//             tmp = table[j] & mulMatrix[i];
//             bool bit = (tmp & 1) ^ ((tmp >> 1) & 1) ^ ((tmp >> 2) & 1) ^ ((tmp >> 3) & 1);
//             element = (element << 1) | bit;
//         }
//         tmp = uint4_t(element);
//         mul.push_back(tmp);
//     }
//     std::cout << std::left << std::setfill(' ') << std::setw(20);
//     std::cout << "(...)*Z^(-1)";
//     for (int i = 0; i < 16; ++i)
//         PRINTHEX(mul[i]);
//     printf("\n");

//     std::cout << std::left << std::setfill(' ') << std::setw(20);
//     std::cout << "(...)*Z^(-1) ^ C";
//     for (int i = 0; i < 16; ++i) {
//         uint4_t x = mul[i] ^ uint4_t(0x07);
//         PRINTHEX(x);
//     }
//     printf("\n");
// }

// TEST(uint8_t, MixColumnsPolynoms_Invers) {
//    uint8_t matrix[] = {0x02, 0x01, 0x01, 0x03};
//    uint8_t B[] = {0x0E, 0x09, 0x0D, 0x0B};

//    uint8_t result =
//        gmul(matrix[0], B[0]) ^ gmul(matrix[3], B[1]) ^ gmul(matrix[2], B[2]) ^ gmul(matrix[1], B[3]);

//    PRINTHEX(result);
//    result =
//        gmul(matrix[1], B[0]) ^ gmul(matrix[0], B[1]) ^ gmul(matrix[3], B[2]) ^ gmul(matrix[2], B[3]);

//    PRINTHEX(result);
//    result =
//        gmul(matrix[2], B[0]) ^ gmul(matrix[1], B[1]) ^ gmul(matrix[0], B[2]) ^ gmul(matrix[3], B[3]);

//    PRINTHEX(result);
//    result =
//        gmul(matrix[3], B[0]) ^ gmul(matrix[2], B[1]) ^ gmul(matrix[1], B[2]) ^ gmul(matrix[0], B[3]);

//    PRINTHEX(result);
//    printf("\n");

//    uint8_t matrixHalfByte[] = {0x03, 0x01, 0x07};
//    for (uint8_t i = 0; i < 16; ++i)
//        for (uint8_t j = 0; j < 16; ++j)
//            for (uint8_t k = 0; k < 16; ++k) {
//                uint8_t first = gmul(uint4_t(matrixHalfByte[1]), uint4_t(k)) ^
//                                gmul(uint4_t(matrixHalfByte[0]), uint4_t(i)) ^
//                                gmul(uint4_t(matrixHalfByte[2]), uint4_t(j));

//                uint8_t second = gmul(uint4_t(matrixHalfByte[1]), uint4_t(i)) ^
//                                 gmul(uint4_t(matrixHalfByte[0]), uint4_t(j)) ^
//                                 gmul(uint4_t(matrixHalfByte[2]), uint4_t(k));

//                uint8_t third = gmul(uint4_t(matrixHalfByte[1]), uint4_t(j)) ^
//                                gmul(uint4_t(matrixHalfByte[2]), uint4_t(i)) ^
//                                gmul(uint4_t(matrixHalfByte[0]), uint4_t(k));

//                if (first == 1 && second == 0 && third == 0) {
//                    PRINTHEX(i);
//                    PRINTHEX(j);
//                    PRINTHEX(k);
//                }
//            }

//    printf("\n");

//    printf("--------------------");
//    printf("\n");
//    uint8_t x = gmul(uint4_t(0x07), uint4_t(0x08)) ^ gmul(uint4_t(0x0D), uint4_t(0x01)) ^
//    gmul(uint4_t(0x03), uint4_t(0x0E)); PRINTHEX(x); x = gmul(uint4_t(0x01), uint4_t(0x0E)) ^
//    gmul(uint4_t(0x08), uint4_t(0x03)) ^ gmul(uint4_t(0x07), uint4_t(0x0D)); PRINTHEX(x); x =
//    gmul(uint4_t(0x01), uint4_t(0x08)) ^ gmul(uint4_t(0x07), uint4_t(0x0E)) ^ gmul(uint4_t(0x0D),
//    uint4_t(0x03)); PRINTHEX(x); printf("\n");
//}
