void loop()
{
//BTSerial получение строки, если она есть. Вызов Terminal для обработки с параметром "BT"
if (SerialBT.available()) Terminal(SerialBT.readStringUntil('\n'));

//Код может быть получен в одном экземляре после его изменения
if (receiver.available())
{
if (code != receiver.getReceivedValue()){code = receiver.getReceivedValue(); code_flag = true; count_ms_Code = millis();}
receiver.resetAvailable();
}

//Переменная с кодом обнуляется каждые две секунды, в течении этих двух секунд повторный код не обрабатывается
if (millis() - count_ms_Code > time_ms_Code) {code = 0;}

//Запись в EEPROM принятого кода, если в Terminal установлен dev_flag и пришел новый код
if (code_flag && dev_flag)
{

//Запрет дальнейшей записи кода, до следующей установки dev_flag
dev_flag = false;
//Флаг принятия кода
code_flag = false; 

//Запись элементов структуры
detect_buf[number].number = number; detect_buf[number].code = code; title.toCharArray(detect_buf[number].title, 50);
//Запись в EEPROM
memory.putBytes("mem", (detector*)detect_buf, sizeof(detect_buf));

//Вывод в терминалы таблицы кодов
print_sensor();
}


//Обработка принятого кода, если это не запись (!dev_flag)
if (code_flag && !dev_flag)
{
//Флаг принятия кода
code_flag = false;

String str ="";
//Определение наименования сработавшего датчика
for (int i=0; i<16; i++) {if (detect_buf[i].number == i && detect_buf[i].code == code) {str = detect_buf[i].title;}};

//Управление реле и светодиодами
if (str.equals("On")) {digitalWrite(RELAY_PIN, HIGH); relay_flag = true; digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, HIGH); digitalWrite(LED_BLUE, LOW);}
if (str.equals("Off")) {digitalWrite(RELAY_PIN, LOW); relay_flag = false; digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, HIGH); digitalWrite(LED_BLUE, LOW);}

//Вывод принятого кода в BT Terminal
SerialBT.println (">>> " + String(code, HEX) + "  " + str);
count_ms_Led = millis();
}

//Включение нужного светодиода после таймаута
if (millis() - count_ms_Led > time_ms_Led)
{
if (relay_flag) {digitalWrite(LED_RED, HIGH); digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, LOW);} 
else            {digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, HIGH);} 
}



//Сброс сторожевого таймера
timerWrite(timer, 0);
}
