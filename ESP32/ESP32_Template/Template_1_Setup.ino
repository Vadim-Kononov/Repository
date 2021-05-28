void setup()
{
//Отключение brownout detector
WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

#ifdef Serial_On
Serial.begin(115200);
Serial.println("Booting");
#endif

//Настройка встроенного светодиода
pinMode(Onboard_Led, OUTPUT);

//Инициализация EEPROM
memory.begin("memory", false);

//Подключение к WiFi
WiFi_reconnect();

//Подключение к Telnet WiFiSerial
TelnetServer.begin();
TelnetServer.setNoDelay(true);

// UDP Multicast рассылка для группы 239.0.0.0, порт 59900
udp.listenMulticast(IPAddress(239,0,0,0), 59900);

//Подключение к BTSerial
SerialBT.begin(NAME);

//Подключение к MQTT
client.setServer(mqtt_server, mqtt_port);
client.setCallback(callback);
//Формирование случайного числа для логина
randomSeed(micros());

//Подключение к OTA
ArduinoOTA.setHostname(NAME);
ArduinoOTA.begin();

//Настройка WDT
timer = timerBegin(0, 80, true);                  //Timer 0, div 80
timerAttachInterrupt(timer, &ResetModule, true);  //Attach callback
timerAlarmWrite(timer, wdtTimeout * 1000, false); //Set time in us
timerAlarmEnable(timer);                          //Enable interrupt

//Получение времени с NTP сервера
digitalWrite(Onboard_Led, HIGH);
GetTime();
digitalWrite(Onboard_Led, LOW);

//Увеличение счетчика перезагрузок на 1
memory.putInt("countReset", memory.getInt("countReset") + 1);

//Отправка сообщения о перезагрузке
SendIFTTT(KEY, EVENT_NAME1, NAME + String(" >") + String (memory.getInt("countReset")) + String("<"), print_reset_reason(rtc_get_reset_reason(0)), print_reset_reason(rtc_get_reset_reason(1)));

//Установка таймеров 60, 5 и 2 сек
count_ms_Timer60 = millis(); time_ms_Timer60 = 60000;
count_ms_Timer5 = millis(); time_ms_Timer5 = 5000;
count_ms_Timer2 = millis(); time_ms_Timer2 = 2000;
}
