void connectToMqtt()
{
Serial.println("Connecting to MQTT");
mqttClient.connect();
}

void onMqttConnect(bool sessionPresent)
{
Serial.println("Connected to MQTT");
mqttClient.subscribe("test", 0);
mqttClient.publish("test", 0, true, "test qos-0");
mqttClient.publish("test", 0, true, "test qos-1"); 
mqttClient.publish("test", 0, true, "test qos-2");
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) 
{
Serial.println("Disconnected from MQTT.");
if (WiFi.isConnected()) xTimerStart(timerMqttReconnect, 0);
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
Serial.println("Subscription confirmed, packetId: " + String(packetId) + "  qos: " + String(qos));
}

void onMqttUnsubscribe(uint16_t packetId)
{
Serial.println("Unsubscribe confirmed, packetId: " + String(packetId));
}

void onMqttPublish(uint16_t packetId)
{
Serial.println("Publication confirmed, packetId: " + String(packetId));
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
String Payload = String (payload).substring(0, len);
Serial.println("topic: " + String(topic) + "  | payload: " + Payload);
}
