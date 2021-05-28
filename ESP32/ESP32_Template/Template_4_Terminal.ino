//Обработка принятых строк, параметры строка и строковое имя терминала от которого пришел запрос
void Terminal(String string, String terminal)
{
String string_0, string_1 = "", string_2 = "", string_3  = "" , string_4 = "", string_5 = "";

//Парсинг входой строки на 5 элементов по "/"
string_0 = string;
if (string.indexOf("/") >=0) {string_1 = string.substring(0, string.indexOf("/")); string.remove(0, string.indexOf("/") + 1);} else {string.trim(); string_1 = string; string.remove(0, string.length());}
if (string.indexOf("/") >=0) {string_2 = string.substring(0, string.indexOf("/")); string.remove(0, string.indexOf("/") + 1);} else {string.trim(); string_2 = string; string.remove(0, string.length());}
if (string.indexOf("/") >=0) {string_3 = string.substring(0, string.indexOf("/")); string.remove(0, string.indexOf("/") + 1);} else {string.trim(); string_3 = string; string.remove(0, string.length());}
if (string.indexOf("/") >=0) {string_4 = string.substring(0, string.indexOf("/")); string.remove(0, string.indexOf("/") + 1);} else {string.trim(); string_4 = string; string.remove(0, string.length());}
if (string.indexOf("/") >=0) {string_5 = string.substring(0, string.indexOf("/")); string.remove(0, string.indexOf("/") + 1);} else {string.trim(); string_5 = string; string.remove(0, string.length());}


//Если был введен пароль
if (key_flag)
{

//"Пароль". Отключение пароля, если он включен. Режим триггера
if (string_1.equalsIgnoreCase("cbhLtpjr")) {key_flag = false; Out ("< ", terminal); return;}

//"Login". Запись в память логина и пароля WiFi
else if (string_1.equalsIgnoreCase("Login")) {memory.putString("login", string_2); memory.putString("pass", string_3); Out("> " + memory.getString("login") + " " + memory.getString("pass"), terminal);}

//"Time". Вывод текущего времени, обращение к элементам структуры: timeinfo.tm_sec|timeinfo.tm_min|timeinfo.tm_hour и т.д., тип - int
else if (string_1.equalsIgnoreCase("Time")) {char time_str[50]; GetTime(); strftime(time_str, 50, "%A %d %b %Y %H:%M:%S", &timeinfo); Out("> " + String(time_str), terminal);}

//"Reset". Перезагрузка модуля
else if (string_1.equalsIgnoreCase("Reset")) {Out("> Reset", terminal); delay (250); ESP.restart();}

//"Count". Вывод счетчиков перезагрузок и реконнектов
else if (string_1.equalsIgnoreCase("Count")) Out("> Count: Reset " + String(memory.getInt("countReset")) + " | " + "Wifi " + String(memory.getInt("countWifi")), terminal);

//"CountR". Сброс счетчиков перезагрузок и реконнектов
else if (string_1.equalsIgnoreCase("CountR")) {memory.putInt("countReset", 0); memory.putInt("countWifi", 0); Out("> Count: Reset " + String(memory.getInt("countReset")) + " | " + "Wifi " + String(memory.getInt("countWifi")), terminal);}

//"Save". Вывод логина и пароля WiFi из памяти
else if (string_1.equalsIgnoreCase("Save")) {Out("> Login: " + memory.getString("login") + " | Pass: " + memory.getString("pass"), terminal);}

//"Mem". Вывод количества свободной памяти
else if (string_1.equalsIgnoreCase("Mem")) {Out ("> Free memory: " + String(ESP.getFreeHeap()), terminal);}

//"Reconnect". Переподключение WiFi по логину и паролю хранящимися в памяти
else if (string_1.equalsIgnoreCase("Reconnect"))
{
memory.getString("login").toCharArray(ssid, memory.getString("login").length() + 1);
memory.getString("pass").toCharArray(password, memory.getString("pass").length() + 1);
WiFi.disconnect(); WiFi_reconnect(); Out("> Reconnect to " + WiFi.SSID(), terminal);
}

//"WiFi". Вывод текущих параметров WiFi
else if (string_1.equalsIgnoreCase("WiFi")) {Out("> " + String(WiFi.SSID()) + " " + WiFi.localIP().toString()+ " " + String(WiFi.RSSI()), terminal);}

//"Scan". Сканирование WiFi
else if (string_1.equalsIgnoreCase("Scan")) {
Out("> Scan: ", terminal);
WiFiScan ();
}

//"Help".
else if (string_1.equalsIgnoreCase("?") || string_1.equalsIgnoreCase("Help"))
{Out (">\n| Login/<SSID>/<Password> |\n| Save | Mem |\n| WiFi | Scan | Reconnect |\n| Count | CountR | Time | Reset |\n| ? | Help |\n>", terminal);}

//Все остальное
else
{
Out ("1>" + string_1 + "<", terminal);
Out ("2>" + string_2 + "<", terminal);
Out ("3>" + string_3 + "<", terminal);
Out ("4>" + string_4 + "<", terminal);
Out ("5>" + string_5 + "<", terminal);
Out ("> ? " + string_0 + " ?", terminal);
}
}


//Обработка пароля при первом после перезагрузке входе
if (!key_flag) {if (!string_1.compareTo("cbhLtpjr")) {key_flag = true; Out ("> ", terminal);} else Out ("Пароль > ", terminal);}
}



// Вывод в терминалы Serial, WiFiSerial, BTSerial
void Out(String string, String terminal)
{
if      (terminal.equalsIgnoreCase("WiFi"))   {if (TelnetClient && TelnetClient.connected()) TelnetClient.println (string);}
else if (terminal.equalsIgnoreCase("BT"))     {if (SerialBT.hasClient()) SerialBT.println (string);}
}
