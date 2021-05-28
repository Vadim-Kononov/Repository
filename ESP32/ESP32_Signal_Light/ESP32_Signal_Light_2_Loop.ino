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



//-----------------------------------------------------------------------Signal_Light-----------------------------------------------------------------------//
//Прием байта по UDP от Signal System
udp.onPacket([](AsyncUDPPacket packet)
{
situation = packet.data()[0];

if (bit_siren != bitRead(situation, 0))
{
  bit_siren = bitRead(situation, 0);
  if (bit_siren) Setting(1800, 1.5, 80); else Setting(0, 1.5, 80);
}

if (bit_on_off != bitRead(situation, 1))
{
  bit_on_off = bitRead(situation, 1); 
  if (bit_on_off) Setting(2, 1.5, 80); else Setting(3, 1.5, 80); 
}
if (bit_light != bitRead(situation, 2))
{
  bit_light = bitRead(situation, 2);
  if (bit_light) Setting(600, 1.5, 100); else Setting(0, 1.5, 80);
}

if (bit_alarm != bitRead(situation, 3))
{
  bit_alarm = bitRead(situation, 3);
  Setting(5, 1, 80);
}

mqtt_flag = true;
});


//Таймер включения реле
if (millis() - count_ms_ON > time_ms_ON)
{
count_ms_ON = millis();
count_ms_OFF = millis();
if (cycle_counter >0) {cycle_counter --; digitalWrite(Onboard_Led, HIGH);}
else digitalWrite(Onboard_Led, LOW);
}


//Таймер отключения реле
if (millis() - count_ms_OFF > time_ms_OFF)
{
digitalWrite(Onboard_Led, LOW);
}


if (mqtt_flag)
{
mqtt_flag = false;  
//Отпрака строки состояния бит
SendMqtt ("signal/state", "#4" + String(bitRead(situation, 3)) + "&#3" + String(bitRead(situation, 2))  + "&#2" + String(bitRead(situation, 1))  + "&#1" + String(bitRead(situation, 0)) + "&");  
}
//-----------------------------------------------------------------------Signal_Light-----------------------------------------------------------------------//




//Сброс сторожевого таймера
timerWrite(timer, 0);
}


//-----------------------------------------------------------------------Signal_Light-----------------------------------------------------------------------//
//Прцедура задания полного времени включения в сек., времени одного цикла включения в сек. и скважности нахождения во включенном состоянии в %
void Setting (int total_light_time, float light_time, int duty_factor)
{
time_ms_ON = light_time * 1000;
time_ms_OFF = time_ms_ON / 100 * duty_factor + 5;
cycle_counter = total_light_time * 1000 / time_ms_ON;
}
//-----------------------------------------------------------------------Signal_Light-----------------------------------------------------------------------//
