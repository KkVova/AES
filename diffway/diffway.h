#ifndef DIFFWAY_H
#define DIFFWAY_H

#include <aes/aes.h>
#include <helper/helper.h>
#include <mset/mset.h>

#include <map>
#include <tuple>
#include <vector>

class DiffWay {
  private:
    std::vector<std::vector<std::vector<uint8_t>>> SboxDiff;

  public:
    DiffWay();
    void checkMiddle(const std::map<uint16_t, std::vector<std::vector<uint16_t>>> &all_W2_dW3,
                     const std::map<uint16_t, std::vector<std::vector<uint16_t>>> &all_Y4_dX3,
                     std::vector<std::tuple<uint16_t, uint64_t, uint16_t>> &ALL_W2_W3_Y4);

    void directDifferential(std::map<uint16_t, std::vector<std::vector<uint16_t>>> &all_W2_dW3);

    void reverseDifferential(std::map<uint16_t, std::vector<std::vector<uint16_t>>> &all_Y4_dX3);

    void multidimensionalDiff(const std::vector<std::tuple<uint16_t, uint64_t, uint16_t>> &ALL_W2_W3_Y4,
                              std::vector<Pair> &ALL_W5_0);

    void precomputeSboxDiff();

    void onlinePhase();
};

#endif // DIFFWAY_H
