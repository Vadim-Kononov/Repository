/**
  Шаблон для ESP32  в котором нстроены ОТА, MQTT, BluetoothSerial, NTP Time, EEPROM, WDT, ThingSpeak, IFTTT
  Был опробован на ESP32 WeMos Mini
*/

/*
  ESP32 LG-Split Room
  I2C device found at address 0x3C  ! Oled Дисплей
  I2C device found at address 0x76  ! BME 280
  GPIO21 (IO21)                     ! SDA
  GPIO22 (IO22)                     ! SCL
  GPIO17 (IO17)                     ! Выход на IR излучатель
*/

// Включение вывода в Serial при компиляции
//#define Serial_On
//#define Serial_On_GET

// Имя модуля
#define NAME "LG Split"

// Количество попыток подключения к WiFi за одну сессию и встроенный светодиод
#define Connection_Count_WiFi 15
#define Onboard_Led 2

// Определения для WiFi, OTA, MQTT
#include <rom/rtc.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

char ssid [30];
char password [30];

#include "Account.h"

// Определения для Bluetoot
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

// Определения для записи-чтения EEPROM
#include <Preferences.h>
Preferences memory;

// Определения для NTP Time
#include "time.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;
const int   daylightOffset_sec = 3600;
struct tm timeinfo;

// Определения для WDT
#include "esp_system.h"
const int wdtTimeout = 60000;  //time in ms to trigger the watchdog
hw_timer_t *timer = NULL;
void IRAM_ATTR ResetModule() {
  Serial.println("Reboot WdT");
  ESP.restart();
}

// Определения для BME280
#include <Wire.h>
#include <BME280I2C.h>
BME280I2C bme;

// Определения для IRremote
#include <IRremoteESP8266.h>    //2.6.3
#include <IRsend.h>
IRsend irsend(17);              // An IR LED is controlled by GPIO17 (IO17)

// Определения для работы с U8g2lib
#include <Arduino.h>
#include <U8g2lib.h>            //2.26.14
U8G2_SSD1306_64X48_ER_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Глобальные переменные

// Таймеры на 61, 2, 1 сек.
long count_ms_Timer60, time_ms_Timer60, count_ms_Timer2, time_ms_Timer2, count_ms_Timer1, time_ms_Timer1;

// Флаги ввода пароля для SerialBT, рекконекта WiFi при перезагрузке
bool key_flag = false, res_flag = false, split_flag = false, timer_flag;

// Данные с BME280
float tem_BME280, tem_split, tem_stored, tem_min, correction, pres_split, hum_split;

// Счетчики обратного таймера
int timer_sec, timer_min, timer_min_mem, timer_min_set;

// Массивы команд для отправки через IR
unsigned long command_value [11] = {0x8800347, 0x88C0051, 0x880830B, 0x880832D, 0x880834F, 0x8810001, 0x8810089, 0x88903C8, 0x8890788, 0x8890B48, 0x8890009};
String command_name [11] = {"🔛 On", "❌ Off", "🌑 Тихо", "🌓 Средне", "🌕 Быстро", "♓ Шторки", "🌀 Интенсивно",  "🕐 1", "🕑 2", "🕒 3", "🕛 0",}, timer_symbol;
