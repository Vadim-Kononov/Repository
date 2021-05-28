/* 
SCL D1 GPIO(5)
SDA D2 GPIO(4)

Встроенный светодиод D4 GPIO(2)

D6 GPIO (12) приемник
D5 GPIO (14) передатчик

Blynk virtual pin
V1 терминал
V2 разрешение приема

V11 - V18 кнопки

*/

#define BLYNK_PRINT Serial
#define BUFFER_SIZE 100
#define Onboard_Led 2
#define WDT   0

#include <ESP8266WiFi.h>                                      // Основная библиотека               
#include <BlynkSimpleEsp8266.h>
#include <RCSwitch.h>
#include <EEPROM.h>                                           // Библиотека для работы с EEPROM

#include "Account.h"

WidgetTerminal terminal(V1);
RCSwitch mySwitch = RCSwitch();


long count_ms_Send_10000, time_ms_Send_10000 = 10000;     
long count_ms_Send_500, time_ms_Send_500 = 500;
int enable_Sending_RC = 0;
bool reception_allowed = false, flag_500 = true, flagWDT = true;                                                                                    



void setup()
{
  Serial.begin(9600);                                       
  mySwitch.enableReceive(12);                               // Приемник на D6  
  
  //mySwitch.setRepeatTransmit(15);
  // Optional set number of transmission repetitions.
  // Optional set pulse length.
  // mySwitch.setPulseLength(320);
  // Optional set protocol (default is 1, will work for most outlets)
  // mySwitch.setProtocol(2);
  
  Blynk.begin(auth, ssid, pass);
 
  count_ms_Send_10000 = millis();     
  count_ms_Send_500 = millis();
  
  pinMode(Onboard_Led, OUTPUT);
  pinMode(WDT, OUTPUT);
  
  delay (250); 
}



void loop() 
{
 Blynk.run();                                                                                       
  
 if (reception_allowed)
 {
  if (mySwitch.available())
  {  
    int value = mySwitch.getReceivedValue(); 
    if (value == 0) {terminal.println("Unknown encoding");}
    else {terminal.println("Code: " + String (mySwitch.getReceivedValue()) + " / " + String (mySwitch.getReceivedBitlength()) + " bit Protocol: " + String (mySwitch.getReceivedProtocol())); terminal.flush();}
    mySwitch.resetAvailable();
  }
 }
  
  if (millis() - count_ms_Send_10000 > time_ms_Send_10000)                    
  {
  count_ms_Send_10000 = millis(); Blynk.virtualWrite(V2, LOW); reception_allowed = false;
  }
  
  if (millis() - count_ms_Send_500 > time_ms_Send_500)                    
  {
  count_ms_Send_500 = millis(); flag_500 = true;
  }
  
  flagWDT = !flagWDT; digitalWrite(WDT, flagWDT); digitalWrite(Onboard_Led, flagWDT);
  }
//__________________________________________________________________________________________________________________




  



// Функция отправки RC 
String Sending_RC (int button)
{
  mySwitch.enableTransmit(14);                              // Передатчик на D5
  enable_Sending_RC = button;
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
  mySwitch.disableTransmit  ();
  enable_Sending_RC = 0;
  return String (value) + " - " + String (bit_n);
}


// Функция работы с терминалом
BLYNK_WRITE(V1)
{
  String str;
  int button;
  int bit_n, addr;
  long value;
  
  str = param.asStr();
    
  button = str.substring(0, str.indexOf(" ")).toInt();
  value = str.substring(str.indexOf(" ")+1, str.lastIndexOf(" ")).toInt();
  bit_n = str.substring(str.lastIndexOf(" ")+1, str.length()).toInt();
  
  if ((button>=1 && button<=8) && (bit_n>=10 && bit_n<=99))
  {
  addr = button*8;
  EEPROM.begin(512);                                  
  EEPROM.put(addr, bit_n);                                              
  EEPROM.put(addr+4, value); 
  EEPROM.end();
  }
  terminal.println("<x>␣<xxxxxxxx>␣<xx>"); terminal.println(); terminal.flush();
  for (button =1; button<=8; button++)
  {
  addr = button*8;
  EEPROM.begin(512);                                      
  EEPROM.get(addr, bit_n);    
  EEPROM.get(addr+4, value);  
  EEPROM.commit();                                       
  EEPROM.end();                                           
  terminal.println(String (button) + " " + String (value) + " " + String (bit_n)); terminal.flush();
  } 

}

// Функция разрешения приема
BLYNK_WRITE(V2)
{
if (param.asInt() == 1) {reception_allowed = true;} else {reception_allowed = false;}
count_ms_Send_10000 = millis();
}

BLYNK_WRITE(V11)
{
String str;
if (flag_500)
{ 
if (param.asInt() == 1 && enable_Sending_RC==0) {str = Sending_RC (1);  Blynk.setProperty(V11, "onLabel", str);}
else if (param.asInt() == 1 && enable_Sending_RC!=0) {Blynk.setProperty(V11, "onLabel", "On"); Blynk.virtualWrite(V11, HIGH); }
Blynk.virtualWrite(V2, LOW); reception_allowed = false;
flag_500 = false; count_ms_Send_500 = millis();
}
}

BLYNK_WRITE(V12)
{
String str;
if (flag_500)
{ 
if (param.asInt() == 1 && enable_Sending_RC==0) {str = Sending_RC (2);  Blynk.setProperty(V12, "onLabel", str);}
else if (param.asInt() == 1 && enable_Sending_RC!=0) {Blynk.setProperty(V12, "onLabel", "On"); Blynk.virtualWrite(V12, HIGH); }
Blynk.virtualWrite(V2, LOW); reception_allowed = false;
flag_500 = false; count_ms_Send_500 = millis();
}
}

BLYNK_WRITE(V13)
{
String str;
if (flag_500)
{ 
if (param.asInt() == 1 && enable_Sending_RC==0) {str = Sending_RC (3);  Blynk.setProperty(V13, "onLabel", str);}
else if (param.asInt() == 1 && enable_Sending_RC!=0) {Blynk.setProperty(V13, "onLabel", "On"); Blynk.virtualWrite(V13, HIGH); }
Blynk.virtualWrite(V2, LOW); reception_allowed = false;
flag_500 = false; count_ms_Send_500 = millis();
}
}


BLYNK_WRITE(V14)
{
String str;
if (flag_500)
{ 
if (param.asInt() == 1 && enable_Sending_RC==0) {str = Sending_RC (4);  Blynk.setProperty(V14, "onLabel", str);}
else if (param.asInt() == 1 && enable_Sending_RC!=0) {Blynk.setProperty(V14, "onLabel", "On"); Blynk.virtualWrite(V14, HIGH); }
Blynk.virtualWrite(V2, LOW); reception_allowed = false;
flag_500 = false; count_ms_Send_500 = millis();
}
}


BLYNK_WRITE(V15)
{
String str;
if (flag_500)
{ 
if (param.asInt() == 1 && enable_Sending_RC==0) {str = Sending_RC (5);  Blynk.setProperty(V15, "onLabel", str);}
else if (param.asInt() == 1 && enable_Sending_RC!=0) {Blynk.setProperty(V15, "onLabel", "On"); Blynk.virtualWrite(V15, HIGH); }
Blynk.virtualWrite(V2, LOW); reception_allowed = false;
flag_500 = false; count_ms_Send_500 = millis();
}
}


BLYNK_WRITE(V16)
{
String str;
if (flag_500)
{ 
if (param.asInt() == 1 && enable_Sending_RC==0) {str = Sending_RC (6);  Blynk.setProperty(V16, "onLabel", str);}
else if (param.asInt() == 1 && enable_Sending_RC!=0) {Blynk.setProperty(V16, "onLabel", "On"); Blynk.virtualWrite(V16, HIGH); }
Blynk.virtualWrite(V2, LOW); reception_allowed = false;
flag_500 = false; count_ms_Send_500 = millis();
}
}


BLYNK_WRITE(V17)
{
String str;
if (flag_500)
{ 
if (param.asInt() == 1 && enable_Sending_RC==0) {str = Sending_RC (7);  Blynk.setProperty(V17, "onLabel", str);}
else if (param.asInt() == 1 && enable_Sending_RC!=0) {Blynk.setProperty(V17, "onLabel", "On"); Blynk.virtualWrite(V17, HIGH); }
Blynk.virtualWrite(V2, LOW); reception_allowed = false;
flag_500 = false; count_ms_Send_500 = millis();
}
}


BLYNK_WRITE(V18)
{
String str;
if (flag_500)
{ 
if (param.asInt() == 1 && enable_Sending_RC==0) {str = Sending_RC (8);  Blynk.setProperty(V18, "onLabel", str);}
else if (param.asInt() == 1 && enable_Sending_RC!=0) {Blynk.setProperty(V18, "onLabel", "On"); Blynk.virtualWrite(V18, HIGH); }
Blynk.virtualWrite(V2, LOW); reception_allowed = false;
flag_500 = false; count_ms_Send_500 = millis();
}
}
