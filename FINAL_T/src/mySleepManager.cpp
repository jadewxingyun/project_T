#include "mySleepManager.h"
#include <esp_sleep.h>
#include <esp_wifi.h>

SleepManager::SleepManager(unsigned long inactivityMs)
: _timeout(inactivityMs)
, _lastActive(millis())
{}

void SleepManager::resetTimer() {
    _lastActive = millis();
}

void SleepManager::update() {
    unsigned long now = millis();
    if ((now - _lastActive) > _timeout) {
        Serial.printf("[Sleep] Inactivity > %lu ms, going to DEEP SLEEP\n", _timeout);
        // 设置唤醒源(外部引脚唤醒、定时唤醒等可以考虑按钮唤醒调用ipressed 事件)
        // 1分钟定时唤醒
        esp_sleep_enable_timer_wakeup(60ULL * 1000000ULL);
        
        //按键唤醒，需要将引脚配置为EXT0/EXT1等
        // eg: esp_sleep_enable_ext0_wakeup((gpio_num_t)14, 0);
        
        Serial.flush();
        esp_deep_sleep_start();
    }
}
