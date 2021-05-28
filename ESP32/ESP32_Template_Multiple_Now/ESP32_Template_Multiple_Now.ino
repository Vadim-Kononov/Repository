/*
ESP32_Exchange_Multiple шаблон для ESP32
Oбмен между несколькими, в данном скетче между тремя ESP32 без WiFi и роутера по технологии ESPNOW с проверкой канала связи 
Обмен по Telnet, обмен по Bluetooth, обмен по MQTT
Вывод данных на ThingSpeak, вывод данных на IFTTT, получение времени по NTP
Сохранение данных в NVS, сторожевой таймер WDT
Загрузка скетча через OTA

ROLE 0  Main_0 плата, иницирует обмен со Slave платами через ESPNOW, осуществляет обмен по MQTT, выводит данные в ThingSpeak, IFTTT
        В цикле таймера Cycle_A отправляет сообщения ESPNOW, MQTT, NTP, WDT
        --> Cycle_A --> Send_Now --TACT_time--> Receiv_Now --TACT_time--> Receiv_Now --TACT_time--> mqtt_Send --> 
        В цикле таймера timerCycle_B отправляет сообщения на ThingSpeak, переподключается к WiFi и MQTT при потере соединения
        WiFi.macAddress  24:62:AB:D4:C3:58

ROLE 1  Slave_1 плата, отвечает на сообщения Main_0 платы по ESPNOW после приема сообщения
        Receiv_Now --TACT_time--> Send_Now
        WiFi.macAddress  C4:4F:33:11:3B:A9   

ROLE 2  Slave_2 плата, отвечает на сообщения Main_0 платы по ESPNOW после приема сообщения
        Receiv_Now --TACT_time--> --TACT_time--> Send_Now
        WiFi.macAddress  A4:CF:12:24:B4:68

        Широковещательный MAC адрес для возможных вариантов:
        FF:FF:FF:FF:FF:FF

DEBUG 1 Включает вывод отладочных сообщений в Serial и (или) Telnet, Bluetooth
*/

/*>-----------< Тип платы Main_0, Slave_1, Slave_2 (0, 1, 2). Количество плат = 3>-----------<*/
#define ROLE        0
#define BOARD_COUNT 3
/*>-----------< Тип платы Main_0, Slave_1, Slave_2 (0, 1, 2). Количество плат = 3>-----------<*/

/*Имена плат*/
const char *  board_name[BOARD_COUNT] PROGMEM = {"Exch_Main_0", "Exch_Slave_1", "Exch_Slave_2"};

/*MAC адрес Main платы*/
uint8_t mac_Main_0[] =  {0x24, 0x62, 0xAB, 0xD4, 0xC3, 0x58};
/*MAC адреса Slave плат*/
uint8_t mac_Slave_1[] = {0xC4, 0x4F, 0x33, 0x11, 0x3B, 0xA9};
uint8_t mac_Slave_2[] = {0xA4, 0xCF, 0x12, 0x24, 0xB4, 0x68};
/*или широковещательный MAC адрес*/
//uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

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
bool flag_Terminal_pass = false, flag_MQTT_connected = false, flag_WiFi_connected = false, flag_Now_connected[BOARD_COUNT] = {false, false, false},
flag_Cycle_A = false, flag_Cycle_B = false, flag_OTA_pass = false, flag_ThSp_time = false, flag_IFTTT_time = false;

/*Cтруктура данных для передачи через ESPNOW*/
struct now_data
{
byte       unit;    //Номер платы
int        hash;    //Случайное число для проверки связи
};
/*Cтруктура данных для приема*/
now_data now_Get;
/*Массив структур данных для передачи и хранения для каждой платы*/
now_data now_Store[BOARD_COUNT];

/*Переменная хранящая старое значение кода проверки для Slave плат*/
int back_hash;

/*Библиотеки для ESPNOW, OTA, Ping*/
#include <esp_now.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP32Ping.h>

/*Cтруктура данных для записи информации о пирах ESPNOW*/
esp_now_peer_info_t peerInfo;

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
#define TACT_time           Cycle_A_time/(BOARD_COUNT+2)      //Время смещения выполнения функций выполняющихся в малом цикле

TimerHandle_t timerCycle_A    = xTimerCreate("timerCycle_A",    pdMS_TO_TICKS(Cycle_A_time),      pdTRUE,   (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(Cycle_A));
TimerHandle_t timerCycle_B    = xTimerCreate("timerCycle_B",    pdMS_TO_TICKS(Cycle_B_time),      pdTRUE,   (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(Cycle_B));
TimerHandle_t timerWatchDog   = xTimerCreate("timerWatchDog",   pdMS_TO_TICKS(WatchDog_time),     pdTRUE,   (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(WatchDog));

TimerHandle_t timerSendNow    = xTimerCreate("timerSendNow",    pdMS_TO_TICKS(TACT_time),         pdFALSE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(Send_Now));
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
xTimerStop(timerCycle_A,    0);
xTimerStop(timerCycle_B,    0);
xTimerStop(timerWatchDog,   0);
xTimerStop(timerSendNow,    0);
xTimerStop(timerMqttSend,   0);
xTimerStop(timerThSpTime,   0);
xTimerStop(timerIFTTTTime,  0);
DEBUG_PRINTLN(F(">-----------< TimerStop >-----------<"));
}
/**/
/*>-----------< Функции >-----------<*/
/*Вспомогательный массив для отладки, счетчик непринятых Now ответов*/
long board_count [BOARD_COUNT];
