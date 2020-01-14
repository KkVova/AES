#ifndef AES_H
#define AES_H

#include <cstdint>
#include <vector>

using std::vector;

class DiffWay;

template <typename TYPE> class AES {
    friend class DiffWay;

  public:
    //
    static void KeySchedule(const vector<uint16_t> &key, vector<vector<uint16_t>> &roundKeys,
                            const uint8_t &numberOfRounds) {

        vector<uint16_t> nextKey = key;
        vector<uint16_t> prevKey;
        roundKeys.push_back(key);
        for (int roundNum = 0; roundNum < numberOfRounds; ++roundNum) {
            prevKey = nextKey;
            nextKey.clear();
            uint16_t tmp = precompute_SubBytes[prevKey[2]];

            uint16_t first = (uint16_t)((tmp >> 4) & 0x0F) ^ (uint16_t)((prevKey[0] >> 8) & 0x0F) ^
                             rcon_HalfByte[roundNum];
            uint16_t second = (uint16_t)(tmp & 0x0F) ^ (uint16_t)((prevKey[0] >> 4) & 0x0F);
            uint16_t third = (uint16_t)((tmp >> 8) & 0x0F) ^ (uint16_t)(prevKey[0] & 0x0F);
            nextKey.push_back((first << 8) | (second << 4) | third);

            for (int i = 1; i < 3; ++i)
                nextKey.push_back(nextKey[i - 1] ^ prevKey[i]);

            roundKeys.push_back(nextKey);
        }
    }

    static void Encrypt(const vector<uint16_t> &data, const vector<vector<uint16_t>> &roundKeys,
                        vector<uint16_t> &encryptedData, const uint8_t &numberOfRounds) {
        assert(data.size() == 3);

        vector<uint16_t> state = data;
        auto keyIt = roundKeys.begin();

        AES::AddRoundKey(state, keyIt++);
        for (int i = 1; i < numberOfRounds; i++) {
            AES::SubBytes(state);
            AES::ShiftRows(state);
            AES::MixColumns(state);
            AES::AddRoundKey(state, keyIt++);
        }

        AES::SubBytes(state);
        AES::ShiftRows(state);
        AES::AddRoundKey(state, keyIt);
        encryptedData = state;
    }

    static void MixColumns(vector<uint16_t> &state) {
        for (int i = 0; i < state.size(); i++)
            state[i] = precompute_MixColumns[state[i]];
    }

    static void ShiftRows(vector<uint16_t> &state) {
        assert(state.size() == 3);

        vector<uint16_t> tmp = state;

        tmp[0] = (state[0] & 0xF00) ^ (state[1] & 0x0F0) ^ (state[2] & 0x00F);
        tmp[1] = (state[1] & 0xF00) ^ (state[2] & 0x0F0) ^ (state[0] & 0x00F);
        tmp[2] = (state[2] & 0xF00) ^ (state[0] & 0x0F0) ^ (state[1] & 0x00F);

        state = tmp;
    }

    static void SubBytes(vector<uint16_t> &state) {
        for (int i = 0; i < state.size(); i++)
            state[i] = precompute_SubBytes[state[i]];
    }

    static void AddRoundKey(vector<uint16_t> &state,
                            const vector<vector<uint16_t>>::const_iterator &key) {
        for (int i = 0; i < state.size(); i++)
            state[i] = state[i] ^ (*key)[i];
    }

    AES() : isFast(false) {}
    ~AES() {}

    void setFast() { isFast = true; }

    void unsetFast() { isFast = false; }

    void Encrypt(const vector<TYPE> &data, vector<TYPE> &key, vector<TYPE> &encryptedMessage,
                 const int &numberOfRounds = 10, bool fast = false);

    void Decrypt(const vector<TYPE> &encryptedData, vector<TYPE> &key, vector<TYPE> &message,
                 const int &numberOfRounds = 10, bool fast = false);

    //===================================================================
    // Unit Tests
    //===================================================================
    bool Test_Tetha_MixColumns(const int size = 16);

    bool Test_Inversion_MixColumns(const int size = 16);

    bool Test_Inversion_ShiftRows(const int size = 16);

    bool Test_Inversion_Sbox(const int size = 16);

    bool Test_Direct_And_Precompute_MixColumns(const int size = 9);

    bool Test_Direct_And_Precompute_InvMixColumns(const int size = 9);

    bool Test_Direct_And_Precompute_Sbox(const int size = 9);

    bool Test_Direct_And_Precompute_InvSbox(const int size = 9);

    bool Test_Uint16_ShiftRows();

    bool Test_Differential(const int size = 9);

    void Precompute_MixColumns_And_InvMixColumns(const int size = 9);

    void Precompute_Sbox_And_InvSbox(const int size = 9);

    //===================================================================
  private:
    // parameter defines substitution for MixColumnes and S_box operations
    bool isFast;

    // constants for MixColumns polynom
    uint8_t A_param, B_param, C_param, D_param;

    void InitAES(vector<TYPE> &key, vector<TYPE> &keySchedule, bool isEncrypt = true,
                 const int &numberOfRounds = 10);

    void Round(vector<TYPE> &state, vector<TYPE> &key);

    void InvRound(vector<TYPE> &state, vector<TYPE> &key);

    void FinalRound(vector<TYPE> &state, vector<TYPE> &key);

    void InvFinalRound(vector<TYPE> &state, vector<TYPE> &key);

    void KeySchedule(vector<TYPE> &prevKey, vector<TYPE> &nextKey, const uint8_t &round);

    void MixColumns(vector<TYPE> &state);

    void ShiftRows(vector<TYPE> &state);

    void SubBytes(vector<TYPE> &state);

    void AddRoundKey(vector<TYPE> &state, vector<TYPE> &key);

    void InvMixColumns(vector<TYPE> &state);

    void InvShiftRows(vector<TYPE> &state);

    void InvSubBytes(vector<TYPE> &state);

    // Helper function
    uint32_t FindTeta(vector<TYPE> &state);

  public:
    // state, 000, 000
    static vector<uint16_t> InvShiftRows(uint16_t state) {

        vector<uint16_t> result;
        result.push_back(state & 0xF00);
        result.push_back(state & 0x0F0);
        result.push_back(state & 0x00F);
        return result;
    }

    // state, 000, 000
    static vector<uint16_t> ShiftRows(uint16_t state) {

        vector<uint16_t> result;
        result.push_back(state & 0xF00);
        result.push_back(state & 0x00F);
        result.push_back(state & 0x0F0);
        return result;
    }

    static void InvShiftRows(vector<uint16_t> &state) {
        assert(state.size() == 3);

        vector<uint16_t> tmp = state;

        tmp[0] = (state[0] & 0xF00) ^ (state[2] & 0x0F0) ^ (state[1] & 0x00F);
        tmp[1] = (state[1] & 0xF00) ^ (state[0] & 0x0F0) ^ (state[2] & 0x00F);
        tmp[2] = (state[2] & 0xF00) ^ (state[1] & 0x0F0) ^ (state[0] & 0x00F);

        state = tmp;
    }
};

#endif // AES_H
