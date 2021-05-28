/*Строковые константы*/
const char * string_mqtt [] PROGMEM = 
{
"Connecting to MQTT...",              //0
"Connected to MQTT :)",               //1
"Disconnected from MQTT :(",          //2
"Subscription confirmed, packetId: ", //3
"  qos: ",                            //4
"Unsubscribe confirmed, packetId: ",  //5
"Publication confirmed, packetId: ",  //6
"\t\ttopic:\t\t",                     //7
"  | payload: ",                      //8
"Time:\t",                            //9
"\tmqtt_Send",                        //10
"\tmqtt_Receiv"                       //11
};

/*Функция подключения*/
void mqtt_Connect()
{
DEBUG_PRINTLN(string_mqtt [0]);
mqttClient.connect();
}
/**/

/*Обработчик события подключения*/
/*Здесь подиска на топики*/
void mqtt_Connected_Complete(bool sessionPresent)
{
flag_MQTT_connected = true;
DEBUG_PRINTLN(string_mqtt [1]);
mqttClient.subscribe("test1", 0);
mqttClient.subscribe("test2", 0);
mqttClient.subscribe("test3", 0);
}
/**/

/*Обработчик события обрыва связи*/
void mqtt_Disconnect_Complete(AsyncMqttClientDisconnectReason reason)
{
/*Флаг обрыва связи*/
flag_MQTT_connected = false;
DEBUG_PRINTLN(string_mqtt [2]);
}
/**/

/*Обработчик события подписки*/
void mqtt_Subscribe_Complete(uint16_t packetId, uint8_t qos)
{
DEBUG_PRINTLN(String(string_mqtt [3]) + String(packetId) + String(string_mqtt [4]) + String(qos));
}
/**/

/*Обработчик события отписки*/
void mqtt_Unsubscribe_Complete(uint16_t packetId)
{
DEBUG_PRINTLN(String(string_mqtt [5]) + String(packetId));
}
/**/

/*Обработчик события публикации*/
void mqtt_Publishe_Complete(uint16_t packetId)
{
DEBUG_PRINTLN(String(string_mqtt [6]) + String(packetId));
}
/**/

/*Обработчик события приема сообщения*/
/*Здесь обработка принятых сообщений*/
void mqtt_Receiving_Complete(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
String Payload = String (payload).substring(0, len);
DEBUG_PRINTLN(String(string_mqtt [9]) + String(xTaskGetTickCount()/1000.0) + "\t" + String(string_mqtt [11]) + String(string_mqtt [7]) + String(topic) + String(string_mqtt [8]) + String(Payload));
}
/**/

/*Функция отправки сообщений MQTT*/
void mqtt_Send ()
{
DEBUG_PRINTLN(String(string_mqtt [9]) + String(xTaskGetTickCount()/1000.0) + "\t" + String(string_mqtt [10]));
#if   (ROLE == 1)
if (flag_MQTT_connected)
{
mqttClient.publish("test1", 0, false, String(struct_Receiv.var1).c_str());
mqttClient.publish("test2", 0, false, String(struct_Receiv.var4).c_str());
mqttClient.publish("test3", 0, false, String(struct_Receiv.var7).c_str());
}
#endif
}
/**/
