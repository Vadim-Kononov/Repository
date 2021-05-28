/*This example show how to ping a remote machine using it's hostname */

#include <WiFi.h>
#include <ESP32Ping.h>

const char* ssid     = "One";
const char* password = "73737373";
const char* remote_host = "www.google.com";

void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("WiFi connected with ip ");  
  Serial.println(WiFi.localIP());

  Serial.print("Pinging host ");
  Serial.println(remote_host);

  if(Ping.ping(remote_host)) Serial.println("Success!!"); else Serial.println("Error :(");
}

void loop()
{

  if(Ping.ping(remote_host, 1)) Serial.println(Ping.averageTime()); else Serial.println("Error :(");
  }
  
}
