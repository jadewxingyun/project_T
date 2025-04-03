#pragma once
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

// 定义指令类型 - 结合按钮和电位器
enum class TxCommand : uint8_t {
    STOP       = 0,
    LEFT_LOW   = 1,
    LEFT_HIGH  = 2,
    RIGHT_LOW  = 3,
    RIGHT_HIGH = 4,
    LEFT_LOW_BOTH=5,
    LEFT_HIGH_BOTH  = 6,
    RIGHT_LOW_BOTH  = 7,
    RIGHT_HIGH_BOTH = 8,
    CLEAN_LOW = 9,
    CLEAN_HIGH = 10,
    INITIAL_STATE = 11,
};

// 发送数据包结构
struct TxPacket {
    uint8_t cmd;
};

class EspNowSender {
public:
    EspNowSender();
    void begin(const uint8_t *peerMac);
    void sendCommand(TxCommand cmd);

private:
    static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
    uint8_t _peerMac[6];
};
