/*Строковые константы*/
const char * string_loop [] PROGMEM = 
{
"My IP: ",          //0
"   |   Your IP: ", //1
};

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

/*Small_Cycle_Function, длительные операции*/
if (flag_Small_Cycle_Function)
{
     flag_Small_Cycle_Function = false;
     /*Обновление NTP времени*/
     GetTime();
}

/*Big_Cycle_Function, длительные операции*/
#if   (ROLE == 1)
if (flag_Big_Cycle_Function)
{
    flag_Big_Cycle_Function = false;
    WiFiReconnect ();
    if (flag_WiFi_connected) ThingSpeakSend ();     
}
#endif

}
