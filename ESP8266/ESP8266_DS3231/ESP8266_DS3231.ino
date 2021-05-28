// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

bool flag = true;

int Relay [4] = {16, 14, 12, 13};
int sec;
int i = 0;
void setup () {
  Serial.begin(9600);
  pinMode(0, OUTPUT); 


   pinMode(16, OUTPUT);
   pinMode(14, OUTPUT); 
   pinMode(12, OUTPUT); 
   pinMode(13, OUTPUT); 

  digitalWrite(16, HIGH);
  digitalWrite(14, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);

/*
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
*/
rtc.begin();
//DateTime corr = rtc.now();
//rtc.adjust (corr + TimeSpan(0,0,0,0));

}


void loop () {
    DateTime now = rtc.now();
    
    if (sec!=now.second())
    {
      sec = now.second();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    flag = !flag;
    digitalWrite(0, flag);
    Serial.println (i);
    digitalWrite(Relay[i], LOW);
    i = i+1;
    if (i>=5) {i = 0; digitalWrite(Relay[0], HIGH); digitalWrite(Relay[1], HIGH); digitalWrite(Relay[2], HIGH); digitalWrite(Relay[3], HIGH);Serial.println ("HIGH");}
  
  
    }
    delay(250);
}
