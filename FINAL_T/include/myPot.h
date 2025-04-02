#pragma once
#include <Arduino.h>

// 这里定义 3 档的指令
enum class PotLevel : uint8_t {
    LEVEL_STOP = 0,
    LEVEL_CLEAN,
    LEVEL_HARDER
};

// 电位器类：根据模拟值划分三个区间
class MyPot {
public:
    MyPot(uint8_t pin);
    void begin();
    PotLevel getLevel();

private:
    uint8_t  _pin;
};
