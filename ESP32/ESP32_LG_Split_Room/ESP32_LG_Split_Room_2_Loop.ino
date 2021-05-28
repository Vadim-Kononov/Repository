void loop()
{
  //Разрешение подсчетов реконнектов WiFI
  res_flag = true;

  //Запуск OTA
  ArduinoOTA.handle();

  //Запуск MQTT
  client.loop();

  //Получение строки с SerialBT, если она есть
  if (SerialBT.available())
  {
    Terminal(SerialBT.readStringUntil('\n'));
  }


  // Таймер 1 сек
  if (millis() - count_ms_Timer1 > time_ms_Timer1)
  {
  count_ms_Timer1 = millis();

  // Организация работы обратного таймера
  if (timer_flag)
  {
    if (timer_sec <=0 ) {timer_flag = false; SendMqtt("split/timer_min_set",String(timer_min_set)); timer_min = timer_min_set; timer_sec = timer_min*60; Ac_Send_Code(10); Ac_Send_Code(1); memory.putBool("timer_flag", timer_flag); timer_symbol = "";}
    else {timer_sec--; timer_symbol = "🔴     ";}
  } 
  }



  //Таймер 2 сек
  if (millis() - count_ms_Timer2 > time_ms_Timer2)
  {
    count_ms_Timer2 = millis();

    //Переподключение при обрыве связи
    if (!client.connected()) if (WiFi.status() == WL_CONNECTED) MQTT_reconnect();

    //Отправка в топик
    SendMqtt ("split/timer", timer_symbol + Hour_display(timer_sec));

    // Измерение и публикация
    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    BME280::PresUnit presUnit(BME280::PresUnit_Pa);                   
    bme.read(pres_split, tem_BME280, hum_split, tempUnit, presUnit);
    tem_split =  tem_BME280 + correction;
    pres_split = pres_split*0.00750063755419211;
  
    if (split_flag) tem_min = tem_split - tem_stored ; else tem_min = 0;
  
    SendMqtt("split/tem",String(tem_split));
    SendMqtt("split/bme280",String(tem_BME280));
    SendMqtt("split/tem_min",String(tem_min));
    SendMqtt("split/cor",String(correction));
    SendMqtt("split/pres",String(pres_split));
    SendMqtt("split/hum",String(hum_split));
  }



  //Таймер 60 сек
  if (millis() - count_ms_Timer60 > time_ms_Timer60)
  {
    count_ms_Timer60 = millis();

    //Переподключение при обрыве связи
    if (WiFi.status() != WL_CONNECTED) WiFi_reconnect();

    //Обновление времени с NTP
    GetTime();
   
    //Сохранение текущих минут
    memory.putInt("timer_min_mem", timer_sec/60); 
    
    //Отправка в канал
    SendThingSpeak(myWriteAPIKey, String(tem_split), "", "", "", "", "", "", "");

  }



  // Вывод информации на дисплей
  u8g2.firstPage();                                                                   
  do
  {
  u8g2.setFont(u8g2_font_profont17_tf);
  u8g2.setCursor(9, 18);
  u8g2.print(tem_split); 
  u8g2.setFont(u8g2_font_profont11_tf);
  u8g2.setCursor(14, 31);
  u8g2.print(String(pres_split));  
  u8g2.setCursor(18, 43);
  u8g2.print(String(hum_split));
  u8g2.drawFrame(0,0,u8g2.getDisplayWidth(),u8g2.getDisplayHeight() );
  } 
  while (u8g2.nextPage());
  

  
  //Сброс сторожевого таймера
  timerWrite(timer, 0);
}
