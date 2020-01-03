#ifndef DIFFWAY_H
#define DIFFWAY_H

#include <aes/aes.h>
#include <helper/helper.h>

#include <vector>

class DiffWay {
  private:
    std::vector<std::vector<std::vector<uint8_t>>> SboxDiff;

  public:
    DiffWay();
    void checkMiddle(std::vector<std::vector<uint16_t>> &all_dW3,
                     std::vector<std::vector<uint16_t>> &all_dX3);

    void directDifferential(std::vector<std::vector<uint16_t>> &all_dW3);

    void reverseDifferential(std::vector<std::vector<uint16_t>> &all_dX3);

    void precomputeSboxDiff();
};

#endif // DIFFWAY_H
