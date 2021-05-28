/*
ESP32_Exchange
Шаблон для ESP32
Oбмен между двумя ESP32 без Wifi и роутера - ESPNOW, обмен по SerialWiFi, обмен по Serial.Bluetooth, обмен по MQTT
Вывод данных на ThingSpeak, вывод данных на IFTTT, получение времени по NTP
Сохранение данных в NVS, сторожевой таймер WDT
Загрузка скетча OTA

ROLE 1  Main плата, иницирует обмен со Slave платой через ESPNOW, обменивается данными с сервисами MQTT, ThingSpeak, IFTTT
        В цикле таймера Small_Cycle_Function отравка сообщений ESPNOW, вывод в терминал, отправка сообщений MQTT, NTP, WDT
        Small_Cycle_Function --TACT_time--> Main_Send --> Now_Receiving_Complete --TACT_time--> Main_Print --TACT_time--> mqtt_Send 
        В цикле таймера timerBig_Cycle_Function вывод данных на ThingSpeak, переподключение WiFi и MQTT
        WiFi.macAddress  C4:4F:33:11:3B:A9

ROLE 2  Slave плата, отвечает на сообщения Main платы по ESPNOW
        Now_Receiving_Complete --TACT_time--> Slave_Send
        WiFi.macAddress  A4:CF:12:24:B4:68

        Широковещательный MAC адрес
        uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

DEBUG 1 Включает вывод отладочных сообщений в Serial и (или) SerialWiFi
*/

/*Тип платы Main или Slave*/
#define ROLE 1

#if   (ROLE == 1)
const char * board_name PROGMEM = "Exch_Main";
#elif (ROLE == 2)
const char * board_name PROGMEM = "Exch_Slave";
#endif

/*Включение вывода в Serial при отладке*/
#define DEBUG 1

#if   (DEBUG == 1)
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_WRITELN(x, y) Writeln(x, y)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_WRITELN(x, y)
#endif

/*Символьные массивы для хранения логина и пароля WiFi в NVS*/
char ssid [30];
char password [30];

#include "Account.h"

/*Флаги введения пароля для Terminal, наличия подключения к MQTT*/
bool flag_Terminal_pass = false, flag_MQTT_connected = false, flag_WiFi_connected = false, flag_Small_Cycle_Function = false, flag_Big_Cycle_Function = false, flag_OTA_pass = false;

/*Cтруктура данных для передачи через ESPNOW*/
struct struct_data
{
int     var1;
int     var2;
int     var3;
int     var4;
int     var5;
int     var6;
int     var7;
};
/*Cтруктура данных для отправки*/
struct_data struct_Send;
/*Cтруктура данных для приема*/
struct_data struct_Receiv;

#if   (ROLE == 1)
/*MAC адрес Slave платы*/
//uint8_t broadcastAddress[] = {0xA4, 0xCF, 0x12, 0x24, 0xB4, 0x68};
/*или широковещательный MAC адрес*/
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#elif (ROLE == 2)
/*MAC адрес Main платы*/
//uint8_t broadcastAddress[] = {0xC4, 0x4F, 0x33, 0x11, 0x3B, 0xA9};
/*или широковещательный MAC адрес*/
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
#endif

/*Определения для ESPNOW, OTA, Ping*/
#include <esp_now.h>
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
void IRAM_ATTR ResetModule()
{DEBUG_PRINTLN(F("\n\n\nThe watchdog timerWDT went off\n\n\n"));ESP.restart();}

/*Определения для NVS памяти*/
#include <Preferences.h>
Preferences memory;

/*Определения для WDT*/
#include "esp_system.h"
hw_timer_t *timerWDT = NULL;


/*Таймеры
SMALL_CYCLE_time    время малого цикла, ESPNOW, MQTT, NTP, WDT
BIG_CYCLE_time      время большого цикла, IFTTT, переподключение WiFi, MQTT
TACT_time           время сдвига выполнения функции
*/
#define SMALL_CYCLE_time        2000
#define BIG_CYCLE_time          60000
#define TACT_time               250

TimerHandle_t timerSmall_Cycle_Function = xTimerCreate("timerSmall_Cycle_Function",   pdMS_TO_TICKS(SMALL_CYCLE_time),  pdTRUE,   (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(Small_Cycle_Function));
TimerHandle_t timerBig_Cycle_Function   = xTimerCreate("timerSmall_Big_Function",     pdMS_TO_TICKS(BIG_CYCLE_time),    pdTRUE,   (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(Big_Cycle_Function));

TimerHandle_t timerMainSend             = xTimerCreate("timerMainSend",               pdMS_TO_TICKS(TACT_time),         pdFALSE,   (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(Main_Send));
TimerHandle_t timerSlaveSend            = xTimerCreate("timerSlaveSend",              pdMS_TO_TICKS(TACT_time),         pdFALSE,   (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(Slave_Send));
TimerHandle_t timerMainPrint            = xTimerCreate("timerMainPrint",              pdMS_TO_TICKS(TACT_time),         pdFALSE,   (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(Main_Print));
TimerHandle_t timerMqttSend             = xTimerCreate("timerMqttSend",               pdMS_TO_TICKS(TACT_time),         pdFALSE,   (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(mqtt_Send));

/*Функции*/


/*Получение времени по NTP каналу*/
void GetTime()
{
if  (flag_WiFi_connected && Ping.ping("8.8.8.8", 1)) {configTime(10800, 0, "pool.ntp.org"); getLocalTime(&timeinfo);}
}
/**/

/*Остановка таймеров перед OTA загрузкой*/
void TimerStop()
{
xTimerStop(timerSmall_Cycle_Function,0);
xTimerStop(timerBig_Cycle_Function, 0);
xTimerStop(timerMainSend, 0);
xTimerStop(timerSlaveSend, 0);
xTimerStop(timerMainPrint, 0);
xTimerStop(timerMqttSend, 0);

DEBUG_PRINTLN(F("------------- TimerStop -------------"));
}
