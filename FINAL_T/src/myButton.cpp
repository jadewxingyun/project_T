#include "myButton.h"

MyButton::MyButton(uint8_t pin)
: _pin(pin)
, _lastSteadyState(HIGH)      // 假设初始未按下(对于 INPUT_PULLUP 来说，默认为高电平)
, _lastFlickerableState(HIGH)
, _lastDebounceTime(0)
, _pressedEventFlag(false)
, _releasedEventFlag(false)
, _pressedTime(0)
, _releasedTime(0)
{
}

void MyButton::begin() {
    // 使用内部上拉。若你是外部上拉/下拉，请调整为 INPUT 或其他
    pinMode(_pin, INPUT_PULLUP);

    // 初始读取，避免一上电时就有杂波导致误判
    int reading = digitalRead(_pin);
    _lastSteadyState = reading;
    _lastFlickerableState = reading;
    _lastDebounceTime = millis();
}

bool MyButton::update() {
    unsigned long currentTime = millis();
    int reading = digitalRead(_pin);

    // 每次更新前，先清空本次循环的“刚按下/刚释放”标志
    _pressedEventFlag = false;
    _releasedEventFlag = false;

    // 若本次读值与上次“波动值”不同，说明有翻转，记录防抖起点
    if (reading != _lastFlickerableState) {
        _lastFlickerableState = reading;
        _lastDebounceTime = currentTime;
    }

    // 判断是否还在防抖时段
    bool isDebouncing = (currentTime - _lastDebounceTime < DEBOUNCE_MS);

    // 超出防抖期，认定新电平已稳定
    if (!isDebouncing) {
        // 如果稳定电平与上一帧不同，说明确实发生了按下或释放
        if (_lastSteadyState != reading) {
            // 这里假设：按下时电平=LOW, 松开时=HIGH
            if (_lastSteadyState == HIGH && reading == LOW) {
                // 从松开->按下
                _pressedTime = currentTime;
                _pressedEventFlag = true;  
            }
            else if (_lastSteadyState == LOW && reading == HIGH) {
                // 从按下->松开
                _releasedTime = currentTime;
                _releasedEventFlag = true;
            }
            // 更新稳定电平
            _lastSteadyState = reading;
        }
    }

    return isDebouncing;
}

bool MyButton::isPressedEvent() {
    // 若 update() 检测到按下事件，就返回 true（一次性）
    return _pressedEventFlag;
}

bool MyButton::isReleasedEvent() {
    // 若 update() 检测到释放事件，就返回 true（一次性）
    return _releasedEventFlag;
}

unsigned long MyButton::getPressDuration() {
    // 上一次从按下到释放的时间差
    // 若按钮还没释放，则值保持上一次释放时计算出的结果
    return (_releasedTime - _pressedTime);
}
