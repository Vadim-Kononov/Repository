#include <ESP8266WiFi.h>
#include <OneWire.h>                  // Библиотека протокола 1-Wire
#include <DallasTemperature.h>        // Библиотека для работы с датчиками DS*

#define ONE_WIRE_BUS 14               // Шина данных на 10 пине

OneWire oneWire(ONE_WIRE_BUS);        // Создаем экземпляр объекта протокола 1-WIRE - OneWire
DallasTemperature sensors(&oneWire);  // На базе ссылки OneWire создаем экземпляр объекта, работающего с датчиками DS*

void setup(void)
{
  Serial.begin(9600);                 // Настраиваем Serial для отображения получаемой информации
  sensors.begin();                    // Запускаем поиск всех датчиков
}

void loop(void)
{ 
 // Serial.println("Requesting temperatures...");
  sensors.requestTemperatures();      // Запускаем измерение температуры на всех датчиках
  
  // Когда температура измерена её можно вывести
  // Поскольку датчик всего один, то запрашиваем данные с устройства с индексом 0
 
  Serial.print("Т1: ");
  Serial.print(sensors.getTempCByIndex(0));
  Serial.print(sensors.getResolution());
  Serial.print("   ");
  Serial.print("Т2: ");
  Serial.print(sensors.getTempCByIndex(1));
  
}
