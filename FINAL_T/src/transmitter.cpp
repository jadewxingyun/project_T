#include <Arduino.h>
#include "myButton.h"
#include "myPot.h"
#include "myEspNowSender.h"
#include "mySleepManager.h"
#include <esp_wifi.h>
// ========== 定义引脚 ==========
static const uint8_t PIN_POT = 36; // 电位器引脚(ESP32 ADC1)
static const uint8_t PIN_BTN_LEFT  = 34;
static const uint8_t PIN_BTN_RIGHT = 32;
static const uint8_t PIN_BTN_SKIP = 33;
// ========== 发送间隔(毫秒) ==========
static const unsigned long SEND_INTERVAL = 150;

// ========== 若超过此时间无输入, 进入休眠 ==========
static const unsigned long INACTIVITY_TIMEOUT = 30000;

// P_to_P MAC address
uint8_t peerMac[6] = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC};

// 全局对象
MyButton btnLeft(PIN_BTN_LEFT);
MyButton btnRight(PIN_BTN_RIGHT);
MyButton btnSkip(PIN_BTN_SKIP);
MyPot       pot(PIN_POT);
SleepManager sleepMgr(INACTIVITY_TIMEOUT);

unsigned long lastSendTime = 0;
bool leftPressedPrev  = false;
bool rightPressedPrev = false;

// 帮助函数：根据电位器档位 + 按钮状态 -> 生成要发送的指令
TxCommand makeCommand(PotLevel level, bool leftDown, bool rightDown , bool skipDown) {
    //LeftDown + Low => LEFT_LOW,  LeftDown + High => LEFT_HIGH
    Serial.printf("Level:%.2f LeftDown:%d  rightDown:%d, Both:%d\n",level,leftDown,rightDown);
    if (level == PotLevel::LEVEL_STOP) {
        return TxCommand::STOP;
    }
    switch (level) {
    case PotLevel::LEVEL_CLEAN:

        if (!skipDown) {
            // 如果左键按下
            if (leftDown && !rightDown) {
                return TxCommand::LEFT_LOW; // Low
            }
            // 如果右键按下
            if (rightDown && !leftDown) {
                return TxCommand::RIGHT_LOW;
            }
        }
        else {
            if (leftDown && !rightDown) {
                return TxCommand::LEFT_LOW_BOTH; // Low
            }
            // 如果右键按下
            if (rightDown && !leftDown) {
                return TxCommand::RIGHT_LOW_BOTH;
            }
        }
        if (!rightDown && !leftDown) {
            return TxCommand::CLEAN_LOW;
        }
        break;
    case PotLevel::LEVEL_HARDER:
        // “中档” => left => LEFT_HIGH, right => RIGHT_HIGH, both => CLEAN
        if (!skipDown) {
            if (leftDown && !rightDown) {
                return TxCommand::LEFT_HIGH;
            }
            if (rightDown && !leftDown) {
                return TxCommand::RIGHT_HIGH;
            }
        }else{
            if (leftDown && !rightDown) {
                return TxCommand::LEFT_HIGH_BOTH;
            }
            if (rightDown && !leftDown) {
                return TxCommand::RIGHT_HIGH_BOTH;
            }
        }
        if (!rightDown && !leftDown) {
            return TxCommand::CLEAN_HIGH;
        }
        break;
    }
    return TxCommand::INITIAL_STATE;
}

void setup() {
    Serial.begin(115200);

    // 初始化各模块
    btnLeft.begin();
    btnRight.begin();
    btnSkip.begin();
    pot.begin();
    espnow.begin(peerMac);

    Serial.println("[MAIN] Setup done.");
}

void loop() {
    unsigned long now = millis();

    // 1. 更新三个按钮的状态（使用 MyButton 类的防抖及事件检测）
    bool leftDebouncing  = btnLeft.update();
    bool rightDebouncing = btnRight.update();
    bool skipDebouncing  = btnSkip.update();

    // 2. 检测是否有按钮事件（按下或释放），若有则重置不活动计时
    bool leftEvent  = btnLeft.isPressedEvent()  || btnLeft.isReleasedEvent();
    bool rightEvent = btnRight.isPressedEvent() || btnRight.isReleasedEvent();
    bool skipEvent  = btnSkip.isPressedEvent()  || btnSkip.isReleasedEvent();
    if (leftEvent || rightEvent || skipEvent) {
        sleepMgr.resetTimer();
    }

    // 3. 检测电位器电平变化，若变化则也重置计时
    static PotLevel prevLevel = PotLevel::LEVEL_STOP;
    PotLevel currentLevel = pot.getLevel();
    if (currentLevel != prevLevel) {
        prevLevel = currentLevel;
        sleepMgr.resetTimer();
    }

    // 4. 获取当前按钮稳定状态
    // 使用 digitalRead() 读取实际电平（假设防抖后状态已稳定），（可以在 MyButton 中增加一个获取稳定状态的接口）
    bool skipDown  = (digitalRead(PIN_BTN_SKIP)  == LOW);
    bool leftDown  = (digitalRead(PIN_BTN_LEFT)  == LOW);
    bool rightDown = (digitalRead(PIN_BTN_RIGHT) == LOW);
    
    // 5. 根据电位器和按钮状态生成命令
    TxCommand cmd = makeCommand(currentLevel, leftDown, rightDown, skipDown);

    // 6. 定时发送命令
    if (now - lastSendTime >= SEND_INTERVAL) {
        lastSendTime = now;
        espnow.sendCommand(cmd);

        // 发送完成后，为了省电，进入 light sleep 模式
        esp_wifi_stop();
        esp_sleep_enable_timer_wakeup(150ULL * 1000ULL); // 100ms
        esp_light_sleep_start();
        // 醒来后重新启动 Wi-Fi 和 esp-now
        WiFi.mode(WIFI_MODE_STA);
        WiFi.disconnect(true);
        esp_now_init();
        
    }


    sleepMgr.update();
}
