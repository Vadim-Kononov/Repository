/*Строковые константы*/
const char * string_setup [] PROGMEM = 
{
"Error initializing ESP-NOW", //0
"Failed to add peer",         //1
"Exch_Main Reloading   ",     //2
"Res:",                       //3
"   WiFi:"                    //4
};

void setup()
{
#if   (DEBUG == 1)
Serial.begin(115200);
#endif

/*Инициализация NVS памяти*/
memory.begin("memory", false);
/*Удаление лишних ключей*/
//memory.remove("name");

/*Инкримент счетчика перезагрузок на 1*/
memory.putInt("countReset", memory.getInt("countReset") + 1);

/*Первоначальное (первое для платы) сохранение логина и пароля WiFi закомментировано*/
/*memory.putString("login", "One"); memory.putString("pass", "73737373");*/

/*Назначение функции WiFiEvent*/
WiFi.onEvent(WiFiEvent);

/*Подключение к WiFi*/
WiFiConnect();

/*Инициализация  ESPNOW*/
if (esp_now_init() != ESP_OK) {DEBUG_PRINTLN(string_setup [0]); return;}

/*Регистрация функции обратного вызова при отправке данных ESPNOW*/
esp_now_register_send_cb(Now_Sending_Complete);

/*Регистрация функции обратного вызова при приеме данных ESPNOW*/
esp_now_register_recv_cb(Now_Receiving_Complete);

/*Запись информации о пире*/
esp_now_peer_info_t peerInfo;
memcpy(peerInfo.peer_addr, broadcastAddress, 6);
peerInfo.channel = 0;
peerInfo.encrypt = false;

/*Добавление пира в список*/
if (esp_now_add_peer(&peerInfo) != ESP_OK) {DEBUG_PRINTLN(string_setup [1]); return;}

/*Инициализация Bluetooth*/
SerialBT.begin(board_name);

/*Инициализация Telnet*/
TelnetServer.begin();
TelnetServer.setNoDelay(true);

/*Назначение функций и параметров OTA*/
ArduinoOTA.setHostname(board_name);
ArduinoOTA.onStart(TimerStop);

/*Назначение функций и параметров MQTT*/
mqttClient.onConnect(mqtt_Connected_Complete);
mqttClient.onDisconnect(mqtt_Disconnect_Complete);
mqttClient.onSubscribe(mqtt_Subscribe_Complete);
mqttClient.onUnsubscribe(mqtt_Unsubscribe_Complete);
mqttClient.onMessage(mqtt_Receiving_Complete);
mqttClient.onPublish(mqtt_Publishe_Complete);
mqttClient.setServer(mqtt_host, mqtt_port);
mqttClient.setCredentials(mqtt_username, mqtt_pass);
mqttClient.setClientId(board_name);

/*Получение времени с NTP сервера*/
GetTime();

/*Инициализация WDT*/
timerWDT = timerBegin(0, 80, true);
timerAttachInterrupt(timerWDT, &ResetModule, true);
timerAlarmWrite(timerWDT, 30000 * 1000, false);
timerAlarmEnable(timerWDT);

/*Отправка сообщения о перезагрузке*/
#if   (ROLE == 1)
if (flag_WiFi_connected)
IFTTTSend (String(ifttt_event), string_setup [2], String(string_setup [3]) + String (memory.getInt("countReset")), String(string_setup [4]) + String (memory.getInt("countWifi")));
#endif

/*Запуск таймера малого цикла*/
xTimerStart(timerSmall_Cycle_Function,0);
/*Запуск таймера большого цикла*/
xTimerStart(timerBig_Cycle_Function, 0);
}
