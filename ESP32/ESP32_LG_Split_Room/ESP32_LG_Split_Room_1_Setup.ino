void setup()
{
#ifdef Serial_On
  Serial.begin(115200);
  Serial.println("Booting");
#endif

  //Настройка встроенного светодиода
  pinMode(Onboard_Led, OUTPUT);
  
  //Инициализация EEPROM
  memory.begin("memory", false);

  //Восстановление параметров таймера
  timer_min_set = memory.getInt("timer_min_set");
  timer_min_mem = memory.getInt("timer_min_mem");
  timer_flag = memory.getBool("timer_flag"); 
  timer_sec = timer_min_mem*60;
  
  //Подключение к WiFi
  WiFi_reconnect();

  //Подключение к MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  //Подключение к OTA
  ArduinoOTA.setHostname(NAME);
  ArduinoOTA.begin();

  //Подключение к BluetoothSerial
  SerialBT.begin(NAME);

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

  // Подключение к BME280
  Wire.begin();
  bme.begin();
  
  // Подключение к IR излучателю
  irsend.begin();
  
  // Подключение к дисплею 68x46
  u8g2.begin();
  u8g2.enableUTF8Print();
  
  //Отправка сообщения
  SendIFTTT(KEY, EVENT_NAME1, NAME + String(".") + String (memory.getInt("countReset")), print_reset_reason(rtc_get_reset_reason(0)), print_reset_reason(rtc_get_reset_reason(1)));

  // Установка таймеров 60, 2, 1 сек
  count_ms_Timer60 = millis();                                 
  time_ms_Timer60 = 60000;
  count_ms_Timer2 = millis();                                
  time_ms_Timer2 = 2000;                                      
  count_ms_Timer1 = millis();                                
  time_ms_Timer1 = 1000;

#ifdef Serial_On
  Serial.println("End of Setup");
#endif
}
