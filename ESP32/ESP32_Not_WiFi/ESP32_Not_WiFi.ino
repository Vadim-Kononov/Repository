#include <WiFi.h>
#define LED_BUILTIN 2   

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println();
  WiFi.disconnect();
  Serial.println("Setup");
 }

void loop() 
{
  
}
