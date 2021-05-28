#define RXD2 25
#define TXD2 27

String respond = "";                                                  // Переменная для хранения ответа модуля
String my_number = "+79198897600";

void setup() {

Serial.begin(115200);
Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

sendATCommand("AT", true);                                            // Отправили AT для настройки скорости обмена данными
sendATCommand("ATE0V1+CMGF=1;&W", true);                              // Отключаем эхо, включаем текстовый формат ответов, включаем Text mode SMS и сразу сохраняем значение (AT&W)!
sendATCommand("AT+CMGDA=\"DEL ALL\"", true);                          // Удалить все сообщения, чтобы не забивали память модуля
//sendSMS(my_number, "Signal System");


}

void loop() {

if (Serial2.available())                                              // Если модем, что-то отправил...
  {
  respond = waitRespond();                                            // Получаем ответ от модема для анализа
  
  respond.trim();                                                     // Убираем лишние пробелы в начале и конце    
    if (respond.startsWith("+CMTI:"))                                 // Пришло сообщение об отправке SMS
      {     
      int index = respond.lastIndexOf(",");                           // Находим последнюю запятую, перед индексом
      String result = respond.substring(index + 1, respond.length()); // Получаем индекс
      result.trim();                                                  // Убираем пробельные символы в начале/конце
      respond=sendATCommand("AT+CMGR="+result, true);                 // Получить содержимое SMS
      sendATCommand("AT+CMGDA=\"DEL ALL\"", true);                    // Удалить все сообщения, чтобы не забивали память модуля
      analysisSMS(respond);                                           // Распарсить SMS на элементы   
      }
  }
}


//---------------------------------------------
String sendATCommand(String cmd, bool waiting)
{
  String answer = "";                                                 // Переменная для хранения результата
  Serial.println(cmd);    
  Serial2.println(cmd);                                               // Отправляем команду модулю
  if (waiting) answer = waitRespond();                                // Если необходимо дождаться ответа ждем, когда будет передан ответ     
  return answer;                                                      // Возвращаем результат. Пусто, если проблема
}
//---------------------------------------------
String waitRespond()                                                  // Функция ожидания ответа и возврата полученного результата
{                         
  String answer = "";                                                 // Переменная для хранения результата
  long timeout = millis() + 5000;                                    // Переменная для отслеживания таймаута (2 секунд)
  while (!Serial2.available() && millis() < timeout)  {};             // Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
  if (Serial2.available()) { answer = Serial2.readString();}          // Если есть, что считывать, считываем и запоминаем               
  else {answer = "Timeout";}                                          // Если пришел таймаут, то оповещаем об этом и...
  Serial.println(answer); 
  return answer;                                                      // ... возвращаем результат. Пусто, если проблема
}
//---------------------------------------------
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
  sendSMS(my_number, "Answer: " + msgbody);
  }  
 }
//---------------------------------------------
void sendSMS(String phone, String message)
{
 
sendATCommand("AT+CMGS=\"" + phone + "\"", true);             // Переходим в режим ввода текстового сообщения
sendATCommand(message + "\r\n" + (String)((char)26), true);   // После текста отправляем перенос строки и Ctrl+Z
}
//---------------------------------------------
