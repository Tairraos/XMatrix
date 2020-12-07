#include "wifi.h"
#include <SoftwareSerial.h>

SoftwareSerial mySerial(4, 5); // RX, TX
#define _SS_MAX_RX_BUFF 300
#define wifi_led 12
#define reset_btn 13

int time_cnt = 0, cnt = 0, init_flag = 0;

void setup() {
  pinMode(reset_btn, INPUT_PULLUP); // 重置Wi-Fi按键初始化
  pinMode(wifi_led, OUTPUT);      // Wi-Fi状态指示灯没有接

  mySerial.begin(9600);    // 软件串口初始化
  mySerial.println("myserial init successful!");
  Serial.begin(115200);    // IO4 RX   IO5 TX
  Serial.println("serial init successful!");

  wifi_protocol_init();
}

void loop() {
  if (init_flag == 0) {
    time_cnt++;
    if (time_cnt % 6000 == 0) {
      time_cnt = 0;
      cnt ++;
    }
    wifi_stat_led(&cnt); // Wi-Fi状态处理
  }
  wifi_uart_service();
  myserialEvent();      // 串口接收处理
  key_scan();           // 重置配网按键检测

}

void myserialEvent() {
  if (mySerial.available()) {
    unsigned char ch = (unsigned char)mySerial.read();
    uart_receive_input(ch);
  }
}

void key_scan(void)
{
  static char ap_ez_change = 0;
  unsigned char buttonState  = HIGH;
  buttonState = digitalRead(reset_btn);
  if (buttonState == LOW) {
    delay(3000);
    buttonState = digitalRead(reset_btn);
      printf("----%d",buttonState);
    if (buttonState == LOW) {
      printf("123\r\n");
      init_flag = 0;
      switch (ap_ez_change) {
        case 0 :
          mcu_set_wifi_mode(SMART_CONFIG);
          break;
        case 1 :
          mcu_set_wifi_mode(AP_CONFIG);
          break;
        default:
          break;
      }
      ap_ez_change = !ap_ez_change;
    }

  }
}

void wifi_stat_led(int *cnt)
{
  switch (mcu_get_wifi_work_state())
  {
    case SMART_CONFIG_STATE:  //0x00
      init_flag = 0;
      if (*cnt == 2) {
        *cnt = 0;
      }
      if (*cnt % 2 == 0)  //LED快闪
      {
        digitalWrite(wifi_led, LOW);
      }
      else
      {
        digitalWrite(wifi_led, HIGH);
      }
      break;
    case AP_STATE:  //0x01
      init_flag = 0;
      if (*cnt >= 30) {
        *cnt = 0;
      }
      if (*cnt  == 0)      // LED 慢闪
      {
        digitalWrite(wifi_led, LOW);
      }
      else if (*cnt == 15)
      {
        digitalWrite(wifi_led, HIGH);
      }
      break;

    case WIFI_NOT_CONNECTED:  // 0x02
      digitalWrite(wifi_led, HIGH); // LED 熄灭
      break;
    case WIFI_CONNECTED:  // 0x03
      break;
    case WIFI_CONN_CLOUD:  // 0x04
      if ( 0 == init_flag )
      {
        digitalWrite(wifi_led, LOW);// LED 常亮
        init_flag = 1;                  // Wi-Fi 连接上后该灯可控
        *cnt = 0;
      }

      break;

    default:
      digitalWrite(wifi_led, HIGH);
      break;
  }
}
