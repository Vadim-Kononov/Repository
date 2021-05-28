void setup()
{
//Отключение brownout detector
WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

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

/*Запуск таймера малого цикла*/
xTimerStart(timerCycle_A, 0);
/*Запуск таймера большого цикла*/
xTimerStart(timerCycle_B, 0);
/*Запуск сторожевого таймера*/
xTimerStart(timerWatchDog, 0);

/*Инкремент счетчика перезагрузок*/
memory.putInt("countReset", memory.getInt("countReset") + 1);

/*Отправка IFTTT сообщения о перезагрузке*/
IFTTTSend (String(ifttt_event), String(board_name) + " " + String(F("Reloading ")), String("Res:") + String (memory.getInt("countReset")), String(" WiFi:") + String (memory.getInt("countWifi")));
}
