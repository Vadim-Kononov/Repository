/*
Страница определений
*/

// Имя модуля
#define NAME "ESP32_Key"

#define LED_ONBOARD   2
#define RECEIVER_PIN  18
#define RELAY_PIN     27
#define LED_RED       19
#define LED_GREEN     23
#define LED_BLUE      5

//Библиотека для отключения brownout detector
#include "soc/rtc_cntl_reg.h"

//Определения для Bluetoot
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

//Определения для WDT
#include "esp_system.h"
//Таймаут WDT, мс
const int wdtTimeout = 60000;
hw_timer_t *timer = NULL;
//Прграмма обработки прерывания WDT
void IRAM_ATTR ResetModule()
{
ESP.restart();
}

//Определения для записи-чтения EEPROM
#include <Preferences.h>
Preferences memory;

#include <RCSwitch.h>
RCSwitch receiver = RCSwitch();

//Структура для датчиков сигнализации номер, тип, код, описание
struct detector {uint8_t number; uint32_t code; char title [50];};

//Структура для хранения в EEPROM
detector detect_mem [16];
//Структура для хранения в ОЗУ
detector detect_buf [16];
//Соответствующие глобальные переменные
uint8_t number;
uint32_t code;
String title;

//Таймер приема кода
long count_ms_Code, time_ms_Code, count_ms_Led, time_ms_Led;

//Флаги
bool key_flag = false, code_flag = false, dev_flag = false, relay_flag = false;
