#ifndef DIFFWAY_H
#define DIFFWAY_H

#include <aes/aes.h>
#include <helper/helper.h>

#include <map>
#include <vector>

class DiffWay {
  private:
    std::vector<std::vector<std::vector<uint8_t>>> SboxDiff;

  public:
    DiffWay();
    void checkMiddle(std::map<uint16_t, std::vector<std::vector<uint16_t>>> &all_W2_dW3,
                     std::map<uint16_t, std::vector<std::vector<uint16_t>>> &all_Y4_dX3);

    void directDifferential(std::map<uint16_t, std::vector<std::vector<uint16_t>>> &all_W2_dW3);

    void reverseDifferential(std::map<uint16_t, std::vector<std::vector<uint16_t>>> &all_Y4_dX3);

    void precomputeSboxDiff();
};

#endif // DIFFWAY_H
