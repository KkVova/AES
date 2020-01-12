#ifndef AES_H
#define AES_H

#include <cstdint>
#include <vector>

using std::vector;

class DiffWay;

template <typename TYPE> class AES {
    friend class DiffWay;

  public:
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
    static vector<uint16_t> InvShiftRows(uint16_t state) {

        vector<uint16_t> result;
        result.push_back(state & 0xF00);
        result.push_back(state & 0x0F0);
        result.push_back(state & 0x0F);
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

    static void ShiftRows(vector<uint16_t> &state) {
        assert(state.size() == 3);

        vector<uint16_t> tmp = state;

        tmp[0] = (state[0] & 0xF00) ^ (state[1] & 0x0F0) ^ (state[2] & 0x00F);
        tmp[1] = (state[1] & 0xF00) ^ (state[2] & 0x0F0) ^ (state[0] & 0x00F);
        tmp[2] = (state[2] & 0xF00) ^ (state[0] & 0x0F0) ^ (state[1] & 0x00F);

        state = tmp;
    }
};

#endif // AES_H
