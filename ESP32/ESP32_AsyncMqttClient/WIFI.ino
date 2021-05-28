void connectToWifi() 
{
  Serial.println("Connecting to Wi-F");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}



void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) 
    {
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        xTimerStop(timerMqttReconnect, 0);
        xTimerStart(timerWiFiReconnect, 0);
        break;
    }
}
