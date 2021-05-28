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
#define WDT   0

#include <ESP8266WiFi.h>                                      // Основная библиотека               

#include <EEPROM.h>                                           // Библиотека для работы с EEPROM

#include "Account.h"

#include <PubSubClient.h>                                     // Определения для работы с MQTT сервером 1mqtt@kononov.xyz

WiFiClient MQTTclient;                                        // Определения для работы с MQTT сервером
PubSubClient client(MQTTclient, mqtt_server, mqtt_port);      // Определения для работы с MQTT сервером

#include <Wire.h>                                             // Определения для работы с часами
#include "RTClib.h"                                           // Определения для работы с часами
RTC_DS3231 DS3231;


// Счетчик мс интервала отправки сообщений на сервер, заданное время отправки в мс
long count_ms_Send, time_ms_Send;                                                                                

int cycle_Counter, count_wifi, count_mqtt, hour_clock, minute_clock, second_clock;

int channel [4] = {16, 14, 12, 13};

int timer0_on = 0, timer1_on = 0, timer2_on = 0, timer3_on = 0, timer0_off = 0, timer1_off = 0, timer2_off = 0, timer3_off = 0;

int timer0_on_hour = 0, timer0_on_minute = 0, timer1_on_hour = 0, timer1_on_minute = 0, timer2_on_hour = 0, timer2_on_minute = 0, timer3_on_hour = 0, timer3_on_minute = 0;
int timer0_off_hour = 0, timer0_off_minute = 0, timer1_off_hour = 0, timer1_off_minute = 0, timer2_off_hour = 0, timer2_off_minute = 0, timer3_off_hour = 0, timer3_off_minute = 0;

int switch_countdown0_on = 0, switch_countdown1_on = 0, switch_countdown2_on = 0, switch_countdown3_on = 0;
int switch_countdown0_off = 0, switch_countdown1_off = 0, switch_countdown2_off = 0, switch_countdown3_off = 0;

long countdown0_on_sec, countdown1_on_sec, countdown2_on_sec, countdown3_on_sec;
long countdown0_off_sec, countdown1_off_sec, countdown2_off_sec, countdown3_off_sec;

String timer_state;

bool flagWDT = true;                                                                                    



void setup()
{
  Serial.begin(9600);                                       // Скорость работы последовательного порта
  WiFi.mode(WIFI_STA);                                      // Режим WiFi только STA
  WiFi.begin(ssid, pass);                                   // Подключение к WiFi
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {ESP.wdtFeed(); delay(50); count_wifi--; if (count_wifi <=0) ESP.restart();}
  count_wifi = 50;
  WiFi.hostname("Internet Switch");
 
  DS3231.begin();                                           // Инициализация часов
  
  ESP.wdtFeed();                                            // Сброс сторожевого таймера
 /*
  EEPROM.begin(512);                                        // Чтение из EEPROM сохраненной корректировки
  EEPROM.get(0, correction);
  EEPROM.commit();
  EEPROM.end();
*/
  count_ms_Send = millis();                                 // Счетчик мс отправки сообщений - сброс 
  time_ms_Send = 1000;                                      // Период отправки сообщений 1 сек.
  count_wifi = 50;                                          // Счетчик неудачных подключений 50
  count_mqtt = 50;                                          // Счетчик неудачных подключений 50
  
  pinMode(Onboard_Led, OUTPUT);
  pinMode(WDT, OUTPUT);
  
  pinMode(16, OUTPUT);
  pinMode(14, OUTPUT); 
  pinMode(12, OUTPUT); 
  pinMode(13, OUTPUT);
  
  digitalWrite(16, 1);
  digitalWrite(14, 1);
  digitalWrite(12, 1);
  digitalWrite(13, 1);
  
  delay (500); 
}



void loop() 
{
                                                                                        
  if (WiFi.status() != WL_CONNECTED)                                                        // Подключение к WiFi       
    {
    WiFi.begin(ssid, pass);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)  {ESP.wdtFeed(); count_wifi--; if (count_wifi<=0) ESP.restart(); else return;}
    count_wifi = 50;
    WiFi.hostname("Internet Switch");
    }
                                                 
  if (WiFi.status() == WL_CONNECTED)                                          
  { 
    if (!client.connected())
    {
        if (client.connect(MQTT::Connect("IS1MQTTDEV001").set_auth(mqtt_user, mqtt_pass))) // Подключение к MQTT серверу  
        {
        count_mqtt = 50;

        client.set_callback(reception);                                                   // Определение процедуры чтения сообщений
        
        client.subscribe("DEV000/settime");                                               // Подписка на топики
        
        client.subscribe("DEV001/on");
        client.subscribe("DEV001/off");
        client.subscribe("DEV002/on");
        client.subscribe("DEV002/off");
        client.subscribe("DEV003/on");
        client.subscribe("DEV003/off");
        client.subscribe("DEV004/on");
        client.subscribe("DEV004/off");

        client.subscribe("DEV001/timeon");
        client.subscribe("DEV001/timeoff");
        client.subscribe("DEV002/timeon");
        client.subscribe("DEV002/timeoff");
        client.subscribe("DEV003/timeon");
        client.subscribe("DEV003/timeoff");
        client.subscribe("DEV004/timeon");
        client.subscribe("DEV004/timeoff");
        
        client.subscribe("DEV001/timer_on");
        client.subscribe("DEV002/timer_on");
        client.subscribe("DEV003/timer_on");
        client.subscribe("DEV004/timer_on");
        
        client.subscribe("DEV001/timer_off");
        client.subscribe("DEV002/timer_off");
        client.subscribe("DEV003/timer_off");
        client.subscribe("DEV004/timer_off");
        
        client.subscribe("DEV001/countdown_on");
        client.subscribe("DEV001/countdown_off");
        client.subscribe("DEV002/countdown_on");
        client.subscribe("DEV002/countdown_off");
        client.subscribe("DEV003/countdown_on");
        client.subscribe("DEV003/countdown_off");
        client.subscribe("DEV004/countdown_on");
        client.subscribe("DEV004/countdown_off");
        
        client.subscribe("DEV001/switch_countdown_on");
        client.subscribe("DEV002/switch_countdown_on");
        client.subscribe("DEV003/switch_countdown_on");
        client.subscribe("DEV004/switch_countdown_on");
        
        client.subscribe("DEV001/switch_countdown_off");
        client.subscribe("DEV002/switch_countdown_off");
        client.subscribe("DEV003/switch_countdown_off");
        client.subscribe("DEV004/switch_countdown_off");
        } 
        else {ESP.wdtFeed(); delay (100); count_mqtt--; if (count_mqtt<=0) ESP.restart();}
    }
    if (client.connected()){client.loop();Sending();}                                   // Если подключение не прервалось вызов отправки показаний
  }
 
  DateTime now = DS3231.now();                                                          // Чтение времени 
  hour_clock = now.hour();
  minute_clock = now.minute();
  second_clock = now.second();
  
  
  if ((timer0_on_hour == hour_clock) && (timer0_on_minute == minute_clock) && timer0_on) digitalWrite(channel[0], LOW); 
  if ((timer0_off_hour == hour_clock) && (timer0_off_minute == minute_clock) && timer0_off) digitalWrite(channel[0], HIGH); 

  if ((timer1_on_hour == hour_clock) && (timer1_on_minute == minute_clock) && timer1_on) digitalWrite(channel[1], LOW); 
  if ((timer1_off_hour == hour_clock) && (timer1_off_minute == minute_clock) && timer1_off) digitalWrite(channel[1], HIGH); 

  if ((timer2_on_hour == hour_clock) && (timer2_on_minute == minute_clock) && timer2_on) digitalWrite(channel[2], LOW); 
  if ((timer2_off_hour == hour_clock) && (timer2_off_minute == minute_clock) && timer2_off) digitalWrite(channel[2], HIGH);

  if ((timer3_on_hour == hour_clock) && (timer3_on_minute == minute_clock) && timer3_on) digitalWrite(channel[3], LOW); 
  if ((timer3_off_hour == hour_clock) && (timer3_off_minute == minute_clock) && timer3_off) digitalWrite(channel[3], HIGH);

  flagWDT = !flagWDT; digitalWrite(WDT, flagWDT); 
  

  
}
// конец void loop



// Функция получения данных от сервера
void reception(const MQTT::Publish& pub)
{
  String payload = pub.payload_string();                                                      // Получение значения                 

  if(String(pub.topic()) == "DEV000/settime") 
  {
    if (payload.length()==8)
    {
    DS3231.adjust(DateTime(2019, 1, 1, (payload.substring(0,2)).toInt(), (payload.substring(3,5)).toInt(), (payload.substring(6,8)).toInt()));
    }
  }

// DEV001
//------------------------------------------------------------------------------------------------------------------
  
  if(String(pub.topic()) == "DEV001/on") 
  {
  if (payload.toInt()) digitalWrite(channel[0], LOW);
  }

  if(String(pub.topic()) == "DEV001/off") 
  {
  if (payload.toInt()) digitalWrite(channel[0], HIGH);
  }

  if(String(pub.topic()) == "DEV001/timeon") 
  {
  timer0_on_hour = (payload.substring(0,2)).toInt(); timer0_on_minute = (payload.substring(3,5)).toInt();
  }

  if(String(pub.topic()) == "DEV001/timeoff") 
  {
  timer0_off_hour = (payload.substring(0,2)).toInt(); timer0_off_minute = (payload.substring(3,5)).toInt();
  }

  if(String(pub.topic()) == "DEV001/timer_on")
  {
  if (payload.toInt()== 0)      {timer0_on = 0;}
  else if (payload.toInt()== 1) {timer0_on = 1;}
  }

  if(String(pub.topic()) == "DEV001/timer_off")
  {
  if (payload.toInt()== 0)      {timer0_off = 0;}
  else if (payload.toInt()== 1) {timer0_off = 1;}
  }

  if(String(pub.topic()) == "DEV001/countdown_on")
  {
  countdown0_on_sec = payload.toInt()*60;
  }

  if(String(pub.topic()) == "DEV001/countdown_off")
  {
  countdown0_off_sec = payload.toInt()*60;
  }
 
  if(String(pub.topic()) == "DEV001/switch_countdown_on")
  {
  if (payload.toInt()== 0)      {switch_countdown0_on = 0; countdown0_on_sec = 3600;}
  else if (payload.toInt()== 1) {switch_countdown0_on = 1;}
  }

  if(String(pub.topic()) == "DEV001/switch_countdown_off")
  {
  if (payload.toInt()== 0)      {switch_countdown0_off = 0; countdown0_off_sec = 3600;}
  else if (payload.toInt()== 1) {switch_countdown0_off = 1;}
  }

//------------------------------------------------------------------------------------------------------------------
 

// DEV002
//------------------------------------------------------------------------------------------------------------------
  
  if(String(pub.topic()) == "DEV002/on") 
  {
  if (payload.toInt()) digitalWrite(channel[1], LOW);
  }

  if(String(pub.topic()) == "DEV002/off") 
  {
  if (payload.toInt()) digitalWrite(channel[1], HIGH);
  }

  if(String(pub.topic()) == "DEV002/timeon") 
  {
  timer1_on_hour = (payload.substring(0,2)).toInt(); timer1_on_minute = (payload.substring(3,5)).toInt();
  }

  if(String(pub.topic()) == "DEV002/timeoff") 
  {
  timer1_off_hour = (payload.substring(0,2)).toInt(); timer1_off_minute = (payload.substring(3,5)).toInt();
  }

  if(String(pub.topic()) == "DEV002/timer_on")
  {
  if (payload.toInt()== 0)      {timer1_on = 0;}
  else if (payload.toInt()== 1) {timer1_on = 1;}
  }

  if(String(pub.topic()) == "DEV002/timer_off")
  {
  if (payload.toInt()== 0)      {timer1_off = 0;}
  else if (payload.toInt()== 1) {timer1_off = 1;}
  }

  if(String(pub.topic()) == "DEV002/countdown_on")
  {
  countdown1_on_sec = payload.toInt()*60;
  }

  if(String(pub.topic()) == "DEV002/countdown_off")
  {
  countdown1_off_sec = payload.toInt()*60;
  }
 
  if(String(pub.topic()) == "DEV002/switch_countdown_on")
  {
  if (payload.toInt()== 0)      {switch_countdown1_on = 0; countdown1_on_sec = 3600;}
  else if (payload.toInt()== 1) {switch_countdown1_on = 1;}
  }

  if(String(pub.topic()) == "DEV002/switch_countdown_off")
  {
  if (payload.toInt()== 0)      {switch_countdown1_off = 0; countdown1_off_sec = 3600;}
  else if (payload.toInt()== 1) {switch_countdown1_off = 1;}
  }

//------------------------------------------------------------------------------------------------------------------


// DEV003
//------------------------------------------------------------------------------------------------------------------
  
  if(String(pub.topic()) == "DEV003/on") 
  {
  if (payload.toInt()) digitalWrite(channel[2], LOW);
  }

  if(String(pub.topic()) == "DEV003/off") 
  {
  if (payload.toInt()) digitalWrite(channel[2], HIGH);
  }

  if(String(pub.topic()) == "DEV003/timeon") 
  {
  timer2_on_hour = (payload.substring(0,2)).toInt(); timer2_on_minute = (payload.substring(3,5)).toInt();
  }

  if(String(pub.topic()) == "DEV003/timeoff") 
  {
  timer2_off_hour = (payload.substring(0,2)).toInt(); timer2_off_minute = (payload.substring(3,5)).toInt();
  }

  if(String(pub.topic()) == "DEV003/timer_on")
  {
  if (payload.toInt()== 0)      {timer2_on = 0;}
  else if (payload.toInt()== 1) {timer2_on = 1;}
  }

  if(String(pub.topic()) == "DEV003/timer_off")
  {
  if (payload.toInt()== 0)      {timer2_off = 0;}
  else if (payload.toInt()== 1) {timer2_off = 1;}
  }

  if(String(pub.topic()) == "DEV003/countdown_on")
  {
  countdown2_on_sec = payload.toInt()*60;
  }

  if(String(pub.topic()) == "DEV003/countdown_off")
  {
  countdown2_off_sec = payload.toInt()*60;
  }
 
  if(String(pub.topic()) == "DEV003/switch_countdown_on")
  {
  if (payload.toInt()== 0)      {switch_countdown2_on = 0; countdown2_on_sec = 3600;}
  else if (payload.toInt()== 1) {switch_countdown2_on = 1;}
  }

  if(String(pub.topic()) == "DEV003/switch_countdown_off")
  {
  if (payload.toInt()== 0)      {switch_countdown2_off = 0; countdown2_off_sec = 3600;}
  else if (payload.toInt()== 1) {switch_countdown2_off = 1;}
  }

//------------------------------------------------------------------------------------------------------------------


// DEV004
//------------------------------------------------------------------------------------------------------------------
  
  if(String(pub.topic()) == "DEV004/on") 
  {
  if (payload.toInt()) digitalWrite(channel[3], LOW);
  }

  if(String(pub.topic()) == "DEV004/off") 
  {
  if (payload.toInt()) digitalWrite(channel[3], HIGH);
  }

  if(String(pub.topic()) == "DEV004/timeon") 
  {
  timer3_on_hour = (payload.substring(0,2)).toInt(); timer3_on_minute = (payload.substring(3,5)).toInt();
  }

  if(String(pub.topic()) == "DEV004/timeoff") 
  {
  timer3_off_hour = (payload.substring(0,2)).toInt(); timer3_off_minute = (payload.substring(3,5)).toInt();
  }

  if(String(pub.topic()) == "DEV004/timer_on")
  {
  if (payload.toInt()== 0)      {timer3_on = 0;}
  else if (payload.toInt()== 1) {timer3_on = 1;}
  }

  if(String(pub.topic()) == "DEV004/timer_off")
  {
  if (payload.toInt()== 0)      {timer3_off = 0;}
  else if (payload.toInt()== 1) {timer3_off = 1;}
  }

  if(String(pub.topic()) == "DEV004/countdown_on")
  {
  countdown3_on_sec = payload.toInt()*60;
  }

  if(String(pub.topic()) == "DEV004/countdown_off")
  {
  countdown3_off_sec = payload.toInt()*60;
  }
 
  if(String(pub.topic()) == "DEV004/switch_countdown_on")
  {
  if (payload.toInt()== 0)      {switch_countdown3_on = 0; countdown3_on_sec = 3600;}
  else if (payload.toInt()== 1) {switch_countdown3_on = 1;}
  }

  if(String(pub.topic()) == "DEV004/switch_countdown_off")
  {
  if (payload.toInt()== 0)      {switch_countdown3_off = 0; countdown3_off_sec = 3600;}
  else if (payload.toInt()== 1) {switch_countdown3_off = 1;}
  }

//------------------------------------------------------------------------------------------------------------------
}



// Функция отправки показаний
void Sending()
{
  if (millis() - count_ms_Send > time_ms_Send)                    // Если разность больше time_ms_Send миллисекунд
  {
  count_ms_Send = millis();                                       // Обуляем разность 
   
       
    
    client.publish("DEV000/time",Clock(hour_clock) + ":" + Clock(minute_clock) + ":" + Clock(second_clock));                            
    
    if (timer0_on) timer_state = "      🔴"; else timer_state = "";
    client.publish("DEV001/timeonset",Clock(timer0_on_hour) + ":" + Clock(timer0_on_minute) + timer_state);
    
    if (timer0_off) timer_state = "      🔴"; else timer_state = "";
    client.publish("DEV001/timeoffset",Clock(timer0_off_hour) + ":" + Clock(timer0_off_minute) + timer_state); 
    
    if (timer1_on) timer_state = "      🔴"; else timer_state = "";
    client.publish("DEV002/timeonset",Clock(timer1_on_hour) + ":" + Clock(timer1_on_minute) + timer_state);
    
    if (timer1_off) timer_state = "      🔴"; else timer_state = "";
    client.publish("DEV002/timeoffset",Clock(timer1_off_hour) + ":" + Clock(timer1_off_minute) + timer_state); 


    if (timer2_on) timer_state = "      🔴"; else timer_state = "";
    client.publish("DEV003/timeonset",Clock(timer2_on_hour) + ":" + Clock(timer2_on_minute) + timer_state);
    
    if (timer2_off) timer_state = "      🔴"; else timer_state = "";
    client.publish("DEV003/timeoffset",Clock(timer2_off_hour) + ":" + Clock(timer2_off_minute) + timer_state); 


    if (timer3_on) timer_state = "      🔴"; else timer_state = "";
    client.publish("DEV004/timeonset",Clock(timer3_on_hour) + ":" + Clock(timer3_on_minute) + timer_state);
    
    if (timer3_off) timer_state = "      🔴"; else timer_state = "";
    client.publish("DEV004/timeoffset",Clock(timer3_off_hour) + ":" + Clock(timer3_off_minute) + timer_state); 


    

    
    
    if (switch_countdown0_on) {countdown0_on_sec--; timer_state = "      🔴";} else timer_state = "";
    if (countdown0_on_sec <=0) {digitalWrite(channel[0], LOW); countdown0_on_sec = 3600; switch_countdown0_on = 0; timer_state = ""; client.publish ("DEV001/switch_countdown_on", "0"); client.publish ("DEV001/countdown_on", "60");}
    client.publish("DEV001/countdownseton",Hour_display(countdown0_on_sec) + timer_state);
  
    if (switch_countdown0_off) {countdown0_off_sec--; timer_state = "      🔴";} else timer_state = "";
    if (countdown0_off_sec <=0) {digitalWrite(channel[0], HIGH); countdown0_off_sec = 3600; switch_countdown0_off = 0; timer_state = ""; client.publish ("DEV001/switch_countdown_off", "0"); client.publish ("DEV001/countdown_off", "60");}
    client.publish("DEV001/countdownsetoff",Hour_display(countdown0_off_sec) + timer_state);
    
    

    if (switch_countdown1_on) {countdown1_on_sec--; timer_state = "      🔴";} else timer_state = "";
    if (countdown1_on_sec <=0) {digitalWrite(channel[1], LOW); countdown1_on_sec = 3600; switch_countdown1_on = 0; timer_state = ""; client.publish ("DEV002/switch_countdown_on", "0"); client.publish ("DEV002/countdown_on", "60");}
    client.publish("DEV002/countdownseton",Hour_display(countdown1_on_sec) + timer_state);
  
    if (switch_countdown1_off) {countdown1_off_sec--; timer_state = "      🔴";} else timer_state = "";
    if (countdown1_off_sec <=0) {digitalWrite(channel[1], HIGH); countdown1_off_sec = 3600; switch_countdown1_off = 0; timer_state = ""; client.publish ("DEV002/switch_countdown_off", "0"); client.publish ("DEV002/countdown_off", "60");}
    client.publish("DEV002/countdownsetoff",Hour_display(countdown1_off_sec) + timer_state);



    if (switch_countdown2_on) {countdown2_on_sec--; timer_state = "      🔴";} else timer_state = "";
    if (countdown2_on_sec <=0) {digitalWrite(channel[2], LOW); countdown2_on_sec = 3600; switch_countdown2_on = 0; timer_state = ""; client.publish ("DEV003/switch_countdown_on", "0"); client.publish ("DEV003/countdown_on", "60");}
    client.publish("DEV003/countdownseton",Hour_display(countdown2_on_sec) + timer_state);
  
    if (switch_countdown2_off) {countdown2_off_sec--; timer_state = "      🔴";} else timer_state = "";
    if (countdown2_off_sec <=0) {digitalWrite(channel[2], HIGH); countdown2_off_sec = 3600; switch_countdown2_off = 0; timer_state = ""; client.publish ("DEV003/switch_countdown_off", "0"); client.publish ("DEV003/countdown_off", "60");}
    client.publish("DEV003/countdownsetoff",Hour_display(countdown2_off_sec) + timer_state);
   
   
    
    if (switch_countdown3_on) {countdown3_on_sec--; timer_state = "      🔴";} else timer_state = "";
    if (countdown3_on_sec <=0) {digitalWrite(channel[3], LOW); countdown3_on_sec = 3600; switch_countdown3_on = 0; timer_state = ""; client.publish ("DEV004/switch_countdown_on", "0"); client.publish ("DEV004/countdown_on", "60");}
    client.publish("DEV004/countdownseton",Hour_display(countdown3_on_sec) + timer_state);
  
    if (switch_countdown3_off) {countdown3_off_sec--; timer_state = "      🔴";} else timer_state = "";
    if (countdown3_off_sec <=0) {digitalWrite(channel[3], HIGH); countdown3_off_sec = 3600; switch_countdown3_off = 0; timer_state = ""; client.publish ("DEV004/switch_countdown_off", "0"); client.publish ("DEV004/countdown_off", "60");}
    client.publish("DEV004/countdownsetoff",Hour_display(countdown3_off_sec) + timer_state);
    
    
    
    
    
    
    digitalWrite(Onboard_Led, LOW);                                         // Короткое включение встроенного светодиода при каждом измерении
    delay(50);
    digitalWrite(Onboard_Led, HIGH);                         
   }
delay(50);  
}



// Функция преобразования времени. Для добавления незначащего нуля к целому числу 
String Clock (int item)
{
  if (item <= 9) return "0" + String (item);
  else return String (item);
}


// Функция преобразования времени в строку с двоеточием
// Для перевода миллисекунд в чч:мм:сек
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
