void setup()
{
#if   (DEBUG == 1)
Serial.begin(115200);
#endif

/*Инициализация NVS памяти*/
memory.begin("memory", false);
/*Удаление лишних ключей*/
//memory.remove("name");

/*Первоначальное (первое для платы) сохранение логина и пароля WiFi закомментировано*/
//memory.putString("login", "One"); memory.putString("pass", "73737373");

/*Назначение функции WiFiEvent*/
WiFi.onEvent(WiFiEvent);

/*Подключение к WiFi*/
WiFiConnect();

/*Инициализация  ESPNOW*/
if (esp_now_init() != ESP_OK) {DEBUG_PRINTLN(String(F("Error initializing ESP-NOW"))); return;}

/*Регистрация функции обратного вызова при отправке данных ESPNOW*/
esp_now_register_send_cb(Sending_Complete_Now);

/*Регистрация функции обратного вызова при приеме данных ESPNOW*/
esp_now_register_recv_cb(Receiv_Now);

/*>-----------< Выбор Роли >-----------<*/
#if   (ROLE == 0)
/*Запись информации о пире Slave_1*/
memcpy(peerInfo.peer_addr, mac_Slave_1, 6); peerInfo.channel = 0; peerInfo.encrypt = false;
/*Добавление Slave_1 в список*/
if (esp_now_add_peer(&peerInfo) != ESP_OK) {DEBUG_PRINTLN(String(F("Failed to add peer"))); return;}
/*Запись информации о пире Slave_2*/
memcpy(peerInfo.peer_addr, mac_Slave_2, 6); peerInfo.channel = 0; peerInfo.encrypt = false;
/*Добавление  Slave_2 в список*/
if (esp_now_add_peer(&peerInfo) != ESP_OK) {DEBUG_PRINTLN(String(F("Failed to add peer"))); return;}
/*>-----------< Выбор Роли >-----------<*/
#elif (ROLE == 1 || ROLE == 2)
/*Запись информации о пире Main*/
memcpy(peerInfo.peer_addr, mac_Main_0, 6); peerInfo.channel = 0; peerInfo.encrypt = false;
/*Добавление пира Main в список*/
if (esp_now_add_peer(&peerInfo) != ESP_OK) {DEBUG_PRINTLN(String(F("Failed to add peer"))); return;}
#endif
/*>-----------< Выбор Роли >-----------<*/

/*Инициализация Bluetooth*/
SerialBT.begin(board_name[ROLE]);

/*Инициализация Telnet*/
TelnetServer.begin();
TelnetServer.setNoDelay(true);

/*Назначение функций и параметров OTA*/
ArduinoOTA.setHostname(board_name[ROLE]);
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
mqttClient.setClientId(board_name[ROLE]);

/*Получение времени с NTP сервера*/
GetTime();

/*Запись номера платы в структуру*/
now_Store[ROLE].unit = ROLE;

/*Запуск таймера малого цикла*/
xTimerStart(timerCycle_A, 0);
/*Запуск таймера большого цикла*/
xTimerStart(timerCycle_B, 0);
/*Запуск сторожевого таймера*/
xTimerStart(timerWatchDog, 0);

/*Инкремент счетчика перезагрузок*/
memory.putInt("countReset", memory.getInt("countReset") + 1);

/*Отправка IFTTT сообщения о перезагрузке*/
IFTTTSend (String(ifttt_event), String(board_name[ROLE]) + " " + String(F("Reloading ")), String("Res:") + String (memory.getInt("countReset")), String(" WiFi:") + String (memory.getInt("countWifi")));
}
