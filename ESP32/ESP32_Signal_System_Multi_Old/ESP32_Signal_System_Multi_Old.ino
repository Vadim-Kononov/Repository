/*
ESP32_Signal_Sysytem
на шаблоне ESP32_Exchange_Multiple
Oбмен между несколькими, в данном скетче между тремя ESP32 без WiFi и роутера по технологии ESPNOW с проверкой канала связи 
Обмен по Telnet, обмен по Bluetooth, обмен по MQTT
Вывод данных на ThingSpeak, вывод данных на IFTTT, получение времени по NTP
Сохранение данных в NVS, сторожевой таймер WDT
Загрузка скетча через OTA

ROLE 0  Main_0 плата, иницирует обмен со Slave платами через ESPNOW, осуществляет обмен по MQTT, выводит данные в ThingSpeak, IFTTT
        В цикле таймера Small_Cycle отправляет сообщения ESPNOW, MQTT, NTP, WDT
        --> Small_Cycle --> Send_Now --TACT_time--> Receiv_Now --TACT_time--> Receiv_Now --TACT_time--> mqtt_Send --> 
        В цикле таймера timerBig_Cycle отправляет сообщения на ThingSpeak, переподключается к WiFi и MQTT при потере соединения
        WiFi.macAddress  A4:CF:12:44:E8:1C

ROLE 1  Slave_1 плата, отвечает на сообщения Main_0 платы по ESPNOW после приема сообщения
        Receiv_Now --TACT_time--> Send_Now
        WiFi.macAddress  C4:4F:33:11:3B:A9      

ROLE 2  Slave_2 плата, отвечает на сообщения Main_0 платы по ESPNOW после приема сообщения
        Receiv_Now --TACT_time--> --TACT_time--> Send_Now
        WiFi.macAddress   A4:CF:12:24:B4:68

        Широковещательный MAC адрес для возможных вариантов:
        FF:FF:FF:FF:FF:FF

DEBUG 1 Включает вывод отладочных сообщений в Serial и (или) Telnet, Bluetooth
*/

/*>-----------< Тип платы Main_0, Slave_1, Slave_2 (0, 1, 2). Количество плат = 3>-----------<*/
#define ROLE        0
#define BOARD_COUNT 3
/*>-----------< Тип платы Main_0, Slave_1, Slave_2 (0, 1, 2). Количество плат = 3>-----------<*/

/*Имена плат*/
const char *  board_name[BOARD_COUNT] PROGMEM = {"Signal_System", "Exch_Slave_1", "Exch_Slave_2"};

/*MAC адреса Slave плат*/

uint8_t mac_Slave_1[] = {0xC4, 0x4F, 0x33, 0x11, 0x3B, 0xA9};
uint8_t mac_Slave_2[] = {0xA4, 0xCF, 0x12, 0x24, 0xB4, 0x68};


/*>-----------< Включение вывода в Serial и (или) Telnet, Bluetooth для отладки >-----------<*/
#define DEBUG 1

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

#include "Account.h"

/*Флаги введения пароля для Terminal, подключения к MQTT, подключения к WiFi, подтверждение приема Now, таймера малого цикла, таймера большого цикла, разрешения загрузки по OTA*/
bool flag_Terminal_pass = false, flag_MQTT_connected = false, flag_WiFi_connected = false, flag_Now_connected[BOARD_COUNT] = {false, false, false}, flag_Small_Cycle = false, flag_Big_Cycle = false, flag_OTA_pass = false;

/*Cтруктура данных для передачи через ESPNOW*/
struct now_data
{
byte       unit;    //Номер платы
int        hash;    //Случайное число для проверки связи
};
/*Cтруктура данных для приема*/
now_data now_Get;
/*Массив структур данных для передачи и хранения*/
now_data now_Store[BOARD_COUNT];


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

/*Определения для WDT*/
#include "esp_system.h"
hw_timer_t *timerWDT = NULL;
void IRAM_ATTR ResetModule()
{DEBUG_PRINTLN(F("\n\n\nThe watchdog timerWDT went off\n\n\n"));ESP.restart();}

/*Таймеры*/
#define SMALL_CYCLE_time        2000    //Время малого цикла, ESPNOW, MQTT, NTP, WDT
#define BIG_CYCLE_time          60000   //Время большого цикла, ThingSpeak, переподключение WiFi, MQTT
#define TACT_time               300     //Время смещения выполнения функции
  
#define AlarmRelayPin_time    1000
#define ResetCode_time        2000
#define CheckeSim800_time     2000
#define LongTime_time         3600000


TimerHandle_t timerSmall_Cycle    = xTimerCreate("timerSmall_Cycle",    pdMS_TO_TICKS(SMALL_CYCLE_time),              pdTRUE,   (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(Small_Cycle));
TimerHandle_t timerBig_Cycle      = xTimerCreate("timerBig_Cycle",      pdMS_TO_TICKS(BIG_CYCLE_time),                pdTRUE,   (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(Big_Cycle));

TimerHandle_t timerSendNow        = xTimerCreate("timerSendNow",        pdMS_TO_TICKS(TACT_time),                     pdFALSE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(Send_Now));
TimerHandle_t timerMqttSend       = xTimerCreate("timerMqttSend",       pdMS_TO_TICKS(TACT_time * (BOARD_COUNT)),     pdFALSE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(mqtt_Send));

TimerHandle_t timerResetCode                     = xTimerCreate("timerResetCode",            pdMS_TO_TICKS(ResetCode_time),                   pdFALSE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(ResetCode));
TimerHandle_t timerCheckeSim800                  = xTimerCreate("timerCheckeSim800",         pdMS_TO_TICKS(CheckeSim800_time),                pdFALSE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(CheckeSim800));
TimerHandle_t timerAlarmLongTime                 = xTimerCreate("timerAlarmLongTime",        pdMS_TO_TICKS(LongTime_time),                    pdFALSE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(AlarmRelayOff));

TimerHandle_t timerRelayOff_1                    = xTimerCreate("timerRelayOff_1",           pdMS_TO_TICKS(AlarmRelayPin_time),               pdFALSE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(AlarmRelayOff));
TimerHandle_t timerRelayOff_2                    = xTimerCreate("timerRelayOff_2",           pdMS_TO_TICKS(AlarmRelayPin_time),               pdFALSE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(AlarmRelayOff));
TimerHandle_t timerRelayOff_3                    = xTimerCreate("timerRelayOff_3",           pdMS_TO_TICKS(AlarmRelayPin_time),               pdFALSE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(AlarmRelayOff));
TimerHandle_t timerRelayOff_4                    = xTimerCreate("timerRelayOff_4",           pdMS_TO_TICKS(AlarmRelayPin_time),               pdFALSE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(AlarmRelayOff));
TimerHandle_t timerRelayOn_1                     = xTimerCreate("timerRelayOn_1",            pdMS_TO_TICKS(AlarmRelayPin_time),               pdFALSE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(AlarmRelayOn_1));
TimerHandle_t timerRelayOn_2                     = xTimerCreate("timerRelayOn_2",            pdMS_TO_TICKS(AlarmRelayPin_time),               pdFALSE,  (void*)0,   reinterpret_cast<TimerCallbackFunction_t>(AlarmRelayOn_2));

/*>-----------< Sygnal System >-----------<*/
/*Определения для INA219*/
#include <Wire.h>
#include <Adafruit_INA219.h>
Adafruit_INA219 ina219;

/*Определения GPIO для SIM800*/
#define RXD2 25
#define TXD2 27
#define SIM800_PWR 5

//Переменная для ответа SIM800 и номераомера телефонов 
String respond_sim800, my_number = "+79198897600", boris_number = "+79094008600", gleb_number = "+78635294929";

//Структура для датчиков сигнализации номер, тип, код, описание
struct detector
{
uint8_t     number;
uint8_t     type;
uint32_t    code;
char        title [50];
};

//Структура для хранения в ОЗУ
detector detector_ram [35];

//Соответствующие глобальные переменные
uint8_t number, type;
uint32_t code;

String title, string_check_random, detector_title;


//Флаги изменения кода, разрешения записи в EEPROM из эфира, флаг включение сигнализации, флаг питания от сети, флаг длительного включения сирены, флаг срабатывания сигнализации
bool flag_code_receive = false, flag_code_save = false, flag_activation_alarm = false, flag_perimeter_alarm = false, flag_power_220 = false , flag_alarm_triggered = false, flag_BigCycleFunction = false ;

//Определения для приемника 433 Мгц
#define RECEIVER_PIN 18
#include <RCSwitch.h>
RCSwitch receiver = RCSwitch();

//Определение пина питания от сети, пинов реле тревоги
#define POWER_PIN 26
#define ALARM_RELAY_PIN_1 19
#define ALARM_RELAY_PIN_2 23

/*>-----------< Sygnal System >-----------<*/


/*>-----------< Функции >-----------<*/
/*Получение времени по NTP каналу*/
void GetTime()
{
if  (flag_WiFi_connected && Ping.ping("8.8.8.8", 1)) {configTime(10800, 0, "pool.ntp.org"); getLocalTime(&timeinfo);}
}
/**/

/*Остановка таймеров перед OTA загрузкой*/
void TimerStop()
{
xTimerStop(timerSmall_Cycle,0);
xTimerStop(timerBig_Cycle, 0);
xTimerStop(timerSendNow, 0);
xTimerStop(timerMqttSend, 0);

DEBUG_PRINTLN(F(">-----------< TimerStop >-----------<"));
}
/**/
/*>-----------< Функции >-----------<*/
