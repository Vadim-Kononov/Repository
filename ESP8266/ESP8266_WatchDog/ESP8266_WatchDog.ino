/* 
D0   GPIO(16)
D1   GPIO(5)
D2   GPIO(4)
D3   GPIO(0)
D4   GPIO(2) Встроенный светодиод
D5   GPIO(14)
D6   GPIO(12)
D7   GPIO(13)
D8   GPIO(15)
D9   GPIO(3)
D10  GPIO(1)
*/

#define D6  12            //Вход
#define LED 2             //Встроенный светодиод
#define D3  0             //Reset
#include <ESP8266WiFi.h>

unsigned long previousMillis_Long , interval_Long; 
unsigned long previousMillis_Short, interval_Short;
boolean reset_flag;

void setup()
{
Serial.begin(9600);
WiFi.persistent(false);  
WiFi.forceSleepBegin();  
//Serial.println("Restart"); 

pinMode (D3,OUTPUT);
pinMode (LED,OUTPUT);
pinMode (D6,INPUT_PULLUP);

digitalWrite (D3, LOW);                                        // формирование сигнала сброса на D5 при перезагрузке
delay (1000);
digitalWrite (D3, HIGH);

reset_flag = false;

interval_Long = 259200000;                                     // трое суток длинный цикл
interval_Short = 30000;                                        // 30 сек. короткий цикл
previousMillis_Long = millis();   
previousMillis_Short = millis();

attachInterrupt (digitalPinToInterrupt (D6), Input, CHANGE);  // преравание по изменению сигнала на D6
}

void loop() 
{
if (millis() - previousMillis_Short > interval_Short)
{
if (reset_flag) ESP.restart();                                // перезагрузка в коротком цикле если не было прерываний на D6
previousMillis_Short = millis();                              // восстановление счетчика
reset_flag = true;                                            // взвод флага перезагрузки
}      
  
if (millis() - previousMillis_Long > interval_Long)           // безусловная перезагрузка в длинном цикле
{
ESP.restart();                                             
}
delay (50);
}

void Input ()
{
reset_flag = false;
digitalWrite (LED, digitalRead(D6));
}
