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

void DiffWay::directDifferential(vector<vector<uint16_t>> &all_dW3) {
    Timer timer("DiffWay_Direct   ");
    static int counter = 0;
    vector<uint16_t> tail(3, 0x000);
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

            all_dW3[counter] = dW3;
            ++counter;
        }
    }
}

void DiffWay::reverseDifferential(std::vector<std::vector<uint16_t>> &all_dX3) {
    Timer timer("DiffWay_Reverse  ");
    static int counter = 0;
    vector<uint16_t> tail(3, 0x000);
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

            all_dX3[counter] = dX3;
            ++counter;
        }
    }
}

void DiffWay::checkMiddle(vector<vector<uint16_t>> &all_dW3, vector<vector<uint16_t>> &all_dX3) {
    Timer timer("CheckMiddle      ");

    vector<uint64_t> tmpl;
    vector<uint64_t> ALL_W;

    for (auto dWit = all_dW3.begin(); (dWit - all_dW3.begin()) < 1024; dWit++) {
        // for (auto dWit = all_dW3.begin(); dWit != all_dW3.end(); dWit++) {
        // std::cout << dWit - all_dW3.begin() << std::endl;
        for (auto dXit = all_dX3.begin(); (dXit - all_dX3.begin()) < 1024; dXit++) {
            // for (auto dXit = all_dX3.begin(); dXit != all_dX3.end(); dXit++) {

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
                                                uint64_t aggreagate = (uint64_t)W_table[0][a] << 32 |
                                                                      (uint64_t)W_table[1][b] << 28 |
                                                                      (uint64_t)W_table[2][c] << 24 |
                                                                      (uint64_t)W_table[3][d] << 20 |
                                                                      (uint64_t)W_table[4][e] << 16 |
                                                                      (uint64_t)W_table[5][f] << 12 |
                                                                      (uint64_t)W_table[6][g] << 8 |
                                                                      (uint64_t)W_table[7][h] << 4 |
                                                                      (uint64_t)W_table[8][i];

                                                ALL_W.push_back(aggreagate);
                                            }
        }
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
}

TEST(DiffWay, PrecomputeSboxDiff) {
    DiffWay dw;
    dw.precomputeSboxDiff();
}

TEST(DiffWay, CheckMiddle) {
    DiffWay dw;
    vector<uint16_t> templ(3, 0x0000);
    vector<vector<uint16_t>> all_dW3(61440, templ);
    vector<vector<uint16_t>> all_dX3(61440, templ);
    dw.directDifferential(all_dW3);
    dw.reverseDifferential(all_dX3);
    dw.precomputeSboxDiff();
    dw.checkMiddle(all_dW3, all_dX3);
}
