#ifndef AES_H
#define AES_H

#include <cstdint>
#include <vector>

using std::vector;

template <typename TYPE> class AES {
  public:
    AES() {}
    ~AES() {}

    void Encrypt(const vector<TYPE> &data, vector<TYPE> &key, vector<TYPE> &encryptedMessage,
                 const int &numberOfRounds = 10);

    void Decrypt(const vector<TYPE> &encryptedData, vector<TYPE> &key, vector<TYPE> &message,
                 const int &numberOfRounds = 10);

    bool Test_Tetha_MixColumns(int size = 16);

    bool Test_Inversion_MixColumns(int size = 16);

    bool Test_Inversion_ShiftRows(int size = 16);

    bool Test_Inversion_Sbox(int size = 16);

  private:
    uint8_t *Rcon;

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

    // Helper functions
    uint32_t FindTeta(vector<TYPE> &state);

    void PrintState(vector<TYPE> &state);
};

#endif // AES_H
