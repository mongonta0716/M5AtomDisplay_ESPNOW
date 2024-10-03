#include <Arduino.h>
#include <M5AtomDisplay.h>
#include <M5Unified.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

M5AtomDisplay display(240, 120, 120, 480, 320, 2, 2, 74250000);

String hello_str = "こんにちは、私はスタックチャンです。スーパーかわいい手乗りロボットとしてあなたのそばにいます。";

esp_now_peer_info_t esp_ap;
const uint8_t *peer_addr = esp_ap.peer_addr;
const esp_now_peer_info_t *peer = &esp_ap;


uint8_t send_data[250]; // ESP-NOWの送信バッファ(250Byteまで)
uint32_t recv_count = 0;
#define WIFI_DEFAULT_CHANNEL 1
#define WIFI_SECONDORY_CHANNEL 2

// コールバック関数実行時に排他制御をする。
SemaphoreHandle_t xMutex = NULL;

void onRecvData(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  BaseType_t xStatus;
  const TickType_t xTicksToWait = 100UL;
  xSemaphoreGive(xMutex);
  xStatus = xSemaphoreTake(xMutex, xTicksToWait);
  if (xStatus == pdTRUE) {
    recv_count++;
    // ログを画面表示したい場合は排他をかけないと失敗する場合あり
    Serial.println("onRecvData");
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
        mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  

    char buf[250];
    memcpy(&buf, data, data_len);
    // gfx.printf("Recv:%s", macStr);
    Serial.printf("%s\n", buf);
    for (int i = display.width(); i > -display.textWidth(buf); --i) {
      display.startWrite();
      display.drawString(buf, i, (display.height() - display.fontHeight()) >> 1);
      //display.printf("recv:%d:%s\n",recv_count,buf);
      display.endWrite();
      vTaskDelay(10/portTICK_RATE_MS); // スクロールするときにWaitしないとハングアップするので必要に応じて調整が必要 
    }
  }
  xSemaphoreGive(xMutex);
}

void onSentData(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // ログを画面表示したい場合は排他をかけないと失敗する場合あり
  Serial.println("onSent");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
  BaseType_t xStatus;
  const TickType_t xTicksToWait = 1000U;
  xStatus = xSemaphoreTake(xMutex, xTicksToWait);
  if (xStatus == pdTRUE) {
    display.startWrite();
    display.print("onSent:");
    display.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
    display.endWrite();
  }
  xSemaphoreGive(xMutex);
}

void setup() {
  display.init();
  delay(5000);
  display.setFont(&fonts::lgfxJapanGothic_40);
  uint8_t mac_addr[6];
  esp_read_mac(mac_addr, ESP_MAC_WIFI_STA);
  //display.printf("%02X:%02X:%02X:%02X:%02X:%02X", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  M5_LOGI("%02X:%02X:%02X:%02X:%02X:%02X\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);


  WiFi.mode(WIFI_STA);
  ESP_ERROR_CHECK(esp_wifi_set_channel(WIFI_DEFAULT_CHANNEL, WIFI_SECOND_CHAN_ABOVE));
  if (esp_now_init() == 0) {
    M5_LOGI("esp now init");
  } else {
    M5_LOGI("esp now init failed");
    display.println("esp now init failed");
  }


  xMutex = xSemaphoreCreateMutex();
  esp_now_register_recv_cb(onRecvData);
  esp_now_register_send_cb(onSentData);


}

void loop() {
  delay(20);
}