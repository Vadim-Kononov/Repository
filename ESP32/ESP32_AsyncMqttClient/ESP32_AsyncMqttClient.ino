#include <WiFi.h>
extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

#include "Account.h"

#include <AsyncMqttClient.h>
AsyncMqttClient mqttClient;

TimerHandle_t timerMqttReconnect;
TimerHandle_t timerWiFiReconnect;


void setup() {
  Serial.begin(115200);
  Serial.println();
  randomSeed(micros());
   
  
  timerMqttReconnect = xTimerCreate("timerMqttReconnect", pdMS_TO_TICKS(5000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  timerWiFiReconnect = xTimerCreate("timerWiFiReconnect", pdMS_TO_TICKS(5000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USERNAME, MQTT_PASS);
  mqttClient.setClientId(NAME);
  connectToWifi();
}

void loop() {
}
