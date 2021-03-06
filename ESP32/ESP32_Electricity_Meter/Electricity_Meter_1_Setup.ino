void setup()
{
//Отключение brownout detector
WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

#if   (DEBUG == 1)
Serial.begin(115200);
#endif

/*Инициализация NVS памяти*/
memory.begin("memory", false);

/*Закомментировано первоначальное (для первой загрузки на плату скетча) сохранение логина и пароля WiFi*/
//memory.putString("login", "login"); memory.putString("pass", "pass");

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
//Извлечение часа дня
strftime(myStr, 3, "%H", &timeinfo);
saved_hour = String(myStr).toInt();
//Извлечение дня даты
strftime(myStr, 3, "%d", &timeinfo);
saved_day = String(myStr).toInt();
 
/*>-----------< PZEM >-----------<*/
//Инициализация экрана
SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI);
TFT_Init ();
//Загрузка сохраненных в NVS значений переменных
energy_main_counter_value 	= memory.getFloat("main_counter", 0.0);
energy_before_value 		    = memory.getFloat("before", 0.0);
energy_correction_value     = memory.getFloat("correction", 0.0);
rate_before                 = memory.getFloat("rate_before", 0.0);

day_before 					        = memory.getInt("day", day_before);
month_before 				        = memory.getInt("month", month_before);
delay_scroll 				        = memory.getFloat("delay", delay_scroll);

price_low 					        = memory.getFloat("price1", price_low);
price_high 					        = memory.getFloat("price2", price_high);
energy_low_value 			      = memory.getFloat("low_value", energy_low_value);

alarm_power 				        = memory.getFloat("alarm_power", 1000.0);
alarm_rate 					        = memory.getFloat("alarm_rate", 10000.0);
//Стирание не используемых ключей
//memory.remove("");

/*>-----------< PZEM >-----------<*/

/*Запуск сторожевого таймера*/
xTimerStart(timerWatchDog, 0);

/*Инкремент счетчика перезагрузок*/
memory.putInt("countReset", memory.getInt("countReset") + 1);

/*Отправка IFTTT оповещения о перезагрузке*/
IFTTTSend (String(ifttt_event), String(board_name) + " " + String(F("Reboot")), String("Res.") + String (memory.getInt("countReset")), String("WiFi.") + String (memory.getInt("countWifi")));
}
