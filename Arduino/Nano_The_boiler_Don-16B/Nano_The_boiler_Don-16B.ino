#include <avr/wdt.h>
#include <OneWire.h>                                    // Библиотека протокола 1-Wire
#include <iarduino_RTC.h>
#include <DallasTemperature.h>                          // Библиотека для работы с датчиками DS*
#include <FlexiTimer2.h>
#include <max6675.h>

#define Pin_DS18B20 10  
#define Pin_CH A0
#define Pin_CO A2 
#define Pin_POWER 7
#define Pin_VALVE 4
#define Pin_SENSOR_CURRENT A4
#define Pin_RED 23
#define Pin_GREEN 25
#define Pin_BLUE 27

int thermoDO = 35;
int thermoCS = 33;
int thermoCLK = 31;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

String respond = "";                                    // Переменная для хранения ответа модуля
String my_number = "+79198897600";
String set_of_parameters = ""; 
String power_on;
int second_saved, minute_saved, hours_saved, day_saved, second_counter, minute_counter, hours_counter, day_counter, period_sending_SMS = 10, counter_valve = 0;
boolean flag_SMS_stream, flag_SMS_alarm, flag_alarm_OFF, flag_valve;
long millis_saved =0, interva_valvel = 25;

unsigned int temperature_inlet, temperature_outlet, temperature_fire, CH_level, CO_level;
byte array [10], room; 



iarduino_RTC time(RTC_DS3231);
OneWire oneWire(Pin_DS18B20);                           //Шина данных протокола OneWire на 8 пине
DallasTemperature DS18B20(&oneWire);                    // На базе ссылки OneWire создаем экземпляр объекта, работающего с датчиками DS*

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);                                  // SIM800
  Serial2.begin(9600);                                  // 433 Мгц
 
 
  sendATCommand("AT", true);                            // Отправили AT для настройки скорости обмена данными
  sendATCommand("ATE0V1+CMGF=1;&W", true);              // Отключаем эхо, включаем текстовый формат ответов, включаем Text mode SMS и сразу сохраняем значение (AT&W)!
  sendATCommand("AT+CMGDA=\"DEL ALL\"", true);          // Удалить все сообщения, чтобы не забивали память модуля
  sendSMS(my_number, "Restart!\r\nThe boiler Don-16B");
  
  time.begin();
  time.gettime();
  second_saved = time.seconds; minute_saved=time.minutes; hours_saved = time.Hours, day_saved = time.day;
  second_counter = 0; minute_counter = 0; hours_counter=0; day_counter =0;
  flag_SMS_stream = false; flag_SMS_alarm = true; flag_alarm_OFF = false; flag_valve = false;

  DS18B20.begin();                                      // Запускаем поиск всех датчиков DS18B20
  pinMode(Pin_POWER, INPUT_PULLUP);                         
  pinMode(Pin_VALVE, OUTPUT);
  digitalWrite(Pin_VALVE, HIGH); 
  
  pinMode(Pin_RED, OUTPUT);
  pinMode(Pin_GREEN, OUTPUT);
  pinMode(Pin_BLUE, OUTPUT);
  blank();
  
  
  
  FlexiTimer2::set(500, 1.0/1000, timerInterupt);       // вызываем эту функцию каждые 500 миллисекунд
  FlexiTimer2::start();
  wdt_enable(WDTO_1S);

}



void loop()
{
  
  
    
  if (Serial1.available())                // Если модем, что-то отправил...
  {
    respond = waitRespond();              // Получаем ответ от модема для анализа
    respond.trim();                       // Убираем лишние пробелы в начале и конце
    
    if (respond.startsWith("+CMTI:"))     // Пришло сообщение об отправке SMS
      {
      void blue();
      int index = respond.lastIndexOf(",");                           // Находим последнюю запятую, перед индексом
      String result = respond.substring(index + 1, respond.length()); // Получаем индекс
      result.trim();                                                  // Убираем пробельные символы в начале/конце
      respond=sendATCommand("AT+CMGR="+result, true);                 // Получить содержимое SMS
      analysisSMS(respond);                                           // Распарсить SMS на элементы   
      sendATCommand("AT+CMGDA=\"DEL ALL\"", true);                    // Удалить все сообщения, чтобы не забивали память модуля
      }
  }

  time.gettime();
  if (second_saved != time.seconds ) {second_counter++; second_saved = time.seconds; }
  if (minute_saved != time.minutes ) {minute_counter++; minute_saved = time.minutes; }
  if (hours_saved != time.Hours ) {hours_counter++; hours_saved = time.Hours; }
  if (day_saved != time.day ) {day_counter++; day_saved = time.day; }
    
  if (second_counter >= 1)        // Каждую секунду
  {
  DS18B20.requestTemperatures();
  second_counter = 0;
  green();
  if (digitalRead(Pin_POWER)==0) {power_on = "ON";} else {power_on = "OFF";}  
  CH_level =  analogRead(Pin_CH); CO_level =  analogRead(Pin_CO);
  temperature_fire = thermocouple.readCelsius()-1 ;
  temperature_inlet = DS18B20.getTempCByIndex(0); temperature_outlet = DS18B20.getTempCByIndex(1);
    
    array[0] = highByte(temperature_inlet);
    array[1] = lowByte(temperature_inlet);
    array[2] = highByte(temperature_outlet);
    array[3] = lowByte(temperature_outlet);
    array[4] = highByte(temperature_fire);
    array[5] = lowByte(temperature_fire);
    array[6] = highByte(CH_level);
    array[7] = lowByte(CH_level);
    array[8] = highByte(CO_level);
    array[9] = lowByte(CO_level);
  
      for (int i=0; i <= 9; i++)
      {
      Serial2.write(array[i]);
      }
  
  
  
    Serial.print(temperature_inlet);
    Serial.print(" -> "); 
    Serial.print(temperature_outlet);
    Serial.print("   F: ");
    Serial.print(temperature_fire);
    Serial.print("   CH:"); 
    Serial.print(CH_level);
    Serial.print("   CO:"); 
    Serial.print(CO_level);
    Serial.print("   Room:"); 
    Serial.println(room);
 
  }

  if (minute_counter >= period_sending_SMS)
  {
  minute_counter = 0;
  sendATCommand("AT+CMGDA=\"DEL ALL\"", true);
  if (flag_SMS_stream) sendSMS(my_number, set_of_parameters);
  }
  
  if (hours_counter >= 3)
  {
  hours_counter = 0;
  flag_SMS_alarm = true;
  }
  
  
  if (day_counter >=150)
  {
    if (time.Hours == 11) {day_counter = 0; sendATCommand("ATD" + my_number + ";", true);}
  }



  
    if(Serial2.available())
    {
    room = Serial2.read();
    }
  
  
     
  set_of_parameters = "Low: " + String(temperature_inlet) + "  Hige: " + String(temperature_outlet) + "  Fire: " + String(temperature_fire);
  set_of_parameters = set_of_parameters + "\r\n\r\nCH: " + String(CH_level) + "  CO: " + String(CO_level) + "\r\n\r\nPower: " + power_on + "\r\n\r\nRoom: " + String(room);
  set_of_parameters = time.gettime("d.m.y H:i:s D")  + String("\r\n\r\n") + set_of_parameters;
    
  if (power_on.compareTo("OFF")==0 || temperature_inlet >= 1000 || temperature_outlet >= 1000 || temperature_outlet - temperature_inlet >= 1000 || CH_level > 1000 || CO_level > 1000 )
  {
      set_of_parameters = "<-ALARM->\r\n\r\n" + set_of_parameters;
      if (flag_SMS_alarm) {flag_SMS_stream = true; minute_counter = period_sending_SMS; flag_SMS_alarm = false; hours_counter = 0;}
  }
 
  // Управление клапаном ->
  if(flag_valve)
  {
    digitalWrite(Pin_VALVE, LOW);
    if (millis() - millis_saved > interva_valvel)
    {
      millis_saved = millis();   
      if (abs(0.0264*(analogRead(Pin_SENSOR_CURRENT)-512))>0.5) {counter_valve++;} 
      else {counter_valve=0;} 
      if (counter_valve>=50)
      {
        flag_valve = false; digitalWrite(Pin_VALVE, HIGH);
        flag_SMS_stream = true;
        minute_counter = period_sending_SMS;
        sendSMS(my_number, "Gas closed!");
      }
    }
  }
  // <- Управление клапаном




}




//
String sendATCommand(String cmd, bool waiting)
{
  String answer = "";                                                 // Переменная для хранения результата
  Serial1.println(cmd);                                               // Отправляем команду модулю
  if (waiting) answer = waitRespond();                                // Если необходимо дождаться ответа ждем, когда будет передан ответ     
  return answer;                                                      // Возвращаем результат. Пусто, если проблема
}
//
String waitRespond()                                                  // Функция ожидания ответа и возврата полученного результата
{                         
  String answer = "";                                                 // Переменная для хранения результата
  long timeout = millis() + 10000;                                    // Переменная для отслеживания таймаута (10 секунд)
  while (!Serial1.available() && millis() < timeout)  {};             // Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
  if (Serial1.available()) { answer = Serial1.readString();}          // Если есть, что считывать, считываем и запоминаем               
  else {answer = "Timeout";}                                          // Если пришел таймаут, то оповещаем об этом и...
  return answer;                                                      // ... возвращаем результат. Пусто, если проблема
}
//
void analysisSMS(String msg)
{
  String msgheader = "";
  String msgbody = "";
  String msgphone = "";

  msg = msg.substring(msg.indexOf("+CMGR: "));
  msgheader = msg.substring(0, msg.indexOf("\r"));

  msgbody = msg.substring(msgheader.length() + 2);
  msgbody = msgbody.substring(0, msgbody.lastIndexOf("OK"));
  msgbody.trim();

  int firstIndex = msgheader.indexOf("\",\"") + 3;
  int secondIndex = msgheader.indexOf("\",\"", firstIndex);
  msgphone = msgheader.substring(firstIndex, secondIndex);

  //Serial.println (msgphone);
  //Serial.println (msgbody);
  
  // Логика обработки SMS-команд.
  if (msgphone.compareTo(my_number)==0)
  {
  if      (msgbody.compareTo("0")==0)         {flag_SMS_stream = false; flag_alarm_OFF = false; sendSMS(my_number, "Sending SMS is stopped!");}
  else if (msgbody.compareTo("1")==0)         {flag_SMS_stream = true; flag_alarm_OFF = false; minute_counter = period_sending_SMS;}
  else if (msgbody.compareTo("2")==0)         {flag_alarm_OFF = false; sendSMS(my_number, set_of_parameters);}
  else if (msgbody.compareTo("Close")==0)     {flag_valve = true; flag_alarm_OFF = false; sendSMS(my_number, "Signal is accepted, waiting...");}
  else if (msgbody.compareTo("Alarm OFF")==0) {flag_SMS_stream = false; flag_alarm_OFF = true; sendSMS(my_number, "Attention. Alarm OFF!");}                             
  else if (msgbody.compareTo("Reset")==0)     {FlexiTimer2::stop();}
  else    sendSMS(my_number, "0 ---> Stop stream SMS\r\n1 ---> Start stream SMS\r\n2 ---> Single SMS\r\nAlarm OFF ---> Disable alarm\r\nClose ---> Close the gas\r\nReset ---> Reboot the system");
  }
  else sendSMS(my_number, "Phone: " + msgphone + "\r\n-------------------------------------\r\n" + msgbody);
 }
//
void sendSMS(String phone, String message)
{
  white();
  //sendATCommand("AT+CMGS=\"" + phone + "\"", true);             // Переходим в режим ввода текстового сообщения
  //sendATCommand(message + "\r\n" + (String)((char)26), true);   // После текста отправляем перенос строки и Ctrl+Z
}
//
void  timerInterupt() 
{
wdt_reset();                                                    // сброс сторожевого таймера 
}
//
void red()
{
digitalWrite(Pin_RED, HIGH);
digitalWrite(Pin_GREEN, LOW);
digitalWrite(Pin_BLUE, LOW);
}
//
void green()
{
digitalWrite(Pin_RED, LOW);
digitalWrite(Pin_GREEN, HIGH);
digitalWrite(Pin_BLUE, LOW);
}
//
void blue()
{
digitalWrite(Pin_RED, LOW);
digitalWrite(Pin_GREEN, LOW);
digitalWrite(Pin_BLUE, HIGH);
}
//
void white()
{
digitalWrite(Pin_RED, HIGH);
digitalWrite(Pin_GREEN, HIGH);
digitalWrite(Pin_BLUE, HIGH);
}
//
void blank()
{
digitalWrite(Pin_RED, LOW);
digitalWrite(Pin_GREEN, LOW);
digitalWrite(Pin_BLUE, LOW);
}
