#include "myEspNowSender.h"

EspNowSender::EspNowSender() {
    memset(_peerMac, 0, 6);
}

void EspNowSender::begin(const uint8_t *peerMac) {
    // 保存对端的 MAC
    memcpy(_peerMac, peerMac, 6);

    // 配置 WiFi 模式
    WiFi.mode(WIFI_MODE_STA);
    WiFi.disconnect(true);

    // 初始化 ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("[TX] ESP-NOW init failed, reboot...");
        ESP.restart();
    }

    // 注册发送回调
    esp_now_register_send_cb(EspNowSender::onDataSent);

    // 添加对端信息
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, _peerMac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("[TX] Add peer failed!");
    }

    Serial.println("[TX] ESPNOW init success");
}

void EspNowSender::sendCommand(TxCommand cmd) {
    TxPacket packet;
    packet.cmd = static_cast<uint8_t>(cmd);

    esp_err_t result = esp_now_send(_peerMac, (uint8_t*)&packet, sizeof(packet));
    if (result == ESP_OK) {
        Serial.printf("[TX] send cmd=%d OK\n", packet.cmd);
    } else {
        Serial.printf("[TX] send cmd=%d FAIL, err=%d\n", packet.cmd, result);
    }
}

void EspNowSender::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        // 发送成功
    } else {
        // 发送失败
    }
}
