/* 
Дисплей SH1106 подключен:
SCL  к D1 GPIO(5)
SDA  к D2 GPIO(4)

SCL BMP280 и Si7021 подключены к D1 GPIO(5)
SDA BMP280 и Si7021 подключены к D2 GPIO(4)

Встроенный светодиод D4 GPIO(2)

Библиотека U8g2, параметры от другого типа дисплея SSD1306, т.к. к SH1106 не нашлось подходящих

I2C device found at address 0x3D  ! Oled Дисплей
I2C device found at address 0x76  ! BME 280

*/
#define BUFFER_SIZE 100
#define Onboard_Led 2

#include <ESP8266WiFi.h>                                      // Основная библиотека               
#include <ESP8266mDNS.h>                                      // Библиотека для работы с именами хостов
#include <WiFiUdp.h>                                          // Библиотека для работы с UDP
#include <ArduinoOTA.h>                                       // Библиотека прошивки через WiFi
#include <EEPROM.h>                                           // Библиотека для работы с EEPROM

const char* ssid =  "One";                                    // Имя вайфай точки доступа
const char* pass =  "73737373";                               // Пароль от точки доступа

#include <IFTTTMaker.h>                                       // Определения для IFTTT
#define KEY "nxDul7UGF8ulaxG3tXgKU2iQSCYkRlJvZ_5pZdC2UX0"     // Get it from this page https://ifttt.com/services/maker/settings
#define EVENT_NAME "split"                                     // Name of your event name, set when you are creating the applet
WiFiClientSecure IFTTclient;                                  // Определения для IFTTT
IFTTTMaker ifttt(KEY, IFTTclient);                            // Определения для IFTTT

#include <PubSubClient.h>                                     // Определения для работы с MQTT сервером
const char *mqtt_server = "m15.cloudmqtt.com";                // Имя сервера MQTT
const int  mqtt_port = 16101;                                 // Порт для подключения к серверу MQTT
const char *mqtt_user = "uwkohibc";                           // Логин от MQTT сервера
const char *mqtt_pass = "IoID9IXfYlM5";                       // Пароль от MQTT сервера
WiFiClient MQTTclient;                                        // Определения для работы с MQTT сервером
PubSubClient client(MQTTclient, mqtt_server, mqtt_port);      // Определения для работы с MQTT сервером

#include <Wire.h>

#include <BME280I2C.h>                                        // Определения для работы метеорологическим датчиком

BME280I2C bme;                                                // Определения для работы метеорологическим датчиком

#include <Arduino.h>                                                          // Определения для работы с OLED экраном
#include <U8g2lib.h>                                                          // Определения для работы с OLED экраном
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);      // Определения для работы с OLED экраном


float tem_split, tem_BME280, tem_stored, tem_min, pres_split, hum_split, correction;

// Счетчик мс интервала отправки сообщений на сервер, заданное время отправки в мс
long count_ms_Send, time_ms_Send, ac_code_to_sent;                                                                                

// Счетчик циклов 30*2*1 сек для ThingSpeak, счетчик попыток подключения к WiFi, счетчик попыток подключения к MQTT, часы, минуты, секунды
int  count_wifi, count_mqtt, timer_sec, timer_min = 30;

// Номер цикла, номер канала (0 или 1)
bool cycle_number, timer_flaf, split_flag;                                                                                  // Состояние термостата и имя цикла                        

String timer_symbol;

#include <IRremoteESP8266.h>
#include <IRsend.h>

IRsend irsend(0);  // An IR LED is controlled by GPIO0 (D3)






void setup()
{
  Serial.begin(9600);                                       // Скорость работы последовательного порта
  WiFi.mode(WIFI_STA);                                      // Режим WiFi только STA
  WiFi.begin(ssid, pass);                                   // Подключение к WiFi
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {ESP.wdtFeed(); delay(50); count_wifi--; if (count_wifi <=0) ESP.restart();}
  count_wifi = 50;
  WiFi.hostname("WeMos_Split");
  ArduinoOTA.setHostname("WeMos_Split");                     // Определение имени устройства в сети
  ArduinoOTA.begin();                                       // Запуск подпрограммы загрузки скетча "по воздуху"
  
  Wire.begin();
  //while(!bme.begin()) {delay(500);}
  bme.begin();
 //ThingSpeak.begin(ThingSpeakclient);                       // Инициализация ThingSpeak
 
  ESP.wdtFeed();                                            // Сброс сторожевого таймера
  EEPROM.begin(512);                                        // Чтение из EEPROM сохраненной корректировки
  EEPROM.get(0, correction);
  EEPROM.commit();
  EEPROM.end();

  count_ms_Send = millis();                                 // Счетчик мс отправки сообщений - сброс 
  time_ms_Send = 1000;                                      // Период отправки сообщений 1 сек.
  count_wifi = 50;                                          // Счетчик неудачных подключений 50
  count_mqtt = 50;                                          // Счетчик неудачных подключений 50
  
  cycle_number = false;                                     // Номер цикла 0
  

  u8g2.begin();                                             // Инициализация дисплея  
  u8g2.enableUTF8Print(); 
  
  pinMode(Onboard_Led, OUTPUT);
  
  delay (500);
  irsend.begin();
  split_flag = false;
  
  
  ifttt.triggerEvent(EVENT_NAME, "Split перезагружен:\n" + ESP.getResetReason()); // Отправка сообщения о перезагрузке
}



void loop() 
{
  ArduinoOTA.handle();                                                      // Процедура загрузки скетча по WiFi
                                                                                      
  if (WiFi.status() != WL_CONNECTED)                                        // Подключение к WiFi       
    {
    WiFi.begin(ssid, pass);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)  {ESP.wdtFeed(); count_wifi--; if (count_wifi<=0) ESP.restart(); else return;}
    count_wifi = 50;
    WiFi.hostname("WeMos_Split");
    }
                                                 
  if (WiFi.status() == WL_CONNECTED)                                          
  { 
    if (!client.connected())
    {
        if (client.connect(MQTT::Connect("Mqtt_Split").set_auth(mqtt_user, mqtt_pass))) // Подключение к MQTT серверу  
        {
        count_mqtt = 50;

        client.set_callback(reception);                                               // Определение процедуры чтения сообщений
        
         
        client.subscribe("split/dec");                                                // Подписка на топики
        client.subscribe("split/inc");
        
        client.subscribe("split/on");
        client.subscribe("split/off");
        client.subscribe("split/timer_on");
        client.subscribe("split/timer_inc");
        client.subscribe("split/timer_dec");
        client.subscribe("split/timer_min_set");
        client.subscribe("split/speed");       
        
        
        } 
        else {ESP.wdtFeed(); delay (100); count_mqtt--; if (count_mqtt<=0) ESP.restart();}
    }
    if (client.connected()){client.loop();Sending();}                                 // Если подключение не прервалось вызов отправки показаний
  }
 
   
  
  u8g2.firstPage();                                                                   // Вывод информации на дисплей
  do
    {
    u8g2.setFont(u8g2_font_profont22_tf);
    u8g2.setCursor(3, 15);
    u8g2.print(tem_split); 
    u8g2.setFont(u8g2_font_profont11_tf);
    u8g2.setCursor(15, 32);
    u8g2.print(String(pres_split));  
    u8g2.setCursor(19, 47);
    u8g2.print(String(hum_split));
    } 
    while (u8g2.nextPage());
}
// конец void loop



// Функция получения данных от сервера
void reception(const MQTT::Publish& pub)
{
  String payload = pub.payload_string();         // Получение значения                 
 
    
  if(String(pub.topic()) == "split/dec")          // Если корректировка -0,1
  {
  if (payload.toInt()) correction = correction - 0.1 ;
  EEPROM.begin(512);                             // Сохраняем значение в памяти
  EEPROM.put(0, correction);
  EEPROM.end();
  }
  
  if(String(pub.topic()) == "split/inc")         // Если корректировка +0,1
  {
  if (payload.toInt()) correction = correction + 0.1 ;                     
  EEPROM.begin(512);                            // Сохраняем значение в памяти
  EEPROM.put(0, correction);
  EEPROM.end();
  }



  if (String(pub.topic()) == "split/on")  
  {
    if (payload.toInt() == 1) Ac_Send_Code(0x8800347); 
  }
  
  if (String(pub.topic()) == "split/off")  
  {
    if (payload.toInt() == 1) Ac_Send_Code(0x88C0051); 
  }
  
  if (String(pub.topic()) == "split/timer_on")  
  {
    if (payload.toInt() == 1) timer_flaf = !timer_flaf; if (!timer_flaf) timer_sec = timer_min*60;
  }
  
  if (String(pub.topic()) == "split/timer_inc")  
  {
    if (payload.toInt() == 1) timer_sec = timer_sec + 5*60; 
  } 

  if (String(pub.topic()) == "split/timer_dec")  
  {
    if (payload.toInt() == 1) {if (timer_sec > 300) timer_sec = timer_sec - 5*60;} 
  } 

  if (String(pub.topic()) == "split/timer_min_set")  
  {
  timer_sec = payload.toInt()*60; 
  }
  
  
  if (String(pub.topic()) == "split/speed")  
  {
    switch (payload.toInt()) 
    {
    case 1:
      Ac_Send_Code(0x880830B);
      break;
    case 2:
      Ac_Send_Code(0x880832D);
      break;
     case 3:
      Ac_Send_Code(0x880834F);
    }
  }   
}



// Функция отправки показаний
void Sending()
{
  if (millis() - count_ms_Send > time_ms_Send)                        // Если разность больше time_ms_Send миллисекунд
  {
  count_ms_Send = millis();                                           // Обуляем разность 
   
    if (cycle_number)                                                 // Если цикл #1
    {
    cycle_number = !cycle_number;                                     // Изменяем на цикл #0. Повторяется на каждый второй вход в процедуру
    client.publish("split/tem",String(tem_split));                    // Отправляем данные в топики
    client.publish("split/bme280",String(tem_BME280));
    client.publish("split/tem_min",String(tem_min));
    
    
    
    client.publish("split/cor",String(correction));
       
    client.publish("split/pres",String(pres_split));
    
    if (timer_flaf) {timer_sec--; timer_symbol = "🔴     ";} else timer_symbol = "🔵      ";
    if (timer_sec <=0) {Ac_Send_Code(0x88C0051); timer_sec = timer_min*60; timer_flaf = false;}
  
    client.publish ("split/timer", timer_symbol + Hour_display(timer_sec));
    client.publish ("split/timer_min_ans", String(timer_sec/60));
    }
    
    else                                                                // Если цикл #0                                                    
    {  
    cycle_number = !cycle_number;                                       // Изменяем на цикл #1. Повторяется на каждый второй вход в процедуру
      
    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);                // Определяем единицы измерения
    BME280::PresUnit presUnit(BME280::PresUnit_Pa);                   
      
    bme.read(pres_split, tem_BME280, hum_split, tempUnit, presUnit);    // Считываем давление, температуру и влажность в заданных единицах  
      
    tem_split =  tem_BME280 + correction;                               // Изменяем температуру на величину коррекции                      
    pres_split = pres_split*0.00750063755419211;                        // Изменяем давление на величину коррекции  
    
    if (split_flag) tem_min = tem_split - tem_stored ; else tem_min = 0;    
   
    digitalWrite(Onboard_Led, LOW);   
    delay(50);
    digitalWrite(Onboard_Led, HIGH);     
    }
   }
delay(50);
}



void Ac_Send_Code(uint32_t code)
{  
  irsend.sendLG(code, 28);

  switch (code) 
    {
    case 0x8800347:
      client.publish ("split/code","Включить"); 
      split_flag = true; tem_stored = tem_split;
      break;
    case 0x88C0051:
      client.publish ("split/code","Отключить"); 
      split_flag = false;
      break;
    case 0x880830B:
      client.publish ("split/code","Медленно"); 
      break;
    case 0x880832D:
      client.publish ("split/code","Средне"); 
      break;
    case 0x880834F:
      client.publish ("split/code","Быстро"); 
    }
}


String Hour_display (int timer)
{
  int hour, minute, second;
  String st_hour, st_minute, st_second;

  hour = timer / 3600;
  minute = (timer % 3600) / 60;
  second = (timer % 3600) % 60;

  if (hour >= 10 && hour <= 99) st_hour = String (hour) + ":"; else if (hour > 0 && hour <= 9) st_hour = "0" + String (hour) + ":"; else st_hour = String ("");
  if (minute >= 10) st_minute = String (minute); else st_minute = "0" + String (minute);
  if (second >= 10) st_second = String (second); else st_second = "0" + String (second);

  return st_hour + st_minute + ":" + st_second;
}
