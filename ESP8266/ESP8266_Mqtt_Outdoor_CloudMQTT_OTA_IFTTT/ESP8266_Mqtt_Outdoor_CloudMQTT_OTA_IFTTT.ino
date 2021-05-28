/* 
Встроенный светодиод подлкючён к D4 (GPIO2) пину
Датчик температуры ds18b20 к D5 (GPIO14) пину
Модуль НС12 подключается к Serial: RX НС12 к TX (GPIO1)(D10) платы, TX НС12 к RX (GPIO1)(D9) платы, при загрузке скетча - отключать.
                         к Serial1:RX НС12 к D4 (GPIO2) платы.
Дисплей SH1106 подключен:
SCK  к D1 GPIO(5)
SDA  к D2 GPIO(4)

SCL BMP280 и Si7021 подключены к D1 GPIO(5)
SDA BMP280 и Si7021 подключены к D2 GPIO(4)

Библиотека U8g2, параметры от другого типа дисплея SSD1306, т.к. к SH1106 не нашлось подходящих

D0   GPIO(16)
D1   GPIO(5)
D2   GPIO(4)
D3   GPIO(0)
D4   GPIO(2)
D5   GPIO(14)
D6   GPIO(12)
D7   GPIO(13)
D8   GPIO(15)
D9   GPIO(3)
D10  GPIO(1)
*/
#define BUFFER_SIZE 100

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <IFTTTMaker.h>


#include "Account.h"

WiFiClientSecure IFTTclient;
IFTTTMaker ifttt(KEY, IFTTclient);

#include <PubSubClient.h>

WiFiClient MQTTclient;      
PubSubClient client(MQTTclient, mqtt_server, mqtt_port);

#include <OneWire.h>
#include <DallasTemperature.h>
#define ds18b20 14
OneWire oneWire(ds18b20);
DallasTemperature thermode(&oneWire);

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "SparkFun_Si7021_Breakout_Library.h"

#define bmp280_Address 0x76
Adafruit_BMP280 bmp280; 

Weather si7021;

float tem_outdoor=22.0, pres_outdoor=760.0, hum_outdoor=50.0, tem_bmp280= 22.0, correction;
long previousMillis = 0, interval = 2000;           // Интервал отправки сообщений на сервер в мс


void setup()
{
  ESP.wdtDisable();
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {Serial.println("Connection Failed! Rebooting..."); delay(5000); ESP.restart();}
  WiFi.hostname("NodeMCU_Outdoor");
  ArduinoOTA.setHostname("NodeMCU_Outdoor");
  ArduinoOTA.begin();
  Serial.println("\n\n\nReady NodeMCU-Outdoor"); Serial.print("IP address: "); Serial.println(WiFi.localIP());
    
  WiFi.printDiag(Serial);
  //Serial.println(WiFi.softAP("NodeMCU-Outdoor", "73737373") ? "Ready" : "Failed!");
  
  thermode.begin();
  if (!bmp280.begin(bmp280_Address)) {Serial.println("\nCould not find a valid BMP280 sensor, check wiring!"); while (1) delay(10);}  
  si7021.begin();

  EEPROM.begin(512);  
  EEPROM.get(0, correction);
//EEPROM.get(4, sensor);
  EEPROM.commit();
  EEPROM.end();

  ifttt.triggerEvent(EVENT_NAME, "The Outdoor module is rebooted:\n" + ESP.getResetReason() + ".\n");
}


void loop() {
  ESP.wdtFeed();
  ArduinoOTA.handle();
  
  // подключаемся к wi-fi
  if (WiFi.status() != WL_CONNECTED)
    {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    
    WiFi.hostname("NodeMCU_Outdoor");
    WiFi.begin(ssid, pass);
        
    if (WiFi.waitForConnectResult() != WL_CONNECTED)  return;
    Serial.println("WiFi connected");
    }

  // подключаемся к MQTT серверу
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!client.connected())
    {
      Serial.println("Connecting to MQTT server");
      if (client.connect(MQTT::Connect("Mqtt_Outdoor").set_auth(mqtt_user, mqtt_pass)))
        {
        Serial.println("Connected to MQTT server");
        delay(50);  
        client.set_callback(reception);
        
        client.subscribe("outdoor/dec");
        client.subscribe("outdoor/inc");    
        } 
        else
        {
        Serial.println("Could not connect to MQTT server");   
        }
    }
    if (client.connected()){client.loop();Sending();}
  }

}
// конец void loop


// Функция получения данных от сервера
void reception(const MQTT::Publish& pub)
{
  //Serial.print(pub.topic());                  // выводим в сериал порт название топика
  //Serial.print(" => ");
  //Serial.println(pub.payload_string());       // выводим в сериал порт значение полученных данных
    
  String payload = pub.payload_string();
 
 /*   
  if(String(pub.topic()) == "outdoor/sens")        // проверяем из нужного ли нам топика пришли данные 
  {
  sensor = payload.toInt();                     // преобразуем выбор датчика в число
  EEPROM.begin(512);  
  EEPROM.put(4, sensor);
  EEPROM.end();
  }
*/
  if(String(pub.topic()) == "outdoor/dec")        // проверяем из нужного ли нам топика пришли данные 
  {
  if (payload.toInt()) correction = correction - 0.1 ;                     // корректируем на -0,1
  EEPROM.begin(512);  
  EEPROM.put(0, correction);
  EEPROM.end();
  }
  
  if(String(pub.topic()) == "outdoor/inc")        // проверяем из нужного ли нам топика пришли данные 
  {
  if (payload.toInt()) correction = correction + 0.1 ;                     // корректируем на +0,1
  EEPROM.begin(512);  
  EEPROM.put(0, correction);
  EEPROM.end();
  }
  
}

// Функция отправки показаний
void Sending()
{
  if (millis() - previousMillis > interval)
  {
  previousMillis = millis();  
    
  thermode.requestTemperatures();                                         
//tem_outdoor = thermode.getTempCByIndex(0);
  pres_outdoor = bmp280.readPressure();
  hum_outdoor =  si7021.getRH();
  tem_bmp280 = bmp280.readTemperature();
  tem_outdoor =  tem_bmp280; 
  tem_outdoor =  tem_outdoor + correction;

  
  /*    
      Serial.print(tem_outdoor); 
      Serial.print("   "); 
      Serial.print(pres_outdoor); 
      Serial.print("   "); 
      Serial.println(hum_outdoor); 
  */
  pres_outdoor = pres_outdoor*0.00750063755419211;
  client.publish("outdoor/tem",String(tem_outdoor));  
  client.publish("outdoor/bmp280",String(tem_bmp280));
  client.publish("outdoor/cor",String(correction));
  client.publish("outdoor/pres",String(pres_outdoor));   
  client.publish("outdoor/hum",String(hum_outdoor));  
  }
delay(50);  
}
