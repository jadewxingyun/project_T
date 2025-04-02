#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// =========== 引入 NeoPixel 库 ===========
#include <Adafruit_NeoPixel.h>

// =========== NeoPixel 配置 ===========
// 8 颗灯珠，接在 GPIO 6 
#define LED_PIN    6        
#define LED_COUNT  8        
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// =========== 分组信息 ===========

int groups[][2] = {
    {0, 1},  // (1,2)
    {2, 5},  // (3,6)
    {3, 4},  // (4,5)
    {6, 7}   // (7,8)
};
int groupCount = sizeof(groups) / sizeof(groups[0]);

// =========== 帮助函数：设置指定组的颜色 ===========
void setGroupColor(int groupIndex, uint32_t color) {
    for (int i = 0; i < 2; i++) {
        int ledIndex = groups[groupIndex][i];
        strip.setPixelColor(ledIndex, color);
    }
}

// =========== 枚举：接收端的指令 ===========
enum RxCommand {
    CMD_STOP = 0,
    CMD_LEFT_LOW,
    CMD_LEFT_HIGH,
    CMD_RIGHT_LOW,
    CMD_RIGHT_HIGH,
    CMD_LEFT_LOW_REQUIRED,
    CMD_LEFT_HIGH_REQUIRED,
    CMD_RIGHT_LOW_REQUIRED,
    CMD_RIGHT_HIGH_REQUIRED,
    CMD_CLEAN_LOW,
    CMD_CLEAN_HIGH,
    CMD_INITIAL_STATE,
    };

// =========== 接收的数据包结构 ===========
typedef struct {
    uint8_t cmd;
} RxPacket;

// =========== ESP-NOW 接收回调函数 ===========
void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    // 当接收到的数据长度小于 RxPacket 大小，直接返回，不处理
    if (len < sizeof(RxPacket)) return;

    // 将接收到的数据复制到 RxPacket 结构体中
    RxPacket rx;
    memcpy(&rx, incomingData, sizeof(rx));

    // 取出命令字段
    uint8_t c = rx.cmd;
    Serial.printf("[RX] Received cmd=%d\n", c);

    // 清空灯带，保证每次都是干净状态
    strip.clear();

    // 根据接收到的命令执行对应的灯光效果
    switch (c) {

        case 0:
            for (int i = 0; i < LED_COUNT; i++) {
                strip.setPixelColor(i, strip.Color(255, 0, 0)); // 红色：R=255, G=0, B=0
            }
            break;

        case 1:
            // CMD_LEFT_LOW：点亮第 0 组灯为蓝色点亮第 2 组灯为绿色（表示左轻扫）
            setGroupColor(0, strip.Color(0, 0, 255));//蓝色
            setGroupColor(2, strip.Color(0, 255, 0));   
            break;
        case 2:
            // CMD_LEFT_HIGH：点亮第 0 组灯为蓝色点亮第 2 组灯为黄色（表示左重扫）
            setGroupColor(0, strip.Color(0, 0, 255));
            setGroupColor(2, strip.Color(255, 255, 0));    
            break;    
        case 3:
            // CMD_RIGHT_LOW：点亮第 3 组灯为蓝色点亮第 2 组灯为绿色（表示右轻扫）
            setGroupColor(3, strip.Color(0, 0, 255));
            setGroupColor(2, strip.Color(0, 255, 0));   
            break;
        case 4:
            // CMD_RIGHT_LOW：点亮第 4 组灯为蓝色点亮第 2 组灯为黄色（表示右重扫）
            setGroupColor(3, strip.Color(0, 0, 255));
            setGroupColor(2, strip.Color(255, 255, 0));    
            break;
        case 5:
            // CMD_LEFT_LOW_BOTH：点亮第 0 组灯为蓝色点亮第 2 组灯为绿色（表示左轻扫）
            // 点亮第1组skip 要求both清扫
            setGroupColor(0, strip.Color(0, 0, 255));
            setGroupColor(2, strip.Color(0, 255, 0));
            setGroupColor(1, strip.Color(128, 0, 128));     
            break;
        case 6:
            // CMD_LEFT_HIGH_BOTH：点亮第 0 组灯为蓝色点亮第 2 组灯为黄色（表示左重扫）
            // 点亮第1组skip 要求both清扫
            setGroupColor(0, strip.Color(0, 0, 255));
            setGroupColor(2, strip.Color(255, 255, 0));
            setGroupColor(1, strip.Color(128, 0, 128));      
            break;    
        case 7:
            // CMD_RIGHT_LOW_BOTH：点亮第 3 组灯为蓝色点亮第 2 组灯为绿色（表示右轻扫）
            // 点亮第1组skip 要求both清扫
            setGroupColor(3, strip.Color(0, 0, 255));
            setGroupColor(2, strip.Color(0, 255, 0));
            setGroupColor(1, strip.Color(128, 0, 128));     
            break;
        case 8:
            // CMD_RIGHT_LOW_BOTH：点亮第 4 组灯为蓝色点亮第 2 组灯为黄色（表示右重扫
            // 点亮第1组skip 要求both清扫
            setGroupColor(3, strip.Color(0, 0, 255));
            setGroupColor(2, strip.Color(255, 255, 0));
            setGroupColor(1, strip.Color(128, 0, 128));   
            break;
        case 9:
            // CMD_CLEAN_LOW：点亮第 2 组灯为绿色（表示左轻扫）
            // 点亮第1组skip 要求both清扫
            setGroupColor(2, strip.Color(0, 255, 0));     
            break;
        case 10:
            // CMD_CLEAN_HIGH：点亮第 2 组灯为黄色（表示左重扫）
            // 点亮第1组skip 要求both清扫
            setGroupColor(2, strip.Color(255, 255, 0));      
            break;    
        case 11:
            // CMD_RIGHT_LOW_BOTH：点亮第 3 组灯为蓝色点亮第 2 组灯为绿色（表示右轻扫）
            // 点亮第1组skip 要求both清扫
            for (int i = 0; i < LED_COUNT; i++) {
                strip.setPixelColor(i, strip.Color(0, 0, 0)); 
            }    
            break;

        default:
            // 其他未定义的命令：不处理
            break;
    }

    // 设置完成后，统一刷新灯带
    strip.show();
}


void setup() {
    Serial.begin(115200);

  // 初始化 NeoPixel
    strip.begin();
  strip.show(); // 清空
    Serial.println("[RX] NeoPixel init OK");

  // 配置 WiFi
    WiFi.mode(WIFI_MODE_STA);
    WiFi.disconnect(true);

    // 初始化 ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("[RX] ESPNOW init failed, restarting...");
        ESP.restart();
    }
    // 注册回调
    esp_now_register_recv_cb(onDataRecv);

    Serial.println("[RX] Setup done.");
    }

void loop() {
    // 这里可以加 light_sleep
    delay(1000);
}
