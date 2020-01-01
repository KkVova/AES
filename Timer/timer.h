#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <cmath>
#include <ctime>
#include <iostream>
#include <string>

// TODO: Implementation of Pause-Resume system
class Timer {
  public:
    Timer(std::string str) : m_testName(str) { m_StartTime = std::chrono::steady_clock::now(); }

    ~Timer() {
        m_EndTime = std::chrono::steady_clock::now();

        auto dur = m_EndTime - m_StartTime;
        typedef std::chrono::duration<float> float_seconds;
        auto secs = std::chrono::duration_cast<float_seconds>(dur);
        std::cout << "[SpeedInfo | " << m_testName << "] : " << secs.count() << std::endl;
    }

  private:
    std::string m_testName;

    std::chrono::time_point<std::chrono::steady_clock> m_StartTime;
    std::chrono::time_point<std::chrono::steady_clock> m_EndTime;
};

#endif // TIMER_H