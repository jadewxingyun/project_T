#pragma once
#include <Arduino.h>

// 简易按钮类：包含防抖、按下/释放时刻记录等
class MyButton {
public:
    MyButton(uint8_t pin);

    void begin();
    bool update();            // 每次循环都调用，用于处理防抖; 返回值表示“是否处于防抖中”
    bool isPressedEvent();    // 判断是否刚刚从 低->高（或根据需要更改）发生了按下事件
    bool isReleasedEvent();   // 判断是否刚刚从 高->低 发生了释放事件
    unsigned long getPressDuration();

private:
    uint8_t  _pin;
    int      _lastSteadyState;
    int      _lastFlickerableState;
    bool _pressedEventFlag;
    bool _releasedEventFlag;
    unsigned long _lastDebounceTime;
    unsigned long _pressedTime;
    unsigned long _releasedTime;

    static const unsigned long DEBOUNCE_MS = 50;      // 防抖时长
};
