/*
  Встроенный светодиод подлкючён к D4 (GPIO2) пину высокий уровень гасит светодиод
  Датчик температуры ds18b20 к D5 (GPIO14) пину
  Реле к D3 (GPIO0) пину
  Термопара к :
  SCK к D6 GPIO(12) пину
  CS  к D7 GPIO(13) пину
  SO  к D8 GPIO(15) пину
  Servo к :
  Сигнал к D2 GPIO(4) пину
  Питание к D1 GPIO(5) пину
  RGB светодиод к:
  R к D4 GPIO(2) пину
  G к D0 GPIO(16) пину
  B к D10 GPIO(1) пину

  Модуль НС12 подключается к Serial: RX НС12 к TX (GPIO1)(D10) платы, TX НС12 к RX (GPIO1)(D9) платы, при загрузке скетча - отключать.
                         к Serial1:RX НС12 к D4 (GPIO2) платы.
  D0   GPIO(16)   GLed
  D1   GPIO(5)    servoVcc
  D2   GPIO(4)    servoPin
  D3   GPIO(0)    WhatchDog_pin
  D4   GPIO(2)
  D5   GPIO(14)   ds18b20
  D6   GPIO(12)   thermocoupleSCK
  D7   GPIO(13)   thermocoupleCS
  D8   GPIO(15)   thermocoupleSO
  D9   GPIO(3)
  D10  GPIO(1)

  Servo красный +7,5В, черный -7,5В, белый сигнал.
*/
#define BUFFER_SIZE 100

#include <ESP8266WiFi.h>                                      // WiFi библиотеки
#include <ESP8266mDNS.h>                                      //
#include <WiFiUdp.h>                                          //
#include <WiFiClientSecure.h>                                 //
#include <ArduinoOTA.h>                                       // Библиотека для загрузки скетча "по воздуху"

#include "Account.h"

WiFiClientSecure IFTTclient;                                  // Определение для IFTT
IFTTTMaker ifttt(KEY, IFTTclient);                            // Определение для IFTT

#include <PubSubClient.h>                                     // MQTT библиотека

WiFiClient MQTTclient;                                        // Определение для MQTT
PubSubClient client(MQTTclient, mqtt_server, mqtt_port);      // Определение для MQTT

#include <OneWire.h>                                          // Определение для DS18B20, температура воды в котле
#include <DallasTemperature.h>                                // Определение для DS18B20
#define ds18b20 14                                            // Определение для DS18B20  
OneWire oneWire(ds18b20);                                     // Определение для DS18B20
DallasTemperature thermode(&oneWire);                         // Определение для DS18B20

#include "max6675.h"                                                    // Определение для термопары, тепература отходящих газов
#define thermocoupleSO 15                                               // Определение для термопары
#define thermocoupleCS 13                                               // Определение для термопары
#define thermocoupleSCK 12                                              // Определение для термопары
MAX6675 thermocouple(thermocoupleSCK, thermocoupleCS, thermocoupleSO);  // Определение для термопары

#include <Servo.h>                                                      // Определение для сервопривода
#define servoPin 4                                                      // Определение для сервопривода
#define servoVcc 5                                                      // Определение для сервопривода
Servo myServo;                                                          // Определение для сервопривода
#define WhatchDog_pin 0                                                 // Определение для пина выхода на WhatchDog
#define GLed 16                                                         // Определение для RGB светодиода

// Глобальные переменные
// Температура в комнате, ее старое значение, заданная, разница с заданной, прогноз разницы, краткосрочный прогноз, скорость изменения, прогноз за цикл регулирования, воды в котле, отходящих газов, положение серво в виде десятичного числа
float tem_room, tem_room_old, tem_zad, tem_dif, tem_big_trend, tem_small_trend, tem_boiler, tem_pipe, return_position_Sit630, prognos;
// Счетчик миллисекунд отправки сообщений, периода регулировки, последнего вращения Sit630, краткосрочного измерения и др.
long  count_ms_Send, count_ms_Control, count_ms_Rotation , count_ms_Check, count_ms_scan_Room;
// Флаг записи старой температуры, изменения состояния whatchDog
boolean tem_room_old_flag, change_clock_flag, whatchDog_flag;
// Символьное значение вращения Sit630 и др.
String control_string, rotation_string, situation_string;
// Положение Sit630, старое положение, изменение положения, режим регулятора, время отправки сообщений, краткосрочного измерения, регулировки и др.
int rotation, rotation_old, rotation_dif, regulator_mode, time_sec_Send, time_min_Check, time_min_Control, time_min_scan_Room, minutes_in_Day;

#include <EEPROM.h>                                                     // Библиотека для работы с EEPROM

// Процедура SETUP
void setup()
{
  Serial.begin(9600);                                                   // Скорость работы последовательного порта
  WiFi.mode(WIFI_STA);                                                  // Режим WiFi только STA
  WiFi.begin(ssid, pass);                                               // Подключение к WiFi
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {delay(50);}      // Ожидание WiFi 
  WiFi.hostname("NodeMCU_Boiler");
  ArduinoOTA.setHostname("NodeMCU_Boiler");                             // Определение имени устройства в сети
  ArduinoOTA.begin();                                                   // Запуск подпрограммы загрузки скетча "по воздуху"

  EEPROM.begin(512);                                      // Определение адреса чтения из EEPROM
  EEPROM.get(0, rotation_old);                            // Перенос из EEPROM в переменную последнего до перезагрузки положения сервопривода
  EEPROM.commit();                                        // Завершение работы с EEPROM
  EEPROM.end();                                           // Завершение работы с EEPROM

  myServo.attach(servoPin);                               // Определение пина управления сервопривода
  pinMode(servoVcc, OUTPUT);                              // Определение пина питания сервопривода
  digitalWrite(servoVcc, LOW);                            // Отключение питания сервопривода

  pinMode(GLed, OUTPUT);                                  // Определение пина зеленого светодиода
  pinMode(WhatchDog_pin, OUTPUT);                         // Определение пина выхода сигнала на WhatchDog

  thermode.begin();                                       // Запуск термопары

// Инициализация переменных
  change_clock_flag = false;                              // Изменения времени нет
  tem_room_old_flag = true;                               // Старого значения нет

  regulator_mode = 1;                                     // Режим регулятора ручной
  control_string = " 🌀";                                 // Строка для IFTTT сообщения
  situation_string = " 🌀 ";                              // Строка для отправки в панель
  
  minutes_in_Day = 0;                                     // Полночь
  time_sec_Send = 2;                                      // Время цикла отправки сообщений 2 в секундах
  time_min_Check = 10;                                    // Время цикла краткосрочных измерений в минутах
  time_min_Control = 30;                                  // Время цикла регулировки котла в минутах
  time_min_scan_Room = 60;                                // Время проверки модуля Room в минутах
  
  count_ms_Check = millis();                              // счетчик миллисекунд проверки температуры
  count_ms_Control    = count_ms_Check;                   // счетчик миллисекунд регулировки котла
  count_ms_Send       = count_ms_Check;                   // счетчик миллисекунд отправки сообщений
  count_ms_Rotation   = count_ms_Check;                   // счетчик миллисекунд между вращениями
  count_ms_scan_Room  = count_ms_Check;                   // счетчик миллисекунд проверки модуля Room

  delay (500);

  ifttt.triggerEvent(EVENT_NAME_1, "Перезагружен:\n" + ESP.getResetReason());// Отправка оповещения через IFTT о перезагрузке модуля
}



// Процедура LOOP
void loop()
{
  whatchDog_flag = true; digitalWrite (WhatchDog_pin, whatchDog_flag); delay (50);    // Изменение состояния пина WhatchDog

  ArduinoOTA.handle();                                                                // Процедура загрузки скетча по WiFi
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(ssid, pass);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {return;}
    WiFi.hostname("NodeMCU_Boiler");
  }
                                                                                      
  if (WiFi.status() == WL_CONNECTED)                                                  // Подключение к MQTT серверу
  { 
    delay (50);
    if (!client.connected())
    {
      if (client.connect(MQTT::Connect("Mqtt_Boiler").set_auth(mqtt_user, mqtt_pass)))
      {
      client.set_callback(reception);                                                // Определение процедуры чтения сообщений
      
      client.subscribe("rotation");                                                  // Подписка на положение Sit630
      client.subscribe("panel/mode");                                                // Подписка на режим работы регулятора 0 - ручной, 1 - автоматический
      client.subscribe("panel/time_control");                                        // Подписка на время цикла работы регулятора
      client.subscribe("panel/time_check");                                          // Подписка на время цикла измерения тренда
      client.subscribe("room/tem");                                                  // Подписка на температуру в комнате
      client.subscribe("room/zad");                                                  // Подписка на заданную температуру
      client.subscribe("room/dif");                                                  // Подписка на разность температуры в комнате и заданной
      client.subscribe("room/minutes_in_day");                                       // Подписка на минуты в сутках
      }
    }
    if (client.connected())                                                          // Если есть подключение к WiFi и к Mqtt серверу
    {
      client.loop();

      if (change_clock_flag)                                                      // Если изменилось время
      {
      change_clock_flag = false;                                                  // Сброс флага изменения времени
      count_ms_scan_Room = millis();                                              // Сброс счетчика миллисекунд контроля модуля Room
          
          if (minutes_in_Day % time_min_Check == 0)                               // Если пришло время короткого измерения
          {
          count_ms_Check = millis();                                              // Обнуляем счетчик миллисекунд короткого измерения
          tem_small_trend = tem_room - tem_room_old;                              // Вычисление малого тренда
          tem_room_old = tem_room;                                                // Сохранение значения температуры в коротком цикле                                                                  
          tem_big_trend = tem_small_trend * time_min_Control / time_min_Check;    // Вычисление тренда за цикл управления           
          if (Situation () == 0) situation_string = " 🔵 "; else situation_string = " 🔴 "; // Символ в панель
                                           
              if (minutes_in_Day % time_min_Control == 0)                                  // Если подошло время работы регулятора температуры
              {       
              count_ms_Control = count_ms_Check;                                           // Обнуление счетчика миллисекунд цикла управления
                
                if ((regulator_mode==1) || (regulator_mode==2))                                                     // Если режим автоматический
                {
                    if      (Situation () < 0)                                             // Если ситуация требует уменьшения температуры котла
                  {
                    if (rotation >= 5) {rotation = rotation - 4;  control_string = " 🔻 -1";}    // Ограничение по минимальной позиции и формирование строки для IFTT сообщения
                    else control_string = " ➖";                                                 // Формирование строки для IFTT сообщения о достигнутом пределе
                    client.publish(MQTT::Publish("rotation", String(rotation)).set_retain());    // Вызов подпрограммы поворота через публикацию   
                  }

                  else if  ((Situation () > 0))                                            // Если ситуация требует увеличения температуры котла
                  {
                    if (rotation <= 17) {rotation = rotation + 4; control_string = " 🔺 +1";}     // Ограничение по максимальной позиции и формирование строки для IFTT сообщения
                    else control_string = " ➕";                                                  // Формирование строки для IFTT сообщения о достигнутом пределе
                    client.publish(MQTT::Publish("rotation", String(rotation)).set_retain());     // Вызов подпрограммы поворота через публикацию   
                  }

                  else control_string = " 🔵";                                              // Если ситуация не требует регулировки, формирование строки для IFTT сообщения                                                                  
                }
               }
          }  
      }  
      
    if (millis() - count_ms_scan_Room > 60000 * time_min_scan_Room)                                               // Если в течении 60 мин время не изменилось
    {
    count_ms_scan_Room = millis();                                                                                // Сброс счетчика контроля модуля Room
    ifttt.triggerEvent(EVENT_NAME_1, "Модуль Room нет сигнала");                                                  // Отправка оповещения через IFTT о зависшем модуле Room
    }    
    Sending();                                                                                                    // Вызов подпрограммы отправки сообщений и измерений каждые 2 секунды
    }  
  }
 
  whatchDog_flag = false; digitalWrite (WhatchDog_pin, whatchDog_flag); delay (50);                               // Изменение состояния пина WhatchDog
}                               
// Конец процедуры LOOP



// Функция получения данных от сервера
void reception(const MQTT::Publish& pub)
{
  String payload = pub.payload_string();

  if (String(pub.topic()) == "rotation")                // Если опубликовано положение Sit630
  {
    rotation =  payload.toInt(); Position (rotation);   // Вызов поворота при публикации
  }

  if (String(pub.topic()) == "panel/mode")              // Если опубликован режим работы регулятора
  {
    regulator_mode =  payload.toInt();
  }

  if (String(pub.topic()) == "panel/time_control")      // Если опубликовано время цикла регулятора
  {
    time_min_Control =  payload.toInt();                // Сохранение его в переменной   
  }

  if (String(pub.topic()) == "panel/time_check")        // Если опубликовано время цикла измерения
  {
    time_min_Check =  payload.toInt();                  // Сохранение его в переменной   
  }
  
  if (String(pub.topic()) == "room/tem")                // Если опубликована температура в комнате
  {
    char x[10];
    payload.toCharArray(x, payload.length() + 1);
    tem_room = atof(x);
    
    if (tem_room_old_flag)
    {                                                   // Срабатывает один раз после перезагрузки
      tem_room_old_flag = false;                        // 
      tem_room_old = tem_room;                          // Приравнивается старая и текущая температура один раз после перезагрузки
    }
  }

  if (String(pub.topic()) == "room/zad")                // Если опубликована заданная температура в комнате
  {
    char x[10];
    payload.toCharArray(x, payload.length() + 1);
    tem_zad = atof(x);
  }

  if (String(pub.topic()) == "room/dif")                // Если опубликована разность текущей и заданной температур
  {
    char x[10];
    payload.toCharArray(x, payload.length() + 1);
    tem_dif = atof(x);
  }

  if (String(pub.topic()) == "room/minutes_in_day")     // Если опубликованы минуты дня
  {
    if (payload.toInt() != minutes_in_Day)              // Если минуты дня изменились
    {
      minutes_in_Day =  payload.toInt();                // Их сохранение в переменной                   
      change_clock_flag = true;                         // Установка флага изменения времени            
    }
  }
}



// Функция отправки данных
void Sending()
{
  if (millis() - count_ms_Send > 1000 * time_sec_Send)                    // Если подоошло время публикования (2 сек.)
  {
    count_ms_Send = millis();                                             // Сброс счетчика миллисекунд
    digitalWrite(GLed, HIGH);                                             // Зажигаем зеленый светодиод

    thermode.requestTemperatures();                                       // Запускаем измерене температуры воды датчиком ds18b20
    tem_boiler = thermode.getTempCByIndex(0);                             // Записываем значение температуры от датчика ds18b20
    tem_pipe = thermocouple.readCelsius() - 1.0;                          // Записываем температуру отходящих газов от термопары
    
    prognos = tem_room + tem_small_trend * (time_min_Control - minutes_in_Day % time_min_Control) / time_min_Check; // Вычисление прогноза до конца текущего цикла управления
        
    if (rotation_dif > 0) rotation_string = "+" + String(rotation_dif/4.0);               // Определение строки для отправки шага регулятора
    else if (rotation_dif < 0) rotation_string = String(rotation_dif/4.0);
    else rotation_string = "";
        
    //〇⛒⧲⎋⊠☒❌◌🄬❂🔺🔻🌀🔵🔴                                                             // Набор отображаемых в телефоне символов про запас
    
    client.publish("boiler/tem", String(tem_boiler));                                     // Публикация температуры воды в котле
    client.publish("boiler/pipe", String(tem_pipe));                                      // Публикация температуру отходящих газов
    client.publish("boiler/trend", String(tem_big_trend));                                // Публикация большого тренда
    client.publish("boiler/prognos", String(prognos));                                    // Публикация прогноза

    client.publish("boiler/elapsed_adj_ms", String(millis() - count_ms_Control));                               // Публикация времени от последней регулировки для диаграммы
    client.publish("boiler/remained_adj_ms", String(60000 * time_min_Control - (millis() - count_ms_Control))); // Публикация оставшегося времени до следующей регулировки для диаграммы
    
    client.publish("boiler/elapsed_check_ms", String(millis() - count_ms_Check));                               // Публикация времени от последнего измерения для диаграммы
    client.publish("boiler/remained_check_ms", String(60000 * time_min_Check - (millis() - count_ms_Check)));   // Публикация оставшегося времени до последнего измерения для диаграммы
    
    client.publish("boiler/time_rotation", rotation_string + situation_string + Hour_display (millis() - count_ms_Rotation));      // Публикация последнего шага и времени от последнего вращения
    client.publish("boiler/return_position_Sit630", String(return_position_Sit630));                            // Публикация текущего положения Sit630
        
    digitalWrite(GLed, LOW);                                                                                    // Гасим зеленый светодиод
  }
  delay(50);                                                                                                    // Делаем паузу для работы WiFi ESP8266
}



// Функция поворота Servo
void Position (int state)
{
  // Все позиции в мс, шаг - четверть, 7,5 град.
  int microsecond[24] = {2359, 1710, 1658, 1600, 1548, 1490, 1433, 1375, 1321, 1267, 1202, 1151, 1092, 1039, 973, 927, 873, 818, 764, 710, 661, 611, 556, 500}; // все позиции в мс, шаг - четверть, 7,5 град.

  if (state > 21) state = 21;                         // Ограничение значения сверху
  if (state < 1) state = 1;                           // Ограничение значения снизу

  rotation_dif = state - rotation_old;                // Вычисление изменения
  if (rotation_dif != 0) count_ms_Rotation = millis();// Сброс счетчика миллисекунд при изменении
  rotation_old = state;                               // Сохранение положения
  
  EEPROM.begin(512);                                  // Сохраняеем в EEPROM положение
  EEPROM.put(0, state);                               //
  EEPROM.end();
  
  myServo.writeMicroseconds(microsecond[state]);      // Поворачиваем Servo на нужный угол
  delay (500);                                        // Задержка
  digitalWrite(servoVcc, HIGH);                       // Подача питания на Servo
  ESP.wdtFeed();                                      // Сбрасываем сторожевой таймер
  delay (6000);                                       // Задержка 6 сек.
  ESP.wdtFeed();                                      // Сбрасываем сторожевой таймер
  digitalWrite(servoVcc, LOW);                        // Снятие питания с Servo

  return_position_Sit630 = state / 4.0 + 0.75;        // Преобразование положения в десятичное число
  ifttt.triggerEvent(EVENT_NAME_2, String(return_position_Sit630), control_string); // Публикация положения и режима в котором оно изменилось в сервисе IFTT для получения оповещения на телефон
  control_string = " 🖐";                             // Строковое значение для изменения положения без участия регулятора температуры
}


// Определение условий для выполнения регулировки мощности котла
int Situation ()
{
if      ((tem_dif > 0.1 && tem_small_trend >= 0.02) || (tem_dif > 0.25 && tem_small_trend >= 0) || (prognos > tem_zad + 1.0 && tem_small_trend >= 0)) return -1;
else if ((tem_dif < -0.1 && tem_small_trend <= -0.02) || (tem_dif < -0.25 && tem_small_trend <= 0) || (prognos < tem_zad - 1.0 && tem_small_trend <= 0)) return 1;
else return 0;
}



// Функция преобразования времени в строку с двоеточием
// Для перевода миллисекунд в чч:мм:сек
String Hour_display (long time_msec)
{
  int hour, minute, second;
  String st_hour, st_minute, st_second;

  hour = time_msec / 3600000;
  minute = (time_msec % 3600000) / 60000;
  second = ((time_msec % 3600000) % 60000) / 1000;

  if (hour >= 10 && hour <= 99) st_hour = String (hour) + ":"; else if (hour > 0 && hour <= 9) st_hour = "0" + String (hour) + ":"; else st_hour = String ("");
  if (minute >= 10) st_minute = String (minute); else st_minute = "0" + String (minute);
  if (second >= 10) st_second = String (second); else st_second = "0" + String (second);

  return st_hour + st_minute + ":" + st_second;
}
