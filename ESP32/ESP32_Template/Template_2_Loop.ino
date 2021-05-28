void loop()
{
//Разрешение подсчетов реконнектов WiFI (без учета перезагрузок)
res_flag = true;

//Запуск OTA
ArduinoOTA.handle();

//Запуск MQTT
client.loop();

//Запуск WiFiSerial если позволяет флаг
if (wt_ota_flag)
{
//WiFiSerial проверка подключения клиентов
    if (TelnetServer.hasClient())
    {
        if (!TelnetClient || !TelnetClient.connected())
        {
            if(TelnetClient) TelnetClient.stop();
            TelnetClient = TelnetServer.available();
            TelnetClient.println("> Your IP: " + TelnetClient.remoteIP().toString());
        }
        else TelnetServer.available().stop();      
    }

//WiFiSerial получение строки, если она есть. Вызов Terminal для обработки с параметром "WiFi"
    if (TelnetClient && TelnetClient.connected() && TelnetClient.available()) {while(TelnetClient.available()) Terminal(TelnetClient.readStringUntil('\n'), "WiFi");}
}



//BTSerial получение строки, если она есть. Вызов Terminal для обработки с параметром "BT"
if (SerialBT.available()) Terminal(SerialBT.readStringUntil('\n'), "BT");



//Таймер 2 сек
if (millis() - count_ms_Timer2 > time_ms_Timer2)
{
count_ms_Timer2 = millis();
}



//Таймер 5 сек
if (millis() - count_ms_Timer5 > time_ms_Timer5)
{
count_ms_Timer5 = millis();

//Переподключение при обрыве связи MQTT
if (!client.connected()) if (WiFi.status() == WL_CONNECTED) MQTT_reconnect();
}



//Таймер 60 сек
if (millis() - count_ms_Timer60 > time_ms_Timer60)
{
count_ms_Timer60 = millis();
//Переподключение при обрыве связи WiFi
if (WiFi.status() != WL_CONNECTED) WiFi_reconnect();

//Обновление времени с NTP
GetTime();
}



//Сброс сторожевого таймера
timerWrite(timer, 0);
}
