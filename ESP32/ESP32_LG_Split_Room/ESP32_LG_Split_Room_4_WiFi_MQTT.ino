//Подключение к WiFi
void WiFi_reconnect()
{
#ifdef Serial_On
  Serial.print("\nConnecting to " + String(ssid));
#endif

  //Получение хранящегося логина и пароля WiFi и перевод их в массивы char[]
  memory.getString("login").toCharArray(ssid, memory.getString("login").length() + 1);
  memory.getString("pass").toCharArray(password, memory.getString("pass").length() + 1);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setHostname(NAME);

  //Инициализация счетчика попыток подключения
  int count = 0;

  while (WiFi.status() != WL_CONNECTED)
  {

#ifdef Serial_On
    Serial.print (".");
#endif

    //Если попыток больше Connection_Count_WiFi, то прекратить
    count++; if (count >= Connection_Count_WiFi) break;
    delay(500);
  }

  //Увеличение счетчика реконнектов к WiFi на 1, если это не перезагрузка модуля
  if (res_flag) {
    memory.putInt("countWifi", memory.getInt("countWifi") + 1);
    SendIFTTT(KEY, EVENT_NAME2, NAME, String (memory.getInt("countReset")), String(memory.getInt("countWifi")));
  }

#ifdef Serial_On
  if (WiFi.status() != WL_CONNECTED) Serial.print ("\nНе удалось подключиться к WiFI");
  else Serial.print ("\nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
#endif
}



//Сканирование WiFi
void WiFiScan ()
{
  int n = WiFi.scanNetworks();
  if (n == 0) {
    SerialBT.println("Сети не найдены");
  }
  else {
    for (int i = 0; i < n; ++i) {
      SerialBT.print(i + 1);
      SerialBT.print(". ");
      SerialBT.print(WiFi.SSID(i));
      SerialBT.print(" (");
      SerialBT.print(WiFi.RSSI(i));
      SerialBT.println(") ");
    }
  }
}



//Подключение к MQTT
void MQTT_reconnect()
{
  while (!client.connected())
  {

#ifdef Serial_On
    Serial.println("MQTT connection to " + String(mqtt_server) + ", wait...");
#endif

    if (client.connect(mqtt_device.c_str(), mqtt_user, mqtt_pass))
    {
#ifdef Serial_On
      Serial.println("Connected user: " + String(mqtt_user));
#endif

      //Подписка на топики
      client.subscribe("split/dec");                                                
      client.subscribe("split/inc");
      client.subscribe("split/on");
      client.subscribe("split/off");
      client.subscribe("split/timer_on");
      client.subscribe("split/timer_inc");
      client.subscribe("split/timer_dec");
      client.subscribe("split/timer_min_set");
      client.subscribe("split/speed");       
   
    }

#ifdef Serial_On
    else {
      Serial.print("Connected failed, rc= ");
      Serial.println(client.state());
    }
#endif
  }
}



//Чтение сообщений брокера
void callback(char* top, byte* pay, unsigned int length)
{
  digitalWrite(Onboard_Led, HIGH);
  //Имя топика
  String topic = top;

  //Сообщение не длинее 50 символов, преобразование byte[] в char[], преобразование char[] в String
  char string[50];
  memcpy (string, pay, length);
  string[length] = '\0';
  String payload = string;
  
  // Обработка принятых сообщений
  if (topic == "split/dec")           {if (payload.toInt()) {correction = correction - 0.1; memory.putFloat("correction", correction);}}         
  if (topic == "split/inc")           {if (payload.toInt()) {correction = correction + 0.1; memory.putFloat("correction", correction);}}
  if (topic == "split/on")            {if (payload.toInt() == 1) Ac_Send_Code(0);}
  if (topic == "split/off")           {if (payload.toInt() == 1) {SendMqtt("split/timer_min_set",String(timer_min_set)); timer_flag = false; memory.putBool("timer_flag", timer_flag); timer_min = timer_min_set; timer_sec = timer_min*60; Ac_Send_Code(10); Ac_Send_Code(1); timer_symbol = "";}}  
  if (topic == "split/timer_on")      {if (payload.toInt() == 1) timer_flag = !timer_flag; memory.putBool("timer_flag", timer_flag); if (!timer_flag) {Ac_Send_Code(10); timer_symbol = "";} else {Break_Time (timer_sec);}}  
  if (topic == "split/timer_inc")     {if (payload.toInt() == 1) {timer_sec = timer_sec + 5*60; memory.putInt("timer_min_mem", timer_sec/60); if (timer_flag) Break_Time(timer_sec);}}
  if (topic == "split/timer_dec")     {if (payload.toInt() == 1) {if (timer_sec > 300) timer_sec = timer_sec - 5*60; memory.putInt("timer_min_mem", timer_sec/60); if (timer_flag) Break_Time(timer_sec);}}
  if (topic == "split/timer_min_set") {timer_min_set = payload.toInt(); memory.putInt("timer_min_set", timer_min_set); timer_min = timer_min_set; timer_sec = timer_min*60; if (timer_flag) Break_Time(timer_sec);}
  if (topic == "split/speed")         {Ac_Send_Code(payload.toInt());}
  
  digitalWrite(Onboard_Led, LOW);
}


//Отправка сообщений брокеру
void SendMqtt (String topic, String payload)
{
  digitalWrite(Onboard_Led, HIGH);
  //преобразование String в char[]
  char top [50]; char msg [50];
  topic.toCharArray(top, topic.length() + 1);
  payload.toCharArray(msg, payload.length() + 1);

  client.publish(top, msg);
  digitalWrite(Onboard_Led, LOW);
}
