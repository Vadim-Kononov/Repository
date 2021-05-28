// Arduino Nano ATmega328 РЅР° COM6

#include <SoftwareSerial.h>

#define Pin_HC12_RX       16
#define Pin_HC12_TX      5

SoftwareSerial HC12(Pin_HC12_TX, Pin_HC12_RX);
String Stroka1  = "Test "; 
String Stroka2  = "Stroka2 "; 
String Stroka3  = "Stroka3 "; 
String Stroka4  = "Stroka4 "; 

int i = 0;

void setup()
{
  Serial.begin(9600);
  HC12.begin(9600);
  Serial1.begin (9600);
 
  
}


void loop()
{
  HC12.println(Stroka1 + i);
  Serial.println(Stroka2 + i);
  Serial1.println(Stroka3 + i);
 
  
  i++;
  delay (500);
}
