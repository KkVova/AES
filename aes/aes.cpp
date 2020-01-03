#include <gtest/gtest.h>

#include <cstdlib>
#include <iostream>
#include <typeinfo>

#include "aes.h"
#include "precompute.h"
#include <Timer/timer.h>
#include <helper/helper.h>

namespace {

// Encryption: Forward Rijndael S-box
uint8_t S_HalfByte[16] = {0x07, 0x06, 0x00, 0x0E,  //
                          0x03, 0x0C, 0x09, 0x08,  //
                          0x0F, 0x0B, 0x02, 0x05,  //
                          0x0D, 0x04, 0x0A, 0x01}; //

uint8_t inv_S_HalfByte[16] = {0x02, 0x0F, 0x0A, 0x04,  //
                              0x0D, 0x0B, 0x01, 0x00,  //
                              0x07, 0x06, 0x0E, 0x09,  //
                              0x05, 0x0C, 0x03, 0x08}; //

uint8_t S_Byte[256] = {
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16};

uint8_t inv_S_Byte[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d};

uint8_t rcon_Byte[10] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

uint8_t rcon_HalfByte[10] = {0x01, 0x02, 0x04, 0x08, 0x01, 0x03, 0x09, 0x0c, 0x0b, 0x06};

template <typename TYPE>
vector<TYPE> GetKeyState(vector<TYPE> &keySchedule, const int iteration, int blockSize) {
    return vector<TYPE>(keySchedule.begin() + (blockSize * iteration),
                        keySchedule.begin() + (blockSize * (iteration + 1)));
}
} // namespace

template <typename TYPE>
void AES<TYPE>::InitAES(vector<TYPE> &key, vector<TYPE> &keySchedule, bool isEncrypt,
                        const int &numberOfRounds) {
    vector<TYPE> currentKey;
    vector<TYPE> nextKey = key;
    keySchedule.insert(keySchedule.begin(), key.begin(), key.end());
    for (int i = 0; i < numberOfRounds; ++i) {
        currentKey = nextKey;
        KeySchedule(currentKey, nextKey, i);

        if (isEncrypt)
            keySchedule.insert(keySchedule.end(), nextKey.begin(), nextKey.end());
        else
            keySchedule.insert(keySchedule.begin(), nextKey.begin(), nextKey.end());
    }
}

template <typename TYPE>
void AES<TYPE>::KeySchedule(vector<TYPE> &prevKey, vector<TYPE> &nextKey, const uint8_t &roundNum) {
    // printf("KeySchedule\n");
    vector<TYPE> tmp = prevKey;

    if (prevKey.size() == 16) {
        tmp[0] = S_Byte[(uint8_t)prevKey[13]] ^ (uint8_t)prevKey[0] ^ rcon_Byte[roundNum];
        tmp[1] = S_Byte[(uint8_t)prevKey[14]] ^ (uint8_t)prevKey[1];
        tmp[2] = S_Byte[(uint8_t)prevKey[15]] ^ (uint8_t)prevKey[2];
        tmp[3] = S_Byte[(uint8_t)prevKey[12]] ^ (uint8_t)prevKey[3];

        for (int i = 4; i < 16; ++i)
            tmp[i] = tmp[i - 4] ^ prevKey[i];

    } else if (prevKey.size() == 9) {

        uint32_t aggregateState =
            ((uint32_t)prevKey[6] << 8) ^ ((uint32_t)prevKey[7] << 4) ^ (uint32_t)prevKey[8];
        aggregateState = precompute_SubBytes[aggregateState];

        tmp[0] = uint8_t(aggregateState >> 4) ^ (uint8_t)prevKey[0] ^ rcon_HalfByte[roundNum];
        tmp[1] = uint8_t(aggregateState) ^ (uint8_t)prevKey[1];
        tmp[2] = uint8_t(aggregateState >> 8) ^ (uint8_t)prevKey[2];

        for (int i = 3; i < 9; ++i)
            tmp[i] = tmp[i - 3] ^ prevKey[i];
    }
    nextKey = tmp;
    // printf("Current key state\n");
    // PrintState(nextKey);
}

template <typename TYPE>
void AES<TYPE>::Encrypt(const vector<TYPE> &data, vector<TYPE> &key, vector<TYPE> &encryptedMessage,
                        const int &numberOfRounds, bool fast) {
    isFast = fast;
    if ((data.size() != 9 && data.size() != 16) || (key.size() != 9 && key.size() != 16))
        assert("Block length is invalid");

    if (data.size() != key.size())
        assert("Length of data isn't equal length of key");

    int stateSize = data.size();
    vector<TYPE> keySchedule;
    vector<TYPE> currentKey;
    vector<TYPE> state = data;

    // printf("input key\n");
    // PrintState(key);
    InitAES(key, keySchedule, true, numberOfRounds);
    currentKey = GetKeyState(keySchedule, 0, stateSize);
    // PrintState(state);
    // printf("Current key state\n");
    // PrintState(currentKey);
    AddRoundKey(state, currentKey);
    for (int i = 1; i < numberOfRounds; i++) {
        currentKey = GetKeyState(keySchedule, i, stateSize);
        // printf("Round %d\n", i);
        Round(state, currentKey);
        // printf("Current key state\n");
        // PrintState(currentKey);
    }
    currentKey = GetKeyState(keySchedule, numberOfRounds, stateSize);
    // printf("Current key state\n");
    // PrintState(currentKey);
    FinalRound(state, currentKey);

    encryptedMessage = state;
    // printf("EncryptedMessage\n");
    // PrintState(encryptedMessage);
}

template <typename TYPE>
void AES<TYPE>::Decrypt(const vector<TYPE> &encryptedData, vector<TYPE> &key, vector<TYPE> &message,
                        const int &numberOfRounds, bool fast) {
    isFast = fast;
    if ((encryptedData.size() != 9 && encryptedData.size() != 16) ||
        (key.size() != 9 && key.size() != 16))
        assert("Block length is invalid");

    if (encryptedData.size() != key.size())
        assert("Length of data isn't equal length of key");

    int stateSize = encryptedData.size();
    vector<TYPE> keySchedule;
    vector<TYPE> currentKey;
    vector<TYPE> state = encryptedData;

    // printf("input key\n");
    // PrintState(key);
    InitAES(key, keySchedule, false, numberOfRounds);
    currentKey = GetKeyState(keySchedule, 0, stateSize);
    // PrintState(state);
    // printf("Current key state\n");
    // PrintState(currentKey);
    AddRoundKey(state, currentKey);
    for (int i = 1; i < numberOfRounds; i++) {
        currentKey = GetKeyState(keySchedule, i, stateSize);
        // printf("Round %d\n", i);
        InvRound(state, currentKey);
        // printf("Current key state\n");
        // PrintState(currentKey);
    }
    currentKey = GetKeyState(keySchedule, numberOfRounds, stateSize);
    // printf("Current key state\n");
    // PrintState(currentKey);
    InvFinalRound(state, currentKey);

    message = state;
    // printf("EncryptedMessage\n");
    // PrintState(encryptedMessage);
}

template <typename TYPE> void AES<TYPE>::Round(vector<TYPE> &state, vector<TYPE> &key) {
    SubBytes(state);
    ShiftRows(state);
    MixColumns(state);
    AddRoundKey(state, key);
}

template <typename TYPE> void AES<TYPE>::InvRound(vector<TYPE> &state, vector<TYPE> &key) {
    InvShiftRows(state);
    InvSubBytes(state);
    AddRoundKey(state, key);
    InvMixColumns(state);
}

template <typename TYPE> void AES<TYPE>::FinalRound(vector<TYPE> &state, vector<TYPE> &key) {
    SubBytes(state);
    ShiftRows(state);
    AddRoundKey(state, key);
}

template <typename TYPE> void AES<TYPE>::InvFinalRound(vector<TYPE> &state, vector<TYPE> &key) {
    InvShiftRows(state);
    InvSubBytes(state);
    AddRoundKey(state, key);
}

template <typename TYPE> void AES<TYPE>::AddRoundKey(vector<TYPE> &state, vector<TYPE> &roundKey) {
    // printf("AddRoundKey\n");
    for (int i = 0; i < state.size(); i++)
        state[i] = state[i] ^ roundKey[i];
    // PrintState(state);
}

template <typename TYPE> void AES<TYPE>::SubBytes(vector<TYPE> &state) {
    // printf("SubBytes\n");
    if (state.size() == 16) {
        for (int i = 0; i < state.size(); ++i)
            state[i] = S_Byte[(uint8_t)state[i]];
    }
    if (state.size() == 9) {
        if (isFast) {
            for (int i = 0; i < 9; i += 3) {
                uint16_t aggregateState =
                    ((uint16_t)state[i] << 8) ^ ((uint16_t)state[i + 1] << 4) ^ (uint16_t)state[i + 2];
                aggregateState = precompute_SubBytes[aggregateState];
                state[i] = uint8_t(aggregateState >> 8);
                state[i + 1] = uint8_t(aggregateState >> 4);
                state[i + 2] = uint8_t(aggregateState);
            }
            return;
        }
        for (int i = 0; i < state.size(); ++i)
            state[i] = S_HalfByte[(uint8_t)state[i]];
    }
    // PrintState(state);
}

template <typename TYPE> void AES<TYPE>::InvSubBytes(vector<TYPE> &state) {
    // printf("InvSubBytes\n");
    if (state.size() == 16) {
        for (int i = 0; i < state.size(); ++i)
            state[i] = inv_S_Byte[(uint8_t)state[i]];
    }
    if (state.size() == 9) {
        if (isFast) {
            for (int i = 0; i < 9; i += 3) {
                uint16_t aggregateState =
                    ((uint16_t)state[i] << 8) ^ ((uint16_t)state[i + 1] << 4) ^ (uint16_t)state[i + 2];
                aggregateState = precompute_InvSubBytes[aggregateState];
                state[i] = uint8_t(aggregateState >> 8);
                state[i + 1] = uint8_t(aggregateState >> 4);
                state[i + 2] = uint8_t(aggregateState);
            }
            return;
        }
        for (int i = 0; i < state.size(); ++i)
            state[i] = inv_S_HalfByte[(uint8_t)state[i]];
    }
    // PrintState(state);
}

template <typename TYPE> void AES<TYPE>::ShiftRows(vector<TYPE> &state) {
    // printf("ShiftRows\n");
    vector<TYPE> tmp = state;

    if (state.size() == 16) {
        tmp[0] = state[0];
        tmp[1] = state[5];
        tmp[2] = state[10];
        tmp[3] = state[15];
        tmp[4] = state[4];
        tmp[5] = state[9];
        tmp[6] = state[14];
        tmp[7] = state[3];
        tmp[8] = state[8];
        tmp[9] = state[13];
        tmp[10] = state[2];
        tmp[11] = state[7];
        tmp[12] = state[12];
        tmp[13] = state[1];
        tmp[14] = state[6];
        tmp[15] = state[11];
    } else if (state.size() == 9) {
        tmp[0] = state[0];
        tmp[1] = state[4];
        tmp[2] = state[8];
        tmp[3] = state[3];
        tmp[4] = state[7];
        tmp[5] = state[2];
        tmp[6] = state[6];
        tmp[7] = state[1];
        tmp[8] = state[5];
    }
    state = tmp;
    // PrintState(state);
}

template <typename TYPE> void AES<TYPE>::InvShiftRows(vector<TYPE> &state) {
    // printf("InvShiftRows\n");
    vector<TYPE> tmp = state;

    if (state.size() == 16) {
        tmp[0] = state[0];
        tmp[1] = state[13];
        tmp[2] = state[10];
        tmp[3] = state[7];
        tmp[4] = state[4];
        tmp[5] = state[1];
        tmp[6] = state[14];
        tmp[7] = state[11];
        tmp[8] = state[8];
        tmp[9] = state[5];
        tmp[10] = state[2];
        tmp[11] = state[15];
        tmp[12] = state[12];
        tmp[13] = state[9];
        tmp[14] = state[6];
        tmp[15] = state[3];
    } else if (state.size() == 9) {
        tmp[0] = state[0];
        tmp[1] = state[7];
        tmp[2] = state[5];
        tmp[3] = state[3];
        tmp[4] = state[1];
        tmp[5] = state[8];
        tmp[6] = state[6];
        tmp[7] = state[4];
        tmp[8] = state[2];
    }
    state = tmp;
    // PrintState(state);
}

template <typename TYPE> void AES<TYPE>::MixColumns(vector<TYPE> &state) {
    // printf("MixColumns\n");
    vector<TYPE> tmp = state;

    if (state.size() == 16) {
        A_param = 0x03;
        B_param = 0x01;
        C_param = 0x01;
        D_param = 0x02;
        for (int i = 0; i < 16; i += 4) {
            // use polynom 3x^3 + x^2 + x + 2
            tmp[i] = gmul(state[i], TYPE(D_param)) ^ gmul(state[i + 1], TYPE(A_param)) ^
                     gmul(state[i + 2], TYPE(B_param)) ^ gmul(state[i + 3], TYPE(C_param));

            tmp[i + 1] = gmul(state[i], TYPE(C_param)) ^ gmul(state[i + 1], TYPE(D_param)) ^
                         gmul(state[i + 2], TYPE(A_param)) ^ gmul(state[i + 3], TYPE(B_param));

            tmp[i + 2] = gmul(state[i], TYPE(B_param)) ^ gmul(state[i + 1], TYPE(C_param)) ^
                         gmul(state[i + 2], TYPE(D_param)) ^ gmul(state[i + 3], TYPE(A_param));

            tmp[i + 3] = gmul(state[i], TYPE(A_param)) ^ gmul(state[i + 1], TYPE(B_param)) ^
                         gmul(state[i + 2], TYPE(C_param)) ^ gmul(state[i + 3], TYPE(D_param));
        }
    } else if (state.size() == 9) {
        if (isFast) {
            for (int i = 0; i < 9; i += 3) {
                uint16_t aggregateState =
                    ((uint16_t)state[i] << 8) ^ ((uint16_t)state[i + 1] << 4) ^ (uint16_t)state[i + 2];
                aggregateState = precompute_MixColumns[aggregateState];
                state[i] = uint8_t(aggregateState >> 8);
                state[i + 1] = uint8_t(aggregateState >> 4);
                state[i + 2] = uint8_t(aggregateState);
            }
            return;
        }

        // use polynom 7x^2 + x + 3
        A_param = 0x07;
        B_param = 0x01;
        C_param = 0x03;
        for (int i = 0; i < 9; i += 3) {
            tmp[i] = gmul(state[i], TYPE(C_param)) ^ gmul(state[i + 1], TYPE(A_param)) ^
                     gmul(state[i + 2], TYPE(B_param));
            tmp[i + 1] = gmul(state[i], TYPE(B_param)) ^ gmul(state[i + 1], TYPE(C_param)) ^
                         gmul(state[i + 2], TYPE(A_param));

            tmp[i + 2] = gmul(state[i], TYPE(A_param)) ^ gmul(state[i + 1], TYPE(B_param)) ^
                         gmul(state[i + 2], TYPE(C_param));
        }
    }

    state = tmp;
    // PrintState(state);
}

template <typename TYPE> void AES<TYPE>::InvMixColumns(vector<TYPE> &state) {
    // printf("InvMixColumns\n");
    vector<TYPE> tmp = state;

    if (state.size() == 16) {
        // use polynom bx^3 + d^2 + 9x + e
        A_param = 0x0B;
        B_param = 0x0D;
        C_param = 0x09;
        D_param = 0x0E;
        for (int i = 0; i < 16; i += 4) {
            tmp[i] = gmul(state[i], TYPE(D_param)) ^ gmul(state[i + 1], TYPE(A_param)) ^
                     gmul(state[i + 2], TYPE(B_param)) ^ gmul(state[i + 3], TYPE(C_param));

            tmp[i + 1] = gmul(state[i], TYPE(C_param)) ^ gmul(state[i + 1], TYPE(D_param)) ^
                         gmul(state[i + 2], TYPE(A_param)) ^ gmul(state[i + 3], TYPE(B_param));

            tmp[i + 2] = gmul(state[i], TYPE(B_param)) ^ gmul(state[i + 1], TYPE(C_param)) ^
                         gmul(state[i + 2], TYPE(D_param)) ^ gmul(state[i + 3], TYPE(A_param));

            tmp[i + 3] = gmul(state[i], TYPE(A_param)) ^ gmul(state[i + 1], TYPE(B_param)) ^
                         gmul(state[i + 2], TYPE(C_param)) ^ gmul(state[i + 3], TYPE(D_param));
        }
    } else if (state.size() == 9) {
        if (isFast) {
            for (int i = 0; i < 9; i += 3) {
                uint16_t aggregateState =
                    ((uint16_t)state[i] << 8) ^ ((uint16_t)state[i + 1] << 4) ^ (uint16_t)state[i + 2];
                aggregateState = precompute_InvMixColumns[aggregateState];
                state[i] = uint8_t(aggregateState >> 8);
                state[i + 1] = uint8_t(aggregateState >> 4);
                state[i + 2] = uint8_t(aggregateState);
            }
            return;
        }

        // use polynom dx^2 + 8x + e
        A_param = 0x0D;
        B_param = 0x08;
        C_param = 0x0E;
        for (int i = 0; i < 9; i += 3) {
            tmp[i] = gmul(state[i], TYPE(C_param)) ^ gmul(state[i + 1], TYPE(A_param)) ^
                     gmul(state[i + 2], TYPE(B_param));
            tmp[i + 1] = gmul(state[i], TYPE(B_param)) ^ gmul(state[i + 1], TYPE(C_param)) ^
                         gmul(state[i + 2], TYPE(A_param));

            tmp[i + 2] = gmul(state[i], TYPE(A_param)) ^ gmul(state[i + 1], TYPE(B_param)) ^
                         gmul(state[i + 2], TYPE(C_param));
        }
    }

    state = tmp;
    // PrintState(state);
}

//////////////////////////////////////////////////////////////////////////
// Test cases AES
//////////////////////////////////////////////////////////////////////////

template <typename TYPE> bool AES<TYPE>::Test_Tetha_MixColumns(const int size) {
    //
    if (size == 9) {

        A_param = 0x07;
        B_param = 0x01;
        C_param = 0x03;
        vector<TYPE> tail(6, 0x00);
        for (uint32_t i = 0; i <= TYPE(0xFF); ++i)
            for (uint32_t j = 0; j <= TYPE(0xFF); ++j)
                for (uint32_t k = 0; k <= TYPE(0xFF); ++k) {
                    vector<TYPE> state;

                    state.push_back((uint8_t)i);
                    state.push_back((uint8_t)j);
                    state.push_back((uint8_t)k);
                    int teta = state.size() - std::count(state.begin(), state.end(), TYPE(0x00));
                    if (!teta)
                        continue;
                    state.insert(state.end(), tail.begin(), tail.end());

                    vector<TYPE> encryptedState = state;
                    teta += FindTeta(encryptedState);
                    if (teta < 4) {
                        // printf("teta = %d\n", teta);
                        // printf("state\n");
                        // PrintState(state);
                        // printf("encryptedState\n");
                        // PrintState(encryptedState);
                        return false;
                    }
                }
    } else if (size == 16) {

        A_param = 0x03;
        B_param = 0x01;
        C_param = 0x01;
        D_param = 0x02;
        vector<TYPE> tail(12, 0x00);
        for (uint32_t i = 0; i <= TYPE(0xFF); ++i)
            for (uint32_t j = 0; j <= TYPE(0xFF); ++j)
                for (uint32_t k = 0; k <= TYPE(0xFF); ++k)
                    for (uint32_t l = 0; l <= TYPE(0xFF); ++l) {
                        vector<TYPE> state;

                        state.push_back((uint8_t)i);
                        state.push_back((uint8_t)j);
                        state.push_back((uint8_t)k);
                        state.push_back((uint8_t)l);
                        int teta = state.size() - std::count(state.begin(), state.end(), TYPE(0x00));
                        if (!teta)
                            continue;
                        state.insert(state.end(), tail.begin(), tail.end());
                        vector<TYPE> encryptedState = state;
                        teta += FindTeta(state);
                        if (teta < 5) {
                            // printf("%d\n", teta);
                            // printf("state\n");
                            // PrintState(state);
                            // printf("encryptedState\n");
                            // PrintState(encryptedState);
                            return false;
                        }
                    }
    }
    return true;
}

template <typename TYPE> uint32_t AES<TYPE>::FindTeta(vector<TYPE> &state) {
    MixColumns(state);
    return (state.size() == 16 ? 4 : 3) -
           std::count(state.begin(), state.begin() + (state.size() == 16 ? 4 : 3), uint4_t(0x00));
}

template <typename TYPE> bool AES<TYPE>::Test_Inversion_MixColumns(const int size) {
    srand(time(0));
    vector<TYPE> state;
    vector<TYPE> startState;
    for (int i = 0; i < size; ++i) {
        state.push_back(rand());
    }

    startState = state;
    MixColumns(state);
    InvMixColumns(state);
    return state == startState;
}

template <typename TYPE> bool AES<TYPE>::Test_Inversion_ShiftRows(const int size) {
    srand(time(0));
    vector<TYPE> state;
    vector<TYPE> startState;
    for (int i = 0; i < size; ++i) {
        state.push_back(rand());
    }

    startState = state;
    ShiftRows(state);
    InvShiftRows(state);
    return state == startState;
}

template <typename TYPE> bool AES<TYPE>::Test_Inversion_Sbox(const int size) {
    srand(time(0));
    vector<TYPE> state;
    vector<TYPE> startState;
    for (int i = 0; i < size; ++i) {
        state.push_back(rand());
    }

    startState = state;
    SubBytes(state);
    InvSubBytes(state);
    return state == startState;
}

template <typename TYPE> bool AES<TYPE>::Test_Direct_And_Precompute_MixColumns(const int size) {
    srand(time(0));
    vector<TYPE> state;
    vector<TYPE> state_fast;
    vector<TYPE> startState;
    for (int i = 0; i < size; ++i) {
        startState.push_back(rand());
    }

    state = startState;
    state_fast = startState;

    isFast = false;
    MixColumns(state);
    isFast = true;
    MixColumns(state_fast);
    return state == state_fast;
}

template <typename TYPE> bool AES<TYPE>::Test_Direct_And_Precompute_InvMixColumns(const int size) {
    srand(time(0));
    vector<TYPE> state;
    vector<TYPE> state_fast;
    vector<TYPE> startState;
    for (int i = 0; i < size; ++i) {
        startState.push_back(rand());
    }

    state = startState;
    state_fast = startState;

    isFast = false;
    InvMixColumns(state);
    isFast = true;
    InvMixColumns(state_fast);
    return state == state_fast;
}

template <typename TYPE> bool AES<TYPE>::Test_Direct_And_Precompute_Sbox(const int size) {
    srand(time(0));
    vector<TYPE> state;
    vector<TYPE> state_fast;
    vector<TYPE> startState;
    for (int i = 0; i < size; ++i) {
        startState.push_back(rand());
    }

    state = startState;
    state_fast = startState;

    isFast = false;
    SubBytes(state);
    isFast = true;
    SubBytes(state_fast);
    return state == state_fast;
}

template <typename TYPE> bool AES<TYPE>::Test_Direct_And_Precompute_InvSbox(const int size) {
    srand(time(0));
    vector<TYPE> state;
    vector<TYPE> state_fast;
    vector<TYPE> startState;
    for (int i = 0; i < size; ++i) {
        startState.push_back(rand());
    }

    state = startState;
    state_fast = startState;

    isFast = false;
    InvSubBytes(state);
    isFast = true;
    InvSubBytes(state_fast);
    return state == state_fast;
}

template <typename TYPE> bool AES<TYPE>::Test_Uint16_ShiftRows() {
    AES<uint4_t> aes;
    vector<uint4_t> testResult;
    vector<uint4_t> state;
    vector<uint4_t> shiftState;
    vector<uint16_t> state_uint16;
    vector<uint16_t> shiftState_uint16;
    for (uint16_t i; i < 0x1000; ++i) {
        state.push_back((uint8_t)(i >> 8));
        state.push_back((uint8_t)(i >> 4));
        state.push_back((uint8_t)i);

        state_uint16.push_back(i);
        for (uint16_t j; j < 0x1000; ++j) {
            state.push_back((uint8_t)(j >> 8));
            state.push_back((uint8_t)(j >> 4));
            state.push_back((uint8_t)j);

            state_uint16.push_back(j);
            for (uint16_t k; i < 0x1000; ++k) {
                state.push_back((uint8_t)(k >> 8));
                state.push_back((uint8_t)(k >> 4));
                state.push_back((uint8_t)k);

                state_uint16.push_back(k);

                //---------------------------------------
                shiftState = state;
                aes.ShiftRows(shiftState);

                //---------------------------------------
                shiftState_uint16 = state_uint16;
                AES<uint4_t>::ShiftRows(shiftState_uint16);
                //
                for (int count = 0; count < 2; ++count) {
                    testResult.push_back((uint8_t)(shiftState_uint16[count] >> 8));
                    testResult.push_back((uint8_t)(shiftState_uint16[count] >> 4));
                    testResult.push_back((uint8_t)shiftState_uint16[count]);
                }

                if (testResult != shiftState)
                    return false;

                testResult.clear();

                state_uint16 = shiftState_uint16;
                AES<uint4_t>::InvShiftRows(state_uint16);
                //
                for (int count = 0; count < 2; ++count) {
                    testResult.push_back((uint8_t)(state_uint16[count] >> 8));
                    testResult.push_back((uint8_t)(state_uint16[count] >> 4));
                    testResult.push_back((uint8_t)state_uint16[count]);
                }

                if (testResult != shiftState)
                    return false;
            }
        }
    }

    for (uint16_t i = 0x0000; i < 0x1000; ++i) {
        testResult.clear();
        shiftState_uint16.clear();
        state = vector<uint4_t>(6, 0x00);

        state.insert(state.begin(), (uint8_t)i);
        state.insert(state.begin(), (uint8_t)(i >> 4));
        state.insert(state.begin(), (uint8_t)(i >> 8));
        aes.InvShiftRows(state);

        shiftState_uint16 = AES<uint4_t>::InvShiftRows(i);

        for (int count = 0; count < 3; ++count) {
            testResult.push_back((uint8_t)(shiftState_uint16[count] >> 8));
            testResult.push_back((uint8_t)(shiftState_uint16[count] >> 4));
            testResult.push_back((uint8_t)shiftState_uint16[count]);
        }
        if (testResult != state)
            return false;
    }
    return true;
}

template <typename TYPE> bool AES<TYPE>::Test_Differential(const int size) {
    srand(time(0));
    {
        vector<TYPE> first;
        vector<TYPE> second;
        vector<TYPE> diff;
        vector<TYPE> diff_check;
        for (int i = 0; i < size; ++i) {
            first.push_back(rand());
        }
        for (int i = 0; i < size; ++i) {
            diff.push_back(rand());
        }
        for (int i = 0; i < size; ++i) {
            second.push_back(first[i] ^ diff[i]);
        }

        MixColumns(first);
        MixColumns(second);
        MixColumns(diff);
        for (int i = 0; i < size; ++i) {
            diff_check.push_back(first[i] ^ second[i]);
        }
        if (diff != diff_check)
            return false;
    }
    {
        vector<TYPE> first;
        vector<TYPE> second;
        vector<TYPE> diff;
        vector<TYPE> diff_check;
        for (int i = 0; i < size; ++i) {
            first.push_back(rand());
        }
        for (int i = 0; i < size; ++i) {
            diff.push_back(rand());
        }
        for (int i = 0; i < size; ++i) {
            second.push_back(first[i] ^ diff[i]);
        }

        InvMixColumns(first);
        InvMixColumns(second);
        InvMixColumns(diff);
        for (int i = 0; i < size; ++i) {
            diff_check.push_back(first[i] ^ second[i]);
        }
        if (diff != diff_check)
            return false;
    }
    return true;
}

template <typename TYPE> void AES<TYPE>::Precompute_MixColumns_And_InvMixColumns(const int size) {
    vector<TYPE> tail(6, 0x00);
    // 3x3
    for (int i = 0; i < 16; ++i)
        for (int j = 0; j < 16; ++j)
            for (int k = 0; k < 16; ++k) {
                vector<TYPE> state;
                state.push_back((uint8_t)i);
                state.push_back((uint8_t)j);
                state.push_back((uint8_t)k);
                state.insert(state.end(), tail.begin(), tail.end());
                uint32_t aggregateState =
                    ((uint32_t)state[0] << 8) ^ ((uint32_t)state[1] << 4) ^ (uint32_t)state[2];
                //                PrintState(state);
                //                PRINTHEX(aggregateState);
                //                printf("\n");

                MixColumns(state);
                //                PrintState(state);
                uint32_t aggregateResult =
                    ((uint32_t)state[0] << 8) ^ ((uint32_t)state[1] << 4) ^ (uint32_t)state[2];
                if (aggregateState % 8 == 0)
                    printf("\n");
                std::cout << "0x" << std::hex << std::setfill('0') << std::setw(8) << aggregateResult
                          << ", ";
                //                PRINTHEX(aggregateResult);
                //                printf("\n=========================\n");
            }
    printf("\nInMixColumns=====================\n");
    for (int i = 0; i < 16; ++i)
        for (int j = 0; j < 16; ++j)
            for (int k = 0; k < 16; ++k) {
                vector<TYPE> state;
                state.push_back((uint8_t)i);
                state.push_back((uint8_t)j);
                state.push_back((uint8_t)k);
                state.insert(state.end(), tail.begin(), tail.end());
                uint32_t aggregateState =
                    ((uint32_t)state[0] << 8) ^ ((uint32_t)state[1] << 4) ^ (uint32_t)state[2];
                //                PrintState(state);
                //                PRINTHEX(aggregateState);
                //                printf("\n");

                InvMixColumns(state);
                //                PrintState(state);
                uint32_t aggregateResult =
                    ((uint32_t)state[0] << 8) ^ ((uint32_t)state[1] << 4) ^ (uint32_t)state[2];
                if (aggregateState % 8 == 0)
                    printf("\n");
                std::cout << "0x" << std::hex << std::setfill('0') << std::setw(8) << aggregateResult
                          << ", ";
                //                PRINTHEX(aggregateResult);
                //                printf("\n=========================\n");
            }
    printf("\n");
}

template <typename TYPE> void AES<TYPE>::Precompute_Sbox_And_InvSbox(const int size) {
    vector<TYPE> tail(6, 0x00);
    // 3x3
    for (int i = 0; i < 16; ++i)
        for (int j = 0; j < 16; ++j)
            for (int k = 0; k < 16; ++k) {
                vector<TYPE> state;
                state.push_back((uint8_t)i);
                state.push_back((uint8_t)j);
                state.push_back((uint8_t)k);
                state.insert(state.end(), tail.begin(), tail.end());
                uint32_t aggregateState =
                    ((uint32_t)state[0] << 8) ^ ((uint32_t)state[1] << 4) ^ (uint32_t)state[2];
                //                PrintState(state);
                //                PRINTHEX(aggregateState);
                //                printf("\n");

                SubBytes(state);
                //                PrintState(state);
                uint32_t aggregateResult =
                    ((uint32_t)state[0] << 8) ^ ((uint32_t)state[1] << 4) ^ (uint32_t)state[2];
                if (aggregateState % 8 == 0)
                    printf("\n");
                std::cout << "0x" << std::hex << std::setfill('0') << std::setw(8) << aggregateResult
                          << ", ";
                //                PRINTHEX(aggregateResult);
                //                printf("\n=========================\n");
            }
    printf("\nInvSubBytes=====================\n");
    for (int i = 0; i < 16; ++i)
        for (int j = 0; j < 16; ++j)
            for (int k = 0; k < 16; ++k) {
                vector<TYPE> state;
                state.push_back((uint8_t)i);
                state.push_back((uint8_t)j);
                state.push_back((uint8_t)k);
                state.insert(state.end(), tail.begin(), tail.end());
                uint32_t aggregateState =
                    ((uint32_t)state[0] << 8) ^ ((uint32_t)state[1] << 4) ^ (uint32_t)state[2];
                //                PrintState(state);
                //                PRINTHEX(aggregateState);
                //                printf("\n");

                InvSubBytes(state);
                //                PrintState(state);
                uint32_t aggregateResult =
                    ((uint32_t)state[0] << 8) ^ ((uint32_t)state[1] << 4) ^ (uint32_t)state[2];
                if (aggregateState % 8 == 0)
                    printf("\n");
                std::cout << "0x" << std::hex << std::setfill('0') << std::setw(8) << aggregateResult
                          << ", ";
                //                PRINTHEX(aggregateResult);
                //                printf("\n=========================\n");
            }
    printf("\n");
}

TEST(AES, Positive_FIPS_197) {
    uint8_t input[16] = {
        0x32, 0x43, 0xf6, 0xa8, //
        0x88, 0x5a, 0x30, 0x8d, //
        0x31, 0x31, 0x98, 0xa2, //
        0xe0, 0x37, 0x07, 0x34  //
    };

    uint8_t inputkey[16] = {
        0x2b, 0x7e, 0x15, 0x16, //
        0x28, 0xae, 0xd2, 0xa6, //
        0xab, 0xf7, 0x15, 0x88, //
        0x09, 0xcf, 0x4f, 0x3c  //
    };

    uint8_t testResult[16] = {
        0x39, 0x25, 0x84, 0x1d, //
        0x02, 0xdc, 0x09, 0xfb, //
        0xdc, 0x11, 0x85, 0x97, //
        0x19, 0x6a, 0x0b, 0x32, //
    };

    AES<uint8_t> aes;
    vector<uint8_t> data(input, input + 16);
    vector<uint8_t> key(inputkey, inputkey + 16);
    vector<uint8_t> test(testResult, testResult + 16);
    vector<uint8_t> encryptedData;
    vector<uint8_t> decryptedData;

    aes.Encrypt(data, key, encryptedData, 10);
    EXPECT_EQ(encryptedData, test);

    aes.Decrypt(encryptedData, key, decryptedData, 10);
    EXPECT_EQ(decryptedData, data);
}

TEST(AES, Positive_Random_3x3_HalfByte) {
    srand(time(0));
    AES<uint4_t> aes;
    vector<uint4_t> data;
    vector<uint4_t> key;
    vector<uint4_t> encryptedData;
    vector<uint4_t> decryptedData;
    for (int i = 0; i < 9; ++i) {
        data.push_back(rand());
    }
    for (int i = 0; i < 9; ++i) {
        key.push_back(rand());
    }

    aes.Encrypt(data, key, encryptedData, 5);
    aes.Decrypt(encryptedData, key, decryptedData, 5);
    EXPECT_EQ(decryptedData, data);
}

TEST(AES, SpeedTest_3x3_HalfByte) {
    srand(time(0));
    AES<uint4_t> aes;
    vector<uint4_t> data;
    vector<uint4_t> key;

    vector<uint4_t> data_direct;
    vector<uint4_t> encryptedData_direct;
    vector<uint4_t> decryptedData_direct;
    vector<uint4_t> data_fast;
    vector<uint4_t> encryptedData_fast;
    vector<uint4_t> decryptedData_fast;

    for (int i = 0; i < 9; ++i) {
        data.push_back(rand());
    }
    for (int i = 0; i < 9; ++i) {
        key.push_back(rand());
    }

    data_direct = data;
    data_fast = data;
    {
        Timer timer("Direct Encrypt: 5 Rounds");
        aes.Encrypt(data_direct, key, encryptedData_direct, 5);
    }

    {
        Timer timer("Direct Decrypt: 5 Rounds");
        aes.Decrypt(encryptedData_direct, key, decryptedData_direct, 5);
    }
    EXPECT_EQ(decryptedData_direct, data_direct);

    //////////////////////////////////////////////////////
    {
        Timer timer("Fast Encrypt: 5 Rounds  ");
        aes.Encrypt(data_fast, key, encryptedData_fast, 5, true);
    }
    EXPECT_EQ(encryptedData_fast, encryptedData_direct);

    {
        Timer timer("Fast Decrypt: 5 Rounds  ");
        aes.Decrypt(encryptedData_fast, key, decryptedData_fast, 5, true);
    }
    EXPECT_EQ(decryptedData_fast, data_fast);
}

TEST(AES, Test_Tetha_MixColumns_3x3_HalfByte) {
    AES<uint4_t> aes;
    EXPECT_TRUE(aes.Test_Tetha_MixColumns(9));
}

TEST(AES, Precompute_MixColumns_And_InvMixColumns) {
    AES<uint4_t> aes;
    aes.Precompute_MixColumns_And_InvMixColumns(9);
}

TEST(AES, Precompute_SubBytes_And_Inv_SubBytes) {
    AES<uint4_t> aes;
    aes.Precompute_Sbox_And_InvSbox(9);
}

TEST(AES, Test_MixColumns_Direct_And_Precompute_3x3_HalfByte) {
    AES<uint4_t> aes;
    EXPECT_TRUE(aes.Test_Direct_And_Precompute_MixColumns(9));
    EXPECT_TRUE(aes.Test_Direct_And_Precompute_InvMixColumns(9));
}

TEST(AES, Test_SubBytes_Direct_And_Precompute_3x3_HalfByte) {
    AES<uint4_t> aes;
    EXPECT_TRUE(aes.Test_Direct_And_Precompute_Sbox(9));
    EXPECT_TRUE(aes.Test_Direct_And_Precompute_InvSbox(9));
}

TEST(AES, Test_InvShiftRows_uint16_t) {
    AES<uint4_t> aes;
    EXPECT_TRUE(aes.Test_Uint16_ShiftRows());
}

TEST(AES, Test_Differential) {
    AES<uint4_t> aes;
    EXPECT_TRUE(aes.Test_Differential(9));
}
//-------------------------------------------------
//      Inversion tests
//-------------------------------------------------
TEST(AES, Test_Inversion_MixColumns_4x4_Byte) {
    AES<uint8_t> aes;
    EXPECT_TRUE(aes.Test_Inversion_MixColumns(16));
}

TEST(AES, Test_Inversion_MixColumns_3x3_HalfByte) {
    AES<uint4_t> aes;
    EXPECT_TRUE(aes.Test_Inversion_MixColumns(9));
}

TEST(AES, Test_Inversion_ShiftRows_4x4_Byte) {
    AES<uint8_t> aes;
    EXPECT_TRUE(aes.Test_Inversion_ShiftRows(16));
}

TEST(AES, Test_Inversion_ShiftRows_3x3_HalfByte) {
    AES<uint4_t> aes;
    EXPECT_TRUE(aes.Test_Inversion_ShiftRows(9));
}

TEST(AES, Test_Inversion_S_box_4x4_Byte) {
    AES<uint8_t> aes;
    EXPECT_TRUE(aes.Test_Inversion_Sbox(16));
}

TEST(AES, Test_Inversion_S_box_3x3_HalfByte) {
    AES<uint4_t> aes;
    EXPECT_TRUE(aes.Test_Inversion_Sbox(9));
}

// TEST(AES, Positive_Random_4x4_HalfByte) {
//    AES<uint4_t> aes;
//    vector<uint4_t> data(16, 0x0A);
//    vector<uint4_t> key(16, 0x08);
//    vector<uint4_t> encryptedData;

//    aes.Encrypt(data, key, encryptedData);
//}

// TEST(AES, Positive_Random_3x3_Byte) {
//    srand(time(0));
//    AES<uint8_t> aes;
//    vector<uint8_t> data;
//    vector<uint8_t> key;
//    for (int i = 0; i < 16; ++i) {
//        data.push_back(rand());
//    }
//    for (int i = 0; i < 16; ++i) {
//        key.push_back(rand());
//    }
//    vector<uint8_t> encryptedData;
//    vector<uint8_t> decryptedData;

//    aes.Encrypt(data, key, encryptedData);
//    aes.Decrypt(encryptedData, key, decryptedData);
//    EXPECT_EQ(decryptedData, data);
//}

// DO NOT PASS, Check Polynom
// TEST(AES, Test_Tetha_MixColumns_3x3_Byte) {
//     AES<uint8_t> aes;
//     EXPECT_TRUE(aes.Test_Tetha_MixColumns(9));
// }

// too long tests
// TEST(AES, Test_Tetha_MixColumns_4x4_HalfByte) {
//    AES<uint4_t> aes;
//    EXPECT_TRUE(aes.Test_Tetha_MixColumns(16));
//}

// TEST(AES, Test_Tetha_MixColumns_4x4_Byte) {
//    AES<uint4_t> aes;
//    EXPECT_TRUE(aes.Test_Tetha_MixColumns(16));
//}

// TEST(AES, Test_Inversion_MixColumns_3x3_HalfByte) {
//     AES<uint4_t> aes;
//     EXPECT_TRUE(aes.Test_Inversion_MixColumns(9));
// }

// TEST(AES, Test_Inversion_MixColumns_3x3_Byte) {
//     AES<uint8_t> aes;
//     EXPECT_TRUE(aes.Test_Inversion_MixColumns(9));
// }

// TEST(AES, Test_Inversion_MixColumns_4x4_HalfByte) {
//     AES<uint4_t> aes;
//     EXPECT_TRUE(aes.Test_Inversion_MixColumns(16));
// }
