// Обработка строк из BluetoothSerial
void Terminal(String string)
{
  //Строка может состоять из трех подстрок разделенных одним пробелом, первая - prefix, вторая - login, третья - pass
  String prefix = string.substring(0, string.indexOf(" "));
  String login = string.substring(string.indexOf(" ") + 1, string.lastIndexOf(" "));
  String pass = string.substring(string.lastIndexOf(" ") + 1, string.length() - 1);

  if (key_flag) //Флаг ввода пароля, сбрасывается при перезагрузке
  {
    //Запись в EEPROM логина и пароля WiFi
    if (prefix.equalsIgnoreCase("Login")) {
      SerialBT.print ("> ");
      memory.putString("login", login);
      memory.putString("pass", pass);
      SerialBT.println(memory.getString("login") + " " + memory.getString("pass"));
    }
    //Вывод логина и пароля WiFi из памяти
    else if (string.equalsIgnoreCase("Mem\r")) {
      SerialBT.print("> ");
      SerialBT.println(memory.getString("login") + " " + memory.getString("pass"));
    }
    //Вывод текущих параметров WiFi
    else if (string.equalsIgnoreCase("WiFi\r")) {
      SerialBT.print ("> ");
      SerialBT.print(WiFi.SSID());
      SerialBT.print ("   ");
      SerialBT.print(WiFi.localIP());
      SerialBT.print ("   ");
      SerialBT.println(WiFi.RSSI());
    }
    //Сканирование WiFi
    else if (string.equalsIgnoreCase("Scan\r")) {
      SerialBT.println ("> Scan: ");
      WiFiScan ();
      SerialBT.println(">");
    }
    //Переподключение WiFi по логину и паролю хранящимися в памяти
    else if (string.equalsIgnoreCase("Reconnect\r")) {
      memory.getString("login").toCharArray(ssid, memory.getString("login").length() + 1);
      memory.getString("pass").toCharArray(password, memory.getString("pass").length() + 1);
      SerialBT.print("> Reconnect to ");
      WiFi.disconnect();
      WiFi_reconnect();
      SerialBT.println(WiFi.SSID());
    }
    //Вывод счетчиков перезагрузок и реконнектов
    else if (string.equalsIgnoreCase("Count\r")) {
      SerialBT.print("> ");
      SerialBT.println("Count: Reset " + String(memory.getInt("countReset")) + " | " + "Wifi " + String(memory.getInt("countWifi")));
    }
    //Сброс счетчиков перезагрузок и реконнектов
    else if (string.equalsIgnoreCase("CountR\r")) {
      memory.putInt("countReset", 0);
      memory.putInt("countWifi", 0);
      SerialBT.print("> ");
      SerialBT.println("Count: Reset " + String(memory.getInt("countReset")) + " | " + "Wifi " + String(memory.getInt("countWifi")));
    }
    //Вывод текущего времени
    else if (string.equalsIgnoreCase("Time\r")) {
      SerialBT.print("> ");
      GetTime();
      SerialBT.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    }
    //Маскировка пароля, если его ввод не требуется
    else if (string.equalsIgnoreCase("cbhLtpjr\r")) {
      SerialBT.println(">");
    }
    //Перезагрузка модуля
    else if (string.equalsIgnoreCase("Reset\r")) {
      SerialBT.println("> Reset");
      delay (250);
      ESP.restart();
    }
    //Help
    else if (string.equalsIgnoreCase("?\r") || string.equalsIgnoreCase("Help\r")) {
      SerialBT.println (">\n| Login␣<SSID>␣<Password> |\n| Mem | WiFi | Scan | Reconnect |\n| Count | CountR | Time | Reset |\n| ? | Help |\n>");
    }
    //Все остальное
    else {
      SerialBT.println ("> ? " + string + " ?");
    }
  }

  //Обработка пароля при первом после перезагрузке входе
  else {
    if (!string.compareTo("cbhLtpjr\r")) {
      key_flag = true;
      SerialBT.println ("> ");
    } else {
      SerialBT.println ("Пароль > ");
    }
  }
}



//Получение времени по NTP
void GetTime()
{
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  getLocalTime(&timeinfo);
}
