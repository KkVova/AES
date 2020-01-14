#include <gtest/gtest.h>

#include "diffway.h"

#include <Timer/timer.h>
#include <aes/precompute.h>
#include <iostream>

using std::vector;

namespace {
uint8_t S_Box[16] = {0x07, 0x06, 0x00, 0x0E,  //
                     0x03, 0x0C, 0x09, 0x08,  //
                     0x0F, 0x0B, 0x02, 0x05,  //
                     0x0D, 0x04, 0x0A, 0x01}; //
}

DiffWay::DiffWay() {}

void DiffWay::directDifferential(std::map<uint16_t, std::vector<std::vector<uint16_t>>> &all_W2_dW3) {
    Timer timer("DiffWay_Direct   ");

    for (uint8_t dY1_0 = 0x01; dY1_0 < 0x10; ++dY1_0) {

        uint16_t dY1;
        uint16_t dZ1;
        uint16_t dW2;

        dY1 = ((uint16_t)dY1_0 << 8);
        // dY1 -> MixColumns() -> dZ1
        dZ1 = precompute_MixColumns[dY1];
        // dZ1 == dW2
        dW2 = dZ1;

        for (uint16_t W2 = 0x0000; W2 < 0x1000; ++W2) {
            uint16_t W2_;
            uint16_t X2;
            uint16_t X2_;
            uint16_t dX2;
            vector<uint16_t> dY2;
            vector<uint16_t> dW3;

            // for all possible W2 => W2_ = W2 ^ dW2
            W2_ = W2 ^ dW2;

            // dX2 -> SubBytes(W2) ^ SubBytes(W2_) -> dX2
            X2 = precompute_SubBytes[W2];
            X2_ = precompute_SubBytes[W2_];
            dX2 = X2 ^ X2_;

            // dX2 -> ShiftRows() -> dY2
            // dY2 -> MixColumns() -> dZ2
            // dZ2 = dW3
            dW3.push_back(precompute_MixColumns[dX2 & 0xF00]);
            dW3.push_back(precompute_MixColumns[dX2 & 0x00F]);
            dW3.push_back(precompute_MixColumns[dX2 & 0x0F0]);

            if (all_W2_dW3[W2].size() == 0)
                all_W2_dW3[W2] = vector<vector<uint16_t>>(1, dW3);
            else
                all_W2_dW3[W2].push_back(dW3);
        }
    }
}

void DiffWay::reverseDifferential(std::map<uint16_t, std::vector<std::vector<uint16_t>>> &all_Y4_dX3) {
    Timer timer("DiffWay_Reverse  ");

    for (uint8_t dZ4_0 = 0x01; dZ4_0 < 0x10; ++dZ4_0) {

        uint16_t dZ4;
        uint16_t dY4;

        dZ4 = ((uint16_t)dZ4_0 << 8);
        // dZ4 -> InvMixColumns() -> dY4
        dY4 = precompute_InvMixColumns[dZ4];

        for (uint16_t Y4 = 0x0000; Y4 < 0x1000; ++Y4) {
            uint16_t Y4_;
            uint16_t X4;
            uint16_t X4_;
            vector<uint16_t> dZ3;
            vector<uint16_t> dY3;
            vector<uint16_t> dX3;

            // for all possible Y4 => Y4_ = Y4 ^ dY4
            Y4_ = Y4 ^ dY4;

            // dY4 -> InvSubBytes(Y4) ^ SubBytes(Y4_) -> InvShiftRows -> dW4 = dZ3
            // bcs (Inv)ShiftRow and (Inv)SubBytes is independent operations
            X4 = precompute_InvSubBytes[Y4];
            X4_ = precompute_InvSubBytes[Y4_];
            uint16_t tmp = X4 ^ X4_;

            dZ3 = AES<uint4_t>::InvShiftRows(tmp);

            // dZ3 -> InvMixColumns() -> dY3
            dY3.push_back(precompute_InvMixColumns[dZ3[0]]);
            dY3.push_back(precompute_InvMixColumns[dZ3[1]]);
            dY3.push_back(precompute_InvMixColumns[dZ3[2]]);

            // dY3 -> MixColumns -> dX3
            dX3 = dY3;
            AES<uint4_t>::InvShiftRows(dX3);

            if (all_Y4_dX3[Y4].size() == 0)
                all_Y4_dX3[Y4] = vector<vector<uint16_t>>(1, dX3);
            else
                all_Y4_dX3[Y4].push_back(dX3);
        }
    }
}

void DiffWay::checkMiddle(const std::map<uint16_t, std::vector<std::vector<uint16_t>>> &all_W2_dW3,
                          const std::map<uint16_t, std::vector<std::vector<uint16_t>>> &all_Y4_dX3,
                          vector<std::tuple<uint16_t, uint64_t, uint16_t>> &ALL_W2_W3_Y4) {
    Timer timer("CheckMiddle      ");
    vector<uint64_t> tmpl;

    uint32_t W2_dW3sCounter = 0;
    uint32_t Y4_dX3sCounter = 0;

    uint32_t counter = 0;
    for (auto W2_dW3s = all_W2_dW3.begin(); /* W2_dW3s != all_W2_dW3.end()*/ W2_dW3sCounter < 64;
         ++W2_dW3s, ++W2_dW3sCounter) {

        for (auto dWit = (*W2_dW3s).second.begin(); dWit != (*W2_dW3s).second.end(); ++dWit) {
            Y4_dX3sCounter = 0;
            // printf("%d\n", counter++);
            for (auto Y4_dX3s = all_Y4_dX3.begin(); /*Y4_dX3s != all_Y4_dX3.end()*/ Y4_dX3sCounter < 64;
                 ++Y4_dX3s, ++Y4_dX3sCounter) {

                for (auto dXit = (*Y4_dX3s).second.begin(); dXit != (*Y4_dX3s).second.end(); ++dXit) {
                    vector<vector<uint64_t>> W_table(9, tmpl);
                    for (int i = 0; i < 9; i++) {
                        uint8_t dWi = ((*dWit)[i / 3] >> (((i % 3) * (-4)) + 8)) & 0x0F;
                        uint8_t dXi = ((*dXit)[i / 3] >> (((i % 3) * (-4)) + 8)) & 0x0F;

                        if (SboxDiff[dWi][dXi].size() == 0) {
                            W_table.clear();
                            break;
                        }
                        for (int j = 0; j < SboxDiff[dWi][dXi].size(); ++j)
                            W_table[i].push_back(SboxDiff[dWi][dXi][j]);
                    }

                    if (W_table.empty())
                        continue;

                    for (int a = 0; a < W_table[0].size(); ++a)
                        for (int b = 0; b < W_table[1].size(); ++b)
                            for (int c = 0; c < W_table[2].size(); ++c)
                                for (int d = 0; d < W_table[3].size(); ++d)
                                    for (int e = 0; e < W_table[4].size(); ++e)
                                        for (int f = 0; f < W_table[5].size(); ++f)
                                            for (int g = 0; g < W_table[6].size(); ++g)
                                                for (int h = 0; h < W_table[7].size(); ++h)
                                                    for (int i = 0; i < W_table[8].size(); ++i) {
                                                        uint64_t aggreagate =
                                                            (uint64_t)W_table[0][a] << 32 |
                                                            (uint64_t)W_table[1][b] << 28 |
                                                            (uint64_t)W_table[2][c] << 24 |
                                                            (uint64_t)W_table[3][d] << 20 |
                                                            (uint64_t)W_table[4][e] << 16 |
                                                            (uint64_t)W_table[5][f] << 12 |
                                                            (uint64_t)W_table[6][g] << 8 |
                                                            (uint64_t)W_table[7][h] << 4 |
                                                            (uint64_t)W_table[8][i];

                                                        ALL_W2_W3_Y4.push_back(
                                                            std::make_tuple((*W2_dW3s).first, aggreagate,
                                                                            (*Y4_dX3s).first));
                                                    }
                }
            }
        }
    }
}

void DiffWay::multidimensionalDiff(
    const std::vector<std::tuple<uint16_t, uint64_t, uint16_t>> &ALL_W2_W3_Y4,
    std::vector<Pair> &ALL_W5_0) {
    Timer timer("MultidimDiff     ");

    for (auto it = ALL_W2_W3_Y4.begin(); it != ALL_W2_W3_Y4.end(); ++it) {
        vector<uint8_t> sW5_0;
        uint64_t agg = 0;

        uint16_t W2;
        uint64_t W3;
        uint16_t Y4;
        std::tie(W2, W3, Y4) = *it;

        uint16_t X2 = precompute_SubBytes[W2];
        vector<uint16_t> X3;
        X3.push_back(precompute_SubBytes[(W3 >> 24) & 0x0FFF]);
        X3.push_back(precompute_SubBytes[(W3 >> 12) & 0x0FFF]);
        X3.push_back(precompute_SubBytes[(W3)&0x0FFF]);
        for (uint8_t sY1_0 = 1; sY1_0 < 0x10; ++sY1_0) {
            uint16_t sW2 = precompute_MixColumns[sY1_0];
            uint16_t sX2 = X2 ^ precompute_SubBytes[W2 ^ sW2];
            vector<uint16_t> sY2 = AES<uint8_t>::ShiftRows(sX2);
            vector<uint16_t> sW3;
            vector<uint16_t> sX3;
            vector<uint16_t> sY3;
            vector<uint16_t> sW4;
            uint16_t W4;
            uint16_t tmp_sW4;
            uint16_t sY4;
            uint16_t sW5;
            for (int i = 0; i < 3; i++)
                sW3.push_back(precompute_MixColumns[sY2[i]]);

            for (int i = 0; i < 3; i++)
                sX3.push_back(X3[i] ^
                              precompute_SubBytes[(uint16_t)((W3 >> (24 - i * 12)) & 0x0FFF) ^ sW3[i]]);

            sY3 = sX3;

            AES<uint8_t>::ShiftRows(sY3);
            for (int i = 0; i < 3; i++)
                sW4.push_back(precompute_MixColumns[sY3[i]]);

            W4 = precompute_InvSubBytes[Y4];
            tmp_sW4 = (sW4[0] & 0x0F00) | (sW4[1] & 0x00F0) | (sW4[2] & 0x000F);

            sY4 = Y4 ^ precompute_SubBytes[W4 ^ tmp_sW4];

            sW5 = precompute_MixColumns[sY4];
            sW5_0.push_back(sW5);
        }

        for (int i = 0; i < 15; ++i) {
            agg = (agg << 4 * i) | ((sW5_0[i] >> 8) & 0x000F);
        }
        ALL_W5_0.push_back(MSET::makePair(agg));
    }
}

void DiffWay::onlinePhase() {
    vector<std::tuple<uint8_t, uint8_t, uint16_t>> all_W5_dW5_dW6;
    for (uint8_t dW5_0 = 0x00; dW5_0 < 0x10; dW5_0++)
        for (uint8_t W5_0 = 0x00; W5_0 < 0x10; W5_0++) {

            uint16_t dY5;
            uint16_t dZ5;
            uint16_t dW6;

            dY5 = S_Box[W5_0] ^ S_Box[(W5_0 ^ dW5_0)];
            // 00* -> *00
            dY5 = dY5 << 8;

            dW6 = precompute_MixColumns[dY5];
            all_W5_dW5_dW6.push_back(std::make_tuple(W5_0, dW5_0, dW6));
        }
}

void DiffWay::precomputeSboxDiff() {

    Timer timer("PreSboxDiff      ");
    for (uint8_t dW = 0x00; dW < 0x10; ++dW) {
        vector<vector<uint8_t>> dWi;
        for (uint8_t dX = 0x00; dX < 0x10; ++dX) {
            vector<uint8_t> dXi = vector<uint8_t>();
            for (uint8_t W = 0x00; W < 0x10; ++W) {
                if ((S_Box[W] ^ S_Box[W ^ dW]) == dX)
                    dXi.push_back(W);
            }
            dWi.push_back(dXi);
        }
        SboxDiff.push_back(dWi);
    }

    //  for (uint8_t i = 0; i < 0x10; i++) {
    // std::cout << std::hex << (uint32_t)i << "/";
    //  }
    //  printf("\n");

    //  for (int i = 0; i < SboxDiff.size(); i++) {
    //      for (int j = 0; j < SboxDiff[i].size(); j++) {
    //          std::cout << std::hex << SboxDiff[i][j].size() << "/";
    //      }
    //      printf("\n");
    //  }
}

TEST(DiffWay, multidimensionalDiff) {
    DiffWay dw;

    std::map<uint16_t, std::vector<std::vector<uint16_t>>> all_W2_dW3;
    std::map<uint16_t, std::vector<std::vector<uint16_t>>> all_Y4_dX3;
    vector<std::tuple<uint16_t, uint64_t, uint16_t>> ALL_W2_W3_Y4;
    std::vector<Pair> ALL_W5_0;

    dw.directDifferential(all_W2_dW3);
    dw.reverseDifferential(all_Y4_dX3);

    // uint32_t counter = 0;
    // for (auto vec_vec = all_W2_dW3.begin(); vec_vec != all_W2_dW3.end(); ++vec_vec) {
    //    counter += (*vec_vec).second.size();
    //    for (auto vec = (*vec_vec).second.begin(); vec != (*vec_vec).second.end(); ++vec) {
    //        std::cout << std::hex << std::setfill('0') << std::setw(4) << (*vec_vec).first << "////";
    //        for (int i = 0; i < 3; i++) {
    //            std::cout << " " << std::hex << std::setfill('0') << std::setw(4) << (*vec)[i];
    //        }
    //        std::cout << std::endl;
    //    }
    //}
    // printf("%d\n", counter);

    dw.precomputeSboxDiff();
    dw.checkMiddle(all_W2_dW3, all_Y4_dX3, ALL_W2_W3_Y4);
    printf("ALL - %d\n", ALL_W2_W3_Y4.size());

    dw.multidimensionalDiff(ALL_W2_W3_Y4, ALL_W5_0);
    std::cout << ALL_W5_0.size() << std::endl;

    // for (auto it = ALL_W2_W3_Y4.begin(); it != ALL_W2_W3_Y4.end(); ++it) {
    //    if (std::get<0>(*it) == 0x00 || std::get<1>(*it) == 0x00 || std::get<2>(*it) == 0x00)
    //        std::cout << std::hex << std::get<0>(*it) << " / " << std::get<1>(*it) << " / "
    //                  << std::get<2>(*it) << std::endl;
    //}
}
