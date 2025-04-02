#include "myPot.h"

MyPot::MyPot(uint8_t pin)
: _pin(pin)
{}

void MyPot::begin() {
    pinMode(_pin, INPUT); // 设置ESP32 ADC1引脚
}

PotLevel MyPot::getLevel() {
    int rawVal = analogRead(_pin); // 0~4095(ESP32默认12bit)
    

    if (rawVal < 50) {
        Serial.println("PotLevel: LEVEL_STOP");
        return PotLevel::LEVEL_STOP;   // Stop
    } else if (rawVal < 4050) { 
        Serial.println("PotLevel: LEVEL_CLEAN");
        return PotLevel::LEVEL_CLEAN;  // Clean
    } else {
        Serial.println("PotLevel: LEVEL_CLEAN_HARDER");
        return PotLevel::LEVEL_HARDER; // Harder
    }
}
