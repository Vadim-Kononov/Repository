void loop()
{
if (flag_OTA_pass) ArduinoOTA.handle();

/*Подключения клиента Telnet*/
if (TelnetServer.hasClient())
{
if(TelnetClient) TelnetClient.stop(); TelnetClient = TelnetServer.available(); flag_OTA_pass = false;
TelnetClient.println("My IP:\t" + WiFi.localIP().toString() + "\t|\tYour IP:\t" + TelnetClient.remoteIP().toString());
}

/*Получение строки из Telnet, при ее наличии. Отправка строки в Terminal для обработки*/
if (TelnetClient && TelnetClient.connected() && TelnetClient.available()) {while(TelnetClient.available()) Terminal(TelnetClient.readStringUntil('\n'), B010);}

/*Получение строки из Bluetooth, при ее наличии. Отправка строки в Terminal для обработки*/
if (SerialBT.available()) Terminal(SerialBT.readStringUntil('\n'), B100);



/*Cycle_A, длительные операции*/
if (flag_Cycle_A)
{
flag_Cycle_A = false;
GetTime();
/*Сброс сторожевого таймера*/
xTimerReset(timerWatchDog, 0);
}
/**/



/*Cycle_B, длительные операции*/
if (flag_Cycle_B)
{
flag_Cycle_B = false;
WiFiReconnect ();
randomSeed(micros());
/*>-----------< Выбор Роли >-----------<*/
/*Отправка данных на ThingSpeak для платы Main*/
if (ROLE == 0) ThingSpeakSend ();
/*>-----------< Выбор Роли >-----------<*/
}
/**/



}
