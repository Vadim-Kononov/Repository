void setup()
{
#if   (DEBUG == 1)
Serial.begin(115200);
#endif

/*Инициализация NVS памяти*/
memory.begin("memory", false);

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
//ArduinoOTA.onStart(TimerStop);

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



/*>-----------< Sygnal System >-----------<*/

/*Настройка встроенного светодиода*/
pinMode(2, OUTPUT);

/*SIM800*/
Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
pinMode(SIM800_PWR, OUTPUT);
//Высокий уровень на RST SIM800
digitalWrite(SIM800_PWR, HIGH);
//AT для настройки скорости обмена данными
sendATCommand("AT", true);
//Отключить эхо, включить текстовый формат ответов, включить Text mode SMS, сохранить настройки (AT&W)
sendATCommand("ATE1V1+CMGF=1;&W", true);
//Удалить все сообщения, чтобы не забивали память модуля
sendATCommand("AT+CMGDA=\"DEL ALL\"", true);

//Случайное число в строку проверки
string_check_random = String(random(100, 1000));

//Инициализаця INA219
ina219.begin();

//Загрузка таблицы из NVS
memory.getBytes("mem", (detector*)detector_ram, sizeof(detector_ram));

//Включение приемника 433 МГц
receiver.enableReceive(RECEIVER_PIN);

//Установка флага питания от сети, установки пинов тревоги
pinMode(POWER_PIN, INPUT);
flag_power_220 = digitalRead(POWER_PIN);

pinMode(ALARM_RELAY_PIN_1, OUTPUT);
pinMode(ALARM_RELAY_PIN_2, OUTPUT);
digitalWrite(ALARM_RELAY_PIN_1, HIGH);
digitalWrite(ALARM_RELAY_PIN_2, HIGH);

/*>-----------< Sygnal System >-----------<*/



/*Инкремент счетчика перезагрузок*/
memory.putInt("countReset", memory.getInt("countReset") + 1);

/*Отправка IFTTT сообщения о перезагрузке*/
IFTTTSend (String(ifttt_event), String(board_name[ROLE]) + " " + String(F("Reloading ")), String("Res:") + String (memory.getInt("countReset")), String(" WiFi:") + String (memory.getInt("countWifi")));

/*Запуск таймера малого цикла*/
xTimerStart(timerCycle_A, 0);
/*Запуск таймера большого цикла*/
xTimerStart(timerCycle_B, 0);
/*Запуск сторожевого таймера*/
//xTimerStart(timerWatchDog, 0);



}
