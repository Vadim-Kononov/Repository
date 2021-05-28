//Обработка принятых строк, параметры строка и строковое имя терминала от которого пришел запрос
void Terminal(String string)
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
if (string_1.equalsIgnoreCase("cbhLtpjr")) {key_flag = false; SerialBT.println ("< "); return;}

//"Reset". Перезагрузка модуля
else if (string_1.equalsIgnoreCase("Reset")) {SerialBT.println ("> Сброс"); delay (250); ESP.restart();}

//"Mem". Вывод количества свободной памяти
else if (string_1.equalsIgnoreCase("Mem")) {SerialBT.println ("> Свободная память: " + String(ESP.getFreeHeap()) + "\n>");}

//"Write". Запись в память данных датчика
else if (string_1.equalsIgnoreCase("Write"))
{
number = string_2.toInt(); title = string_3;
if ((number>=0 && number<=15) && (title.length() >=1 && title.length() <=48)) {dev_flag = true; SerialBT.println ("--- Ожидание кода ---");}
}

//"Read". Чтение записанных датчиков
else if (string_1.equalsIgnoreCase("Read")) print_sensor();

//"Clear". Стирание записанного датчика
else if (string_1.equalsIgnoreCase("Clear"))
{
number = string_2.toInt();
if (number>=0 && number<=15)
{
detect_buf[number].number = 0; detect_buf[number].code = 0; title = "Empty"; title.toCharArray(detect_buf[number].title, 50);
memory.putBytes("mem", (detector*)detect_buf, sizeof(detect_buf));
print_sensor();
}
}

//"Help".
else if (string_1.equalsIgnoreCase("?") || string_1.equalsIgnoreCase("Help"))
SerialBT.println ("| Read |\n| Write/<Номер>/<Имя> |\n| Clear/<Номер> |\n| Mem | Reset | ? | Help |\n>");

//Все остальное
else
{
SerialBT.println ("1>" + string_1 + "<");
SerialBT.println ("2>" + string_2 + "<");
SerialBT.println ("3>" + string_3 + "<");
SerialBT.println ("4>" + string_4 + "<");
SerialBT.println ("5>" + string_5 + "<");
SerialBT.println ("?> " + string_0 + "<?");
SerialBT.println ("> ");
}
}

//Обработка пароля при первом после перезагрузке входе
if (!key_flag) {if (!string_1.compareTo("cbhLtpjr")) {key_flag = true; SerialBT.println ("> ");} else SerialBT.println ("Пароль > ");}
}


//Вывод записанных в память датчиков
void print_sensor()
{
memory.getBytes("mem", (detector*)detect_buf, sizeof(detect_buf));
SerialBT.println ("Записано:\n> ");
for (int i=0; i<16; i++) {SerialBT.println (String(detect_buf[i].number) + "  " + String(detect_buf[i].code, HEX) + "  " +  String(detect_buf[i].title)); SerialBT.flush();}
SerialBT.println ("> ");
}
