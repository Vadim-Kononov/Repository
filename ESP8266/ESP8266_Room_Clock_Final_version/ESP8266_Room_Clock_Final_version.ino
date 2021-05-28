/* 
Дисплей SH1106 подключен:
SCL  к D1 GPIO(5)
SDA  к D2 GPIO(4)

SCL BMP280 и Si7021 подключены к D1 GPIO(5)
SDA BMP280 и Si7021 подключены к D2 GPIO(4)

Встроенный светодиод D4 GPIO(2)


Библиотека U8g2, параметры от другого типа дисплея SSD1306, т.к. к SH1106 не нашлось подходящих

I2C device found at address 0x3D  ! Oled Дисплей
I2C device found at address 0x57  ! DS3231 Flash
I2C device found at address 0x68  ! DS3231 Clock 
I2C device found at address 0x76  ! BME 280

*/
#define BUFFER_SIZE 100
#define Onboard_Led 2

#include <ESP8266WiFi.h>                                      // Основная библиотека               
#include <ESP8266mDNS.h>                                      // Библиотека для работы с именами хостов
#include <WiFiUdp.h>                                          // Библиотека для работы с UDP
#include <ArduinoOTA.h>                                       // Библиотека прошивки через WiFi
#include <EEPROM.h>                                           // Библиотека для работы с EEPROM

#include "Account.h"

#include <IFTTTMaker.h>                                       // Определения для IFTTT

WiFiClientSecure IFTTclient;                                  // Определения для IFTTT
IFTTTMaker ifttt(KEY, IFTTclient);                            // Определения для IFTTT

#include "PubSubClient.h"                                     // Определения для работы с MQTT сервером

WiFiClient MQTTclient;                                        // Определения для работы с MQTT сервером
PubSubClient client(MQTTclient, mqtt_server, mqtt_port);      // Определения для работы с MQTT сервером

#include <Wire.h>                                             // Определения для работы с часами
#include "RTClib.h"                                           // Определения для работы с часами
RTC_DS3231 DS3231;

#include <BME280I2C.h>                                        // Определения для работы метеорологическим датчиком
BME280I2C bme;                                                // Определения для работы метеорологическим датчиком

#include <Arduino.h>                                                          // Определения для работы с OLED экраном
#include <U8g2lib.h>                                                          // Определения для работы с OLED экраном
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);      // Определения для работы с OLED экраном

#include "ThingSpeak.h"                                                       // Определение для сервиса 

WiFiClient ThingSpeakclient;                                                  // Определение для сервиса ThingSpeak


// Температура в комнате, температура с датчика, температура заданная, температура во дворе, давление в комнате, давление во дворе, влажность в комнате, влажность во дворе
// Температура в котле, температура в газоходе, поправка к температуре в комнате, положение Sit630
float tem_room, tem_BME280, tem_set, tem_out, pres_room, pres_out, hum_room, hum_out, tem_boiler, tem_pipe, correction, return_position_Sit630;

// Счетчик мс интервала отправки сообщений на сервер, заданное время отправки в мс
long count_ms_Send, time_ms_Send;                                                                                

// Счетчик циклов 30*2*1 сек для ThingSpeak, счетчик попыток подключения к WiFi, счетчик попыток подключения к MQTT, часы, минуты, секунды
int  cycle_Counter, count_wifi, count_mqtt, hour_clock, minute_clock, second_clock;

// Номер цикла, номер канала (0 или 1)
bool cycle_number, channel_number;                                                                                   // Состояние термостата и имя цикла                        



void setup()
{
  Serial.begin(9600);                                       // Скорость работы последовательного порта
  WiFi.mode(WIFI_STA);                                      // Режим WiFi только STA
  WiFi.begin(ssid, pass);                                   // Подключение к WiFi
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {ESP.wdtFeed(); delay(50); count_wifi--; if (count_wifi <=0) ESP.restart();}
  count_wifi = 50;
  WiFi.hostname("WeMos_Room");
  ArduinoOTA.setHostname("WeMos_Room");                     // Определение имени устройства в сети
  ArduinoOTA.begin();                                       // Запуск подпрограммы загрузки скетча "по воздуху"
  
  // Корректировка времени
  //DateTime corr = DS3231.now();
  //DS3231.adjust (corr + TimeSpan(0,0,0,0));
  
  DS3231.begin();                                           // Инициализация часов
  bme.begin();                                              // Инициализация BME280
  ThingSpeak.begin(ThingSpeakclient);                       // Инициализация ThingSpeak
 
  ESP.wdtFeed();                                            // Сброс сторожевого таймера
  EEPROM.begin(512);                                        // Чтение из EEPROM сохраненной корректировки
  EEPROM.get(0, correction);
  EEPROM.commit();
  EEPROM.end();

  count_ms_Send = millis();                                 // Счетчик мс отправки сообщений - сброс 
  time_ms_Send = 1000;                                      // Период отправки сообщений 1 сек.
  cycle_Counter = 30;                                       // Счетчик циклов 30
  count_wifi = 50;                                          // Счетчик неудачных подключений 50
  count_mqtt = 50;                                          // Счетчик неудачных подключений 50
  
  cycle_number = false;                                     // Номер цикла 0
  channel_number = true;                                    // Номер канала 1        
  
  u8g2.begin();                                             // Инициализация дисплея  
  u8g2.enableUTF8Print(); 
  
  pinMode(Onboard_Led, OUTPUT);
  
  delay (500);
  ifttt.triggerEvent(EVENT_NAME, "Перезагружен:\n" + ESP.getResetReason()); // Отправка сообщения о перезагрузке
}



void loop() 
{
  ArduinoOTA.handle();                                                      // Процедура загрузки скетча по WiFi
                                                                                      
  if (WiFi.status() != WL_CONNECTED)                                        // Подключение к WiFi       
    {
    WiFi.begin(ssid, pass);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)  {ESP.wdtFeed(); count_wifi--; if (count_wifi<=0) ESP.restart(); else return;}
    count_wifi = 50;
    WiFi.hostname("WeMos_Room");
    }
                                                 
  if (WiFi.status() == WL_CONNECTED)                                          
  { 
    if (!client.connected())
    {
        if (client.connect(MQTT::Connect("Mqtt_Room").set_auth(mqtt_user, mqtt_pass))) // Подключение к MQTT серверу  
        {
        count_mqtt = 50;

        client.set_callback(reception);                                               // Определение процедуры чтения сообщений
        
        client.subscribe("panel/zad");                                                // Подписка на топики
        client.subscribe("room/dec");
        client.subscribe("room/inc");
        
        client.subscribe("outdoor/tem");
        client.subscribe("outdoor/pres");
        client.subscribe("outdoor/hum");

        client.subscribe("boiler/tem");
        client.subscribe("boiler/pipe");
        client.subscribe("boiler/return_position_Sit630");  
        } 
        else {ESP.wdtFeed(); delay (100); count_mqtt--; if (count_mqtt<=0) ESP.restart();}
    }
    if (client.connected()){client.loop();Sending();}                                 // Если подключение не прервалось вызов отправки показаний
  }
 
  DateTime now = DS3231.now();                                                        // Чтение времени 
  hour_clock = now.hour();
  minute_clock = now.minute();
  second_clock = now.second();
  
  
  u8g2.firstPage();                                                                   // Вывод информации на дисплей
  do
    {
    u8g2.setFont(u8g2_font_profont22_tf);
    u8g2.setCursor(3, 15);
    u8g2.print(tem_room); 
    u8g2.setFont(u8g2_font_profont11_tf);
    u8g2.setCursor(7, 30);
    u8g2.print(String(int(pres_room)) + " - " + String(int(hum_room)));     
    u8g2.setCursor(7, 45);
    u8g2.print(Clock (hour_clock) + ":" + Clock (minute_clock) + ":" + Clock (second_clock));
    } 
    while (u8g2.nextPage());
}
// конец void loop



// Функция получения данных от сервера
void reception(const MQTT::Publish& pub)
{
  String payload = pub.payload_string();         // Получение значения                 
 
  if(String(pub.topic()) == "panel/zad")         // Если заданная температура
  {
  char x[10];                                    // Преобразование строки в float
  payload.toCharArray(x,payload.length()+1);
  tem_set = atof(x);                            
  }
  
  if(String(pub.topic()) == "room/dec")          // Если корректировка -0,1
  {
  if (payload.toInt()) correction = correction - 0.1 ;
  EEPROM.begin(512);                             // Сохраняем значение в памяти
  EEPROM.put(0, correction);
  EEPROM.end();
  }
  
  if(String(pub.topic()) == "room/inc")         // Если корректировка +0,1
  {
  if (payload.toInt()) correction = correction + 0.1 ;                     
  EEPROM.begin(512);                            // Сохраняем значение в памяти
  EEPROM.put(0, correction);
  EEPROM.end();
  }
  
  if(String(pub.topic()) == "outdoor/tem")      // Если температура во дворе
  {
  char x[10];                                   // Преобразование строки в float
  payload.toCharArray(x,payload.length()+1);
  tem_out = atof(x);                            
  }

  if(String(pub.topic()) == "outdoor/pres")     // Если давление во дворе
  {
  char x[10];                                   // Преобразование строки в float
  payload.toCharArray(x,payload.length()+1);
  pres_out = atof(x);                           
  }

  if(String(pub.topic()) == "outdoor/hum")      // Если влажность во дворе
  {
  char x[10];                                   // Преобразование строки в float
  payload.toCharArray(x,payload.length()+1);
  hum_out = atof(x);                            
  }

  if(String(pub.topic()) == "boiler/tem")       // Если температура в котле
  {
  char x[10];                                   // Преобразование строки в float
  payload.toCharArray(x,payload.length()+1);
  tem_boiler = atof(x);                            
  }

  if(String(pub.topic()) == "boiler/pipe")      // Если температура в газоходе
  {
  char x[10];                                   // Преобразование строки в float
  payload.toCharArray(x,payload.length()+1);
  tem_pipe = atof(x);                            
  }

  if(String(pub.topic()) == "boiler/return_position_Sit630")      // Если положение Sit630
  {
  char x[10];                                                     // Преобразование строки в float
  payload.toCharArray(x,payload.length()+1);
  return_position_Sit630 = atof(x);                            
  }
}



// Функция отправки показаний
void Sending()
{
  if (millis() - count_ms_Send > time_ms_Send)                    // Если разность больше time_ms_Send миллисекунд
  {
  count_ms_Send = millis();                                       // Обуляем разность 
   
    if (cycle_number)                                             // Если цикл #1
    {
    cycle_number = !cycle_number;                                 // Изменяем на цикл #0. Повторяется на каждый второй вход в процедуру
    client.publish("room/tem",String(tem_room));                  // Отправляем данные в топики
    client.publish("room/bme280",String(tem_BME280));
   
    client.publish("room/cor",String(correction));
    client.publish("room/zad",String(tem_set));                          
    
    client.publish("room/dif",String(tem_room-tem_set));                  
    client.publish("room/pres",String(pres_room));
    
    client.publish("room/minutes",String(minute_clock));                            
    client.publish("room/hour",String(hour_clock));
    client.publish("room/minutes_in_day",String(hour_clock*60 + minute_clock));        
    }
    else                                                                // Если цикл #0                                                    
      {  
      cycle_number = !cycle_number;                                     // Изменяем на цикл #1. Повторяется на каждый второй вход в процедуру
      
      BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);              // Определяем единицы измерения
      BME280::PresUnit presUnit(BME280::PresUnit_Pa);                   //
      
      bme.read(pres_room, tem_BME280, hum_room, tempUnit, presUnit);    // Считываем давление, температуру и влажность в заданных единицах  
      
      tem_room =  tem_BME280 + correction;                              // Изменяем температуру на величину коррекции                      
      pres_room = pres_room*0.00750063755419211 + 0.01;                 // Изменяем давление на величину коррекции  
      hum_room = hum_room + 6.53;
      
    
      digitalWrite(Onboard_Led, LOW);                                   // Короткое включение встроенного светодиода при каждом измерении
      delay(50);
      digitalWrite(Onboard_Led, HIGH);  
 
        
        
        
        
        if (cycle_Counter==0)                                           // Если сюда было 30 входов и счетчик входов обнулился то отправляем данные в ThingSpeak
        {
            cycle_Counter=30;                                           // Восстанавливаем счетчик входов 
            if (channel_number)                                         // Если текущий канал #1 Дом
            {
            channel_number = !channel_number;                           // Изменяем на канал #0 Котел
            ThingSpeak.setField(1,tem_room);         
            ThingSpeak.setField(2,tem_room-tem_set);
            ThingSpeak.setField(3,tem_set);
            ThingSpeak.setField(4,tem_out);
            ThingSpeak.setField(5,pres_room);
            ThingSpeak.setField(6,pres_out);
            ThingSpeak.setField(7,hum_room);
            ThingSpeak.setField(8,hum_out);
            ThingSpeak.writeFields(myChannelNumber_1, myWriteAPIKey_1); // Отправляем 8 значений  
            }
            else                                                        // Если текущий канал #0 Котел
            {
            channel_number = !channel_number;                           // Изменяем на канал #1 Дом
            ThingSpeak.setField(1,tem_boiler);                                                                                                              
            ThingSpeak.setField(2,tem_pipe);                                                              
            ThingSpeak.setField(3,return_position_Sit630);       
            ThingSpeak.writeFields(myChannelNumber_2, myWriteAPIKey_2); // Отправляем 3 значения
            }
         }
      cycle_Counter--;                                                  // Если входа в отправку ThingSpeak уменьшаем счетчик циклов на 1. Отправка произодит каждый тридцатый из второго односекундного цикла
      }
   }
delay(50);  
}



// Функция преобразования времени. Для добавления незначащего нуля к целому числу 
String Clock (int item)
{
  if (item <= 9) return "0" + String (item);
  else return String (item);
}
