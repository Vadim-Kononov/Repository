/* 
SCL D1 GPIO(5)
SDA D2 GPIO(4)

Встроенный светодиод D4 GPIO(2)

D6 GPIO (12) приемник
D0 GPIO (16) передатчик


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

#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();

#include <EEPROM.h>  

int reception_allowed = 0, count_wifi, count_mqtt;
long count_ms_Send, time_ms_Send;     
String cod, protocol;

bool flagWDT = true;                                                                                    



void setup()
{
  Serial.begin(9600);                                       // Скорость работы последовательного порта
  WiFi.mode(WIFI_STA);                                      // Режим WiFi только STA
  WiFi.begin(ssid, pass);                                   // Подключение к WiFi
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {ESP.wdtFeed(); delay(50); count_wifi--; if (count_wifi <=0) ESP.restart();}
  count_wifi = 50;
  WiFi.hostname("RC_Grabber");
 
  mySwitch.enableReceive(12);                             // Приемник на D6
  
  // Transmitter is connected to Arduino Pin D0
  mySwitch.enableTransmit(16);                            // Передатчик на D0

  // Optional set pulse length.
  // mySwitch.setPulseLength(320);
  
  // Optional set protocol (default is 1, will work for most outlets)
  // mySwitch.setProtocol(2);
  
  // Optional set number of transmission repetitions.
  // mySwitch.setRepeatTransmit(15);
  
  ESP.wdtFeed();                                            // Сброс сторожевого таймера
 
  count_ms_Send = millis();                                 // Счетчик мс отправки сообщений - сброс 
  time_ms_Send = 7500;                                      // Период отправки сообщений 1 сек.
  count_wifi = 50;                                          // Счетчик неудачных подключений 50
  count_mqtt = 50;                                          // Счетчик неудачных подключений 50
  
  pinMode(Onboard_Led, OUTPUT);
  pinMode(WDT, OUTPUT);
  
  delay (1000); 
}



void loop() 
{
                                                                                        
  if (WiFi.status() != WL_CONNECTED)                                                        // Подключение к WiFi       
    {
    WiFi.begin(ssid, pass);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)  {ESP.wdtFeed(); count_wifi--; if (count_wifi<=0) ESP.restart(); else return;}
    count_wifi = 50;
    WiFi.hostname("RC_Grabber");
    }
                                                 
  if (WiFi.status() == WL_CONNECTED)                                          
  { 
    if (!client.connected())
    {
        if (client.connect(MQTT::Connect("RC_Grabber_001").set_auth(mqtt_user, mqtt_pass))) // Подключение к MQTT серверу  
        {
        count_mqtt = 50;

        client.set_callback(reception);                                                     // Определение процедуры чтения сообщений
        
        client.subscribe("rec_all");                                                        // Подписка на топики
        client.subscribe("send");
        client.subscribe("1");
        client.subscribe("2");
        client.subscribe("3");
        client.subscribe("4");
        client.subscribe("5");
        client.subscribe("6");
        client.subscribe("7");
        client.subscribe("8");
        } 
        else {ESP.wdtFeed(); delay (100); count_mqtt--; if (count_mqtt<=0) ESP.restart();}
    }
    if (client.connected()){client.loop();Sending();}                                       // Если подключение не прервалось вызов отправки показаний
  }

  if (mySwitch.available())
  {  
    int value = mySwitch.getReceivedValue(); 
    if (value == 0) {if (reception_allowed) client.publish("cod", "Unknown encoding");}
      else
      {
      cod = String(mySwitch.getReceivedValue());
      if (reception_allowed) client.publish("cod", String (mySwitch.getReceivedValue()) + " / " + String (mySwitch.getReceivedBitlength()) + " bit   " + "Protocol: " + String (mySwitch.getReceivedProtocol())); 
      }

    mySwitch.resetAvailable();
  }
 
  
 
  delay(50); 
  flagWDT = !flagWDT; digitalWrite(WDT, flagWDT); 
  
}
// конец void loop



// Функция получения данных от сервера
void reception(const MQTT::Publish& pub)
{
  String payload = pub.payload_string();                                                      // Получение значения                 


  if(String(pub.topic()) == "rec_all") 
  {
  reception_allowed = payload.toInt();
  if (!reception_allowed) client.publish("cod", ""); 
  }

  
  if (String(pub.topic()) == "send") 
  {
  int button, bit_n, addr;
  long value;
  
  button = payload.substring(0, payload.indexOf(" ")).toInt();
  value = payload.substring(payload.indexOf(" ")+1, payload.lastIndexOf(" ")).toInt();
  bit_n = payload.substring(payload.lastIndexOf(" ")+1, payload.length()).toInt();
  
  addr = button*8;
  EEPROM.begin(512);                                  
  EEPROM.put(addr, bit_n);                                              
  EEPROM.put(addr+4, value); 
  EEPROM.end();
  
  for (button =8; button>=1; button--)
  {
  addr = button*8;
  EEPROM.begin(512);                                      
  EEPROM.get(addr, bit_n);    
  EEPROM.get(addr+4, value);  
  EEPROM.commit();                                       
  EEPROM.end();                                           
  client.publish("cod_mem", "№" + String (button) + "   " + String (value) + " " + String (bit_n));
  } 
  }
    
  if (String(pub.topic()) == "1")
  {
  Sending_RC (1);
  }
  
  if (String(pub.topic()) == "2")
  {
  Sending_RC (2);
  }

  if (String(pub.topic()) == "3")
  {
  Sending_RC (3);
  }

  if (String(pub.topic()) == "4")
  {
  Sending_RC (4);
  }

  if (String(pub.topic()) == "5")
  {
  Sending_RC (5);
  }

  if (String(pub.topic()) == "6")
  {
  Sending_RC (6);
  }

  if (String(pub.topic()) == "7")
  {
  Sending_RC (7);
  }

  if (String(pub.topic()) == "8")
  {
  Sending_RC (8);
  }
  
  
}



// Функция отправки MQTT 
void Sending()
{
  if (millis() - count_ms_Send > time_ms_Send)                    // Если разность больше time_ms_Send миллисекунд
  {
  count_ms_Send = millis();                                       // Обуляем разность 

  client.publish("rec_all", String (reception_allowed));
  client.publish("cod_send", "");
  
  
  digitalWrite(Onboard_Led, LOW);                                 // Короткое включение встроенного светодиода при каждом измерении
  delay(50);
  digitalWrite(Onboard_Led, HIGH); 
   
  }  
}


// Функция отправки RC 
void Sending_RC (int button)
{
  int bit_n, addr;
  long value;
  reception_allowed = 0;
  addr = button*8;
  EEPROM.begin(512);                                      
  EEPROM.get(addr, bit_n);    
  EEPROM.get(addr+4, value);  
  EEPROM.commit();                                       
  EEPROM.end();
  mySwitch.send(value, bit_n);
  delay(100);
  mySwitch.send(value, bit_n);
  delay(100);
  mySwitch.send(value, bit_n);   
  count_ms_Send = millis();
  client.publish("cod_send", "№" + String (button) + "   " + String (value) + " / " + String (bit_n)); 
  
}
