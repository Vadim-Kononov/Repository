// Arduino Nano ATmega328 на COM6
//>>>>--------------------------------SIM800L & Watchdog----------------------------------------------------------------------
#include <SoftwareSerial.h>
#include <iarduino_RTC.h>

#define Pin_Lider_Ch1     2                                         // Лидер канал 1
#define Pin_Lider_Ch2     3                                         // Лидер канал 2
#define Pin_SIM800L_Rxd   4                                         // Линия приема данных на SIM800L
#define Pin_SIM800L_Txd   5                                         // Линия передачи данных на SIM800L

#define Pin_SoundRelay    7                                         // Реле сирены
#define Pin_No_Power      8                                         // Исчезновение 220В
#define Pin_HC12_RX       9
#define Pin_HC12_TX       10                                        
#define Pin_Watchdog_In   11
#define Pin_Watchdog_Out  12

#define latency           1000

SoftwareSerial SIM800L(Pin_SIM800L_Txd,Pin_SIM800L_Rxd);            // Инициализация последовательного порта для SIM800L
SoftwareSerial HC12(Pin_HC12_TX,Pin_HC12_RX);
iarduino_RTC time(RTC_DS3231);                                      // Инициализация часов

String  My_number_1 = "+79198897600", My_number_2 = "+79198897200";  // Номера телефонов
String  MsgHeader = "", MsgBody = "", MsgPhone = My_number_1, Respond= "";    // Строковые переменные для обработки SMS   
boolean SMS_Ready_flag = false, Reset_flag = false, Pin_Watchdog_Out_flag = false, Pin_Watchdog_In_flag;                 // Флаги событий
int     Ch1_saved, Ch2_saved, No_Power_saved, Hour_saved_Test, Hour_saved_Watchdog;
long    Pin_Watchdog_In_counter;
boolean Light_flag = false, Siren_flag = false, Test_flag = true, Watchdog_flag = false ;
byte	  Command[5] = {0, 1, 0, 0, 0};                             // Создаём массив для передачи данных, записываем шаблон команды



void setup()
//>>>>--------------------------------void setup()----------------------------------------------------------------------------------
{
  Serial.begin(9600);                                     // Открываем COM порт для вывода сообщений на монитор
  SIM800L.begin(9600);                            				// Открываем последовательный порт для SIM800L
  HC12.begin(9600);
  HC12.setTimeout(2000);
  time.begin();
  time.period(1); 
  pinMode(Pin_Lider_Ch1, INPUT_PULLUP); 
  pinMode(Pin_Lider_Ch2, INPUT_PULLUP); 
  pinMode(Pin_No_Power, INPUT_PULLUP); 
  pinMode(Pin_SoundRelay, OUTPUT);
  digitalWrite(Pin_SoundRelay, HIGH); 
  pinMode(Pin_Watchdog_In, INPUT); 
  pinMode(Pin_Watchdog_Out, OUTPUT);                          // Линия для Pin_Watchdog_Out как цифровой выход
     
  time.gettime();
    
  sendATCommand("AT");                                          // Отправили AT для настройки скорости обмена данными
  sendATCommand("ATE0V1+CMGF=1;&W");              				      // Отключаем эхо, включаем текстовый формат ответов, включаем Text mode SMS и сразу сохраняем значение (AT&W)!
  sendATCommand("AT+CMGDA=\"DEL ALL\"");         	  			      // Удалили все сообщения, чтобы не забивали память модуля
  //sendSMS(My_number_1, "<-- Device Restart -->");               // Отправили SMS о перезагрузке устройства

  
  
  Hour_saved_Test = time.Hours; 
  Hour_saved_Watchdog= time.Hours;
  
  Ch1_saved=digitalRead(Pin_Lider_Ch1);
  Ch2_saved=digitalRead(Pin_Lider_Ch2);
  No_Power_saved=digitalRead(Pin_No_Power); 
  
  Pin_Watchdog_In_flag=digitalRead(Pin_Watchdog_In);
  Pin_Watchdog_In_counter = latency;
  
  Serial.println ("void setup() завершен.");
  
 

}
//<<<<--------------------------------void setup()----------------------------------------------------------------------------------





void loop()
//>>>>--------------------------------void loop()-----------------------------------------------------------------------------------
{
time.gettime();
//>>>>--------------------------------Watchdog----------------------------------------------------------------------
 if (!Reset_flag) {Pin_Watchdog_Out_flag = !Pin_Watchdog_Out_flag; digitalWrite(Pin_Watchdog_Out, Pin_Watchdog_Out_flag);}
  
 if (Pin_Watchdog_In_flag==digitalRead(Pin_Watchdog_In)) {--Pin_Watchdog_In_counter; if (Pin_Watchdog_In_counter<=0) {Watchdog_flag = true;}
 else                                                    {Pin_Watchdog_In_counter = latency; Pin_Watchdog_In_flag=digitalRead(Pin_Watchdog_In); Watchdog_flag = false;}
//<<<<--------------------------------Watchdog----------------------------------------------------------------------

//>>>>--------------------------------SIM800L-----------------------------------------------------------------------
 if (SIM800L.available())                                                 // Если модем, что-то отправил...
  {
    Respond = SIM800L.readString();                                       // Получаем ответ от модема для анализа
    Serial.print ("void loop() Неожидаемый ответ --->");
	  Serial.println (Respond);
    Respond.trim();                                                       // Убираем лишние пробелы в начале и конце
    if (Respond.startsWith("+CMTI:"))                                     // Пришло сообщение об отправке SMS
		{
		int index = Respond.lastIndexOf(",");                                 // Находим последнюю запятую, перед индексом
		String result = Respond.substring(index + 1, Respond.length());       // Получаем индекс
		result.trim();                                                        // Убираем пробельные символы в начале/конце
		Respond=sendATCommand("AT+CMGR="+result);                             // Получить содержимое SMS
		Respond = Respond.substring(Respond.indexOf("+CMGR: "));              // Распарсить SMS на элементы   
		MsgHeader = Respond.substring(0, Respond.indexOf("\r"));
		MsgBody = Respond.substring(MsgHeader.length() + 2);
		MsgBody = MsgBody.substring(0, MsgBody.lastIndexOf("OK"));
		MsgBody.trim();
		MsgBody = MsgBody.substring(0, 69);
		int firstIndex = MsgHeader.indexOf("\",\"") + 3;
		int secondIndex = MsgHeader.indexOf("\",\"", firstIndex);
		MsgPhone = MsgHeader.substring(firstIndex, secondIndex);
		SMS_Ready_flag = true;
		sendATCommand("AT+CMGDA=\"DEL ALL\"");                                // Удалить все сообщения, чтобы не забивали память модуля
    }
  } 
   
// Логика обработки SMS-команд.
  if (SMS_Ready_flag == true)
  {
	SMS_Ready_flag = false;
	Serial.println ("Обработка SMS:");
	Serial.print ("MsgPhone: "); Serial.print (MsgPhone); Serial.print ("   MsgBody: "); Serial.println (MsgBody);
	  if (MsgPhone.compareTo(My_number_1)==0 || MsgPhone.compareTo(My_number_2)==0)
	  { 
		if      (MsgBody.compareTo("Alarm")==0)     {Siren_flag = true; digitalWrite(Pin_SoundRelay, LOW); Light_flag = true; Command[0] = 10; HC12.write(Command, 5);}
		else if (MsgBody.compareTo("Light")==0)     {Siren_flag = false; digitalWrite(Pin_SoundRelay, HIGH); Light_flag = !Light_flag; if (Light_flag) {Command[0] = 20; HC12.write(Command, 5);} if (!Light_flag) {Command[0] = 30; HC12.write(Command, 5);} }
		else if (MsgBody.compareTo("Stop")==0)      {Siren_flag = false; digitalWrite(Pin_SoundRelay, HIGH); Light_flag = false; Command[0] = 30; HC12.write(Command, 5);}
		else if (MsgBody.compareTo("Test")==0)      {Test_flag  = !Test_flag; sendSMS(MsgPhone, "<-- Test = " + String (Test_flag) + " -->"); Command[0] = 40; HC12.write(Command, 5);}
		else if (MsgBody.compareTo("Reset")==0)     {sendSMS(MsgPhone, "<-- Reset accepted -->"); Reset_flag = true; Command[0] = 60; HC12.write(Command, 5);}
  	else    {sendSMS(MsgPhone, "-> Test\r\n-> Stop\r\n-> Light\r\n-> Reset\r\n-> Alarm\r\n--------------\r\nTest = " + String (Test_flag)); Command[0] = 0; HC12.write(Command, 5);}
	  }
  	else {sendSMS(My_number_1, "Unknown phone: " + MsgPhone+ "\r\n\r\n" + MsgBody);}
  } 
//<<<<--------------------------------SIM800L-----------------------------------------------------------------------


//>>>>--------------------------------HC12-------------------------------------------------------------------------
  if(HC12.available())
  {
  int i = HC12.readBytes(Command,sizeof(Command));
     if (i==5)
     { 
        if (Command[1] = 2)
        {
          switch (Command[0])
          {
          case 10:
          sendSMS(MsgPhone, "<-- Alarm  accepted -->");
          break;

          case 20:
          sendSMS(MsgPhone, "--> Light On -->");
          break;

          case 30:
          sendSMS(MsgPhone, "--> Light Off -->");
          break;
          }
        }
     }
     if (i!=5) sendSMS(MsgPhone, "--> Error HC12 -->");
  Command[0] = 0;
  Command[1] = 1;
  }
//<<<<--------------------------------HC12-------------------------------------------------------------------------


//>>>>--------------------------------Lider-------------------------------------------------------------------------
  
  if (Ch1_saved!=digitalRead(Pin_Lider_Ch1))
  {
  Ch1_saved=digitalRead(Pin_Lider_Ch1); 
  Siren_flag = true; digitalWrite(Pin_SoundRelay, LOW);Light_flag = true; Command[0] = 10; HC12.write(Command, 5);
  }
  
  if (Ch2_saved!=digitalRead(Pin_Lider_Ch2))
  {
  Ch2_saved=digitalRead(Pin_Lider_Ch2);
  Siren_flag = false; digitalWrite(Pin_SoundRelay, HIGH); Light_flag = !Light_flag; if (Light_flag) {Command[0] = 20; HC12.write(Command, 5);} if (!Light_flag) {Command[0] = 30; HC12.write(Command, 5);}
  }

  if (No_Power_saved!=digitalRead(Pin_No_Power))
  {
  No_Power_saved=digitalRead(Pin_No_Power);
  if (No_Power_saved==1) sendSMS(MsgPhone, "<-- No Power -->");
  if (No_Power_saved==0) sendSMS(MsgPhone, "<-- Power On -->");
  }
  
  if (Hour_saved_Test != time.Hours && time.Hours==20)
  {
  Hour_saved_Test = time.Hours; 
    if (Test_flag && !Siren_flag)
    {
    sendSMS(My_number_1, time.gettime("d.m.y H:i:s D"));
    digitalWrite(Pin_SoundRelay, LOW); delay(50); digitalWrite(Pin_SoundRelay, HIGH);
    }
  }
//<<<<--------------------------------Lider-------------------------------------------------------------------------


  if (Hour_saved_Watchdog != time.Hours && time.Hours==12)
  {
  Hour_saved_Watchdog = time.Hours; 
  if (Watchdog_flag) {sendSMS(MsgPhone, "<-- Watchdog is Crash -->");}
  }


}
//<<<<--------------------------------void loop()-----------------------------------------------------------------------------------









//>>>>--------------------------------SIM800L----------------------------------------------------------------------
String sendATCommand(String cmd)
{                                        
  String answer = "";                                                 		// Переменная для хранения результата
  long timeout = millis() + 10000;                                    		// Переменная для отслеживания таймаута (10 секунд)
  Serial.print ("sendATCommand Команда --->");
  Serial.println (cmd);
  SIM800L.println(cmd);
  while (!SIM800L.available() && millis() < timeout)  {};             		// Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
  if (SIM800L.available()) answer = SIM800L.readString();          		  	// Если есть, что считывать, считываем и запоминаем                                          
  Serial.print ("sendATCommand Ответ --->");
  Serial.println (answer);
  return answer;                                                      		// Возвращаем результат. Пусто, если проблема
}

void sendSMS(String phone, String message)
{
	Serial.println ("Отправка SMS:");
	//Serial.print ("message: ");
	//Serial.println (message);
	sendATCommand("AT+CMGS=\"" + phone + "\"");             				        // Переходим в режим ввода текстового сообщения
	sendATCommand(message + "\r\n" + (String)((char)26));  					        // После текста отправляем перенос строки и Ctrl+Z
	sendATCommand("AT+CMGDA=\"DEL ALL\"");
}
//<<<<--------------------------------SIM800L----------------------------------------------------------------------
