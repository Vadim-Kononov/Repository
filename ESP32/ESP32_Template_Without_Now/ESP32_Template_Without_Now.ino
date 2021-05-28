
//Библиотека для отключения brownout detector
#include "soc/rtc_cntl_reg.h"

/*Имя платы*/
const char *  board_name PROGMEM = "PZEM";

/*>-----------< Включение вывода в Serial и (или) Telnet, Bluetooth для отладки >-----------<*/
#define DEBUG 0

#if     (DEBUG == 1)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_WRITE(x, y) Write(x, y)
#define DEBUG_WRITELN(x, y) Writeln(x, y)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_WRITE(x, y)
#define DEBUG_WRITELN(x, y)
#endif
/*>-----------< Включение вывода в Serial и (или) Telnet, Bluetooth для отладки >-----------<*/

/*Символьные массивы для хранения логина и пароля WiFi в NVS*/
char ssid [30];
char password [30];

#include "Account.h"

/*Флаги введения пароля для Terminal, подключения к MQTT, подключения к WiFi, подтверждение приема Now,
таймера малого цикла, таймера большого цикла, разрешения загрузки по OTA, таймера ожидания ответа ThingSpeak, таймера ожидания ответа IFTTT*/
bool flag_Terminal_pass = false, flag_MQTT_connected = false, flag_WiFi_connected = false,
flag_Cycle_A = false, flag_Cycle_B = false, flag_OTA_pass = false, flag_ThSp_time = false, flag_IFTTT_time = false;

/*Библиотеки для OTA, Ping*/
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP32Ping.h>

/*Определения для Bluetooth*/
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

/*Определения для Telnet*/
WiFiServer TelnetServer(59990);
WiFiClient TelnetClient;

/*Определения для MQTT*/
#include <AsyncMqttClient.h>
AsyncMqttClient mqttClient;

/*Определения для NTP*/
#include "time.h"
struct tm timeinfo;

/*Определения для NVS памяти*/
#include <Preferences.h>
Preferences memory;

/*Таймеры*/
#define Cycle_A_time        2000                              //Время малого цикла, ESPNOW, MQTT, NTP, WDT
#define Cycle_B_time        Cycle_A_time*30                   //Время большого цикла, ThingSpeak, переподключение WiFi, MQTT
#define WatchDog_time       Cycle_A_time*15                   //Время срабатывания сторожевого таймера
#define TACT_time           300                               //Время ожидания ответа сервера ThingSpeak, MQTT, IFTTTT

TimerHandle_t timerCycle_A    = xTimerCreate("timerCycle_A",    pdMS_TO_TICKS(Cycle_A_time),      pdTRUE,   (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(Cycle_A));
TimerHandle_t timerCycle_B    = xTimerCreate("timerCycle_B",    pdMS_TO_TICKS(Cycle_B_time),      pdTRUE,   (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(Cycle_B));
TimerHandle_t timerWatchDog   = xTimerCreate("timerWatchDog",   pdMS_TO_TICKS(WatchDog_time),     pdTRUE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(WatchDog));

TimerHandle_t timerMqttSend   = xTimerCreate("timerMqttSend",   pdMS_TO_TICKS(TACT_time),         pdFALSE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(mqtt_Send));
TimerHandle_t timerThSpTime   = xTimerCreate("timerThSpTime",   pdMS_TO_TICKS(TACT_time),         pdFALSE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(ThSpTime));
TimerHandle_t timerIFTTTTime  = xTimerCreate("timerIFTTTTime",  pdMS_TO_TICKS(TACT_time),         pdFALSE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(IFTTTTime));

/*>-----------< Функции >-----------<*/
/*Сторожевой таймер*/
void WatchDog() {DEBUG_PRINTLN(F("\n\n\nThe watchdog timerWDT went off\n\n\n")); ESP.restart();}
/**/

/*Получение времени по NTP каналу*/
void GetTime() {if  (flag_WiFi_connected && Ping.ping("8.8.8.8", 1)) {configTime(10800, 0, "pool.ntp.org"); getLocalTime(&timeinfo);}}
/**/

/*Остановка таймеров перед OTA загрузкой*/
void TimerStop()
{
xTimerStop(timerCycle_A, 0);
xTimerStop(timerCycle_B, 0);
xTimerStop(timerWatchDog,0);
xTimerStop(timerMqttSend, 0);
xTimerStop(timerThSpTime, 0);
xTimerStop(timerIFTTTTime,0);
DEBUG_PRINTLN(F(">-----------< TimerStop >-----------<"));
}
/**/

/*Функция малого цикла*/
void Cycle_A ()
{
/*Вывод информации о системном времени*/
DEBUG_PRINTLN(); DEBUG_PRINTLN (String(xTaskGetTickCount()/1000.0));
/*Установка флага для выполнения дительных операций в цикле Loop*/
flag_Cycle_A = true;
}
/**/

/*Функция большого цикла*/
void Cycle_B ()
{
/*Установка флага для выполнения дительных операций в цикле Loop*/
flag_Cycle_B = true;
}
/**/
/*>-----------< Функции >-----------<*/
