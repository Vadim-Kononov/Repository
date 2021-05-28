bool checkConnection()
{
if(!telegramClient.connected())
{
Serial.println("Connecting to the server");
//telegramClient.stop();
telegramClient.setTimeout(5);
telegramClient.setHandshakeTimeout(5);
telegramClient.connect(TELEGRAM_HOST, TELEGRAM_PORT);    
}
return telegramClient.connected();
}



bool getUpdates()
{
str_response = "";
String command, parameter;
command.reserve(16); parameter.reserve(16);
command = "getUpdates";
parameter = "";
DynamicJsonDocument root(16);       
root["offset"] = -1;
serializeJson(root, parameter);

if(checkConnection())
{      
        //Serial.println("connected");
        String request((char *)0);
        request.reserve(SIZE_MIN);
        request = "POST https://" TELEGRAM_HOST "/bot";
        request += m_token;
        request += "/";
        request += command;
        request += " HTTP/1.1" "\nHost: api.telegrasav.org" "\nConnection: keep-alive" "\nContent-Type: application/json";
        request += "\nContent-Length: ";
        request += String(strlen(parameter.c_str()));
        request += "\n\n";
        request += parameter;
        telegramClient.print(request);
        //Serial.println(request);       
        
        while (telegramClient.connected())
        {
          String line = telegramClient.readStringUntil('\n');
          if (line == "\r") {/*Serial.println("Headers received");*/ break;}
        }
        
        while (telegramClient.available()){str_response += (char) telegramClient.read();}
        //Serial.println(str_response); Serial.println(); 
}
DynamicJsonDocument doc(SIZE_MAX);
deserializeJson(doc, str_response);
bool ok         = doc["ok"];
update_id       = doc["result"][0]["update_id"];
message_id      = doc["result"][0]["message"]["message_id"];
user_id         = doc["result"][0]["message"]["from"]["id"];
str_first_name  = doc["result"][0]["message"]["from"]["first_name"].as<String>();
chat_id         = doc["result"][0]["message"]["chat"]["id"];
str_chat_title  = doc["result"][0]["message"]["chat"]["title"].as<String>();
str_text        = doc["result"][0]["message"]["text"].as<String>();   
if (ok) return true; else return false;
}



void sendMessage(int64_t chat_id)
{

String command, parameter;
command.reserve(16); parameter.reserve(SIZE_MIN);
command = "sendMessage";
parameter = "";
DynamicJsonDocument root(SIZE_MIN);      
root["chat_id"] = chat_id;
root["text"] = str_answer;
serializeJson(root, parameter);

if(checkConnection())
{      
        //Serial.println("connected");
        String request((char *)0);
        request.reserve(SIZE_MAX);
        request = "POST https://" TELEGRAM_HOST "/bot";
        request += m_token;
        request += "/";
        request += command;
        request += " HTTP/1.1" "\nHost: api.telegrasav.org" "\nConnection: keep-alive" "\nContent-Type: application/json";
        request += "\nContent-Length: ";
        request += String(strlen(parameter.c_str()));
        request += "\n\n";
        request += parameter;
        telegramClient.print(request);
        //Serial.println(request); Serial.println(str_response); Serial.println();      
}
}



String FirstCharUp (String stroka)
{
char c[SIZE_MIN];
stroka.toCharArray(c, stroka.length()+1);
  if        (c[0] == 208 && c[1] >=176 && c[1] <=191) c[1] -= 32;
  else if   (c[0] == 209 && c[1] >=128 && c[1] <=143) {c[0] = 208; c[1] += 32;}
  else if   (c[0] == 209 && c[1] == 145 ) {c[0] = 208; c[1] = 129;}
return (String(c));
}



String LowCase (String stroka)
{
char c[SIZE_MIN];
stroka.toCharArray(c, stroka.length()+1);
for (int i = 0; i<= stroka.length(); i +=1)
  {
  if        (c[i] == 208 && c[i + 1] >=144 && c[i + 1] <=159) {c[i + 1] = c[i + 1] + 32;}
  else if   (c[i] == 208 && c[i + 1] >=160 && c[i + 1] <=175) {c[i] = 209; c[i + 1] = c[i + 1] - 32;}
  else if   (c[i] == 208 && c[i + 1] == 129 ) {c[i] = 209; c[i + 1] = 145;}
  }  
return (String(c));
}



void WatchDog()
{
Serial.println("Timer WatchDog went off"); ESP.restart();
}



String int64ToAscii(int64_t value)
{
  String  buffer; buffer.reserve(16); buffer = "";
  int64_t temp;
  uint8_t rest;
  char    ascii;
  if (value < 0) temp = -value;
  else           temp = value;
    
  while (temp != 0)
  {
    rest = temp % 10;
    temp = (temp - rest) / 10;
    ascii = 0x30 + rest;
    buffer = ascii + buffer;
  }
  if (value < 0) buffer = '-' + buffer;
  return buffer;
}



void Manag ()
{
/*Парсинг входой строки на 2 элемента по "/"*/
if (str_text.indexOf("/") >=0) {str_1 = str_text.substring(0, str_text.indexOf("/")); str_text.remove(0, str_text.indexOf("/") + 1);} else {str_text.trim(); str_1 = str_text; str_text.remove(0, str_text.length());}
if (str_text.indexOf("/") >=0) {str_2 = str_text.substring(0, str_text.indexOf("/")); str_text.remove(0, str_text.indexOf("/") + 1);} else {str_text.trim(); str_2 = str_text; str_text.remove(0, str_text.length());}
if (str_text.indexOf("/") >=0) {str_3 = str_text.substring(0, str_text.indexOf("/")); str_text.remove(0, str_text.indexOf("/") + 1);} else {str_text.trim(); str_3 = str_text; str_text.remove(0, str_text.length());}
if (str_text.indexOf("/") >=0) {str_4 = str_text.substring(0, str_text.indexOf("/")); str_text.remove(0, str_text.indexOf("/") + 1);} else {str_text.trim(); str_4 = str_text; str_text.remove(0, str_text.length());}
if (str_text.indexOf("/") >=0) {str_5 = str_text.substring(0, str_text.indexOf("/")); str_text.remove(0, str_text.indexOf("/") + 1);} else {str_text.trim(); str_5 = str_text; str_text.remove(0, str_text.length());}
if (str_text.indexOf("/") >=0) {str_6 = str_text.substring(0, str_text.indexOf("/")); str_text.remove(0, str_text.indexOf("/") + 1);} else {str_text.trim(); str_6 = str_text; str_text.remove(0, str_text.length());}
if (str_temp.indexOf("/") >=0) {str_7 = str_temp.substring(0, str_temp.indexOf("/")); str_temp.remove(0, str_temp.indexOf("/") + 1);} else {str_temp.trim(); str_7 = str_temp; str_temp.remove(0, str_temp.length());}
if (str_temp.indexOf("/") >=0) {str_8 = str_temp.substring(0, str_temp.indexOf("/")); str_temp.remove(0, str_temp.indexOf("/") + 1);} else {str_temp.trim(); str_8 = str_temp; str_temp.remove(0, str_temp.length());}


int part_1 = str_1.toInt();
int part_2 = str_2.toInt();
int part_3 = str_3.toInt();
str_temp = str_3;
if (str_4.length() != 0) str_temp += "/" + str_4;
if (str_5.length() != 0) str_temp += "/" + str_5;
if (str_6.length() != 0) str_temp += "/" + str_6;
if (str_7.length() != 0) str_temp += "/" + str_7;
if (str_8.length() != 0) str_temp += "/" + str_8;


if (!flag_chat && part_1 >=1 && part_1 <=FILES_SUM)
{
  if      (str_2.length() == 0) {Read(String ("/" + str_1 + ".txt").c_str());}
  else if ((str_2.equalsIgnoreCase("0") || str_2.equalsIgnoreCase("300")) && (part_3 >=1 && part_3 <=100) && (str_3.length()<=3)) Write(String ("/" + str_1 + ".txt").c_str(), part_2, part_3);
  else if (part_2 >= 0 && part_2 <=100) Write(String ("/" + str_1 + ".txt").c_str(), part_2, str_temp);
}
else if (!flag_chat && part_1 == 10) {memory.putBool("listen", !memory.getBool("listen")); if (memory.getBool("listen")) str_answer = "Слушаю"; else str_answer = "Не слушаю"; sendMessage(main_id);}
else if (!flag_chat && part_1 == 11) Dir();
else if (!flag_chat && str_1.equalsIgnoreCase("Obv")) ObverseFile(1, FILES_SUM);
else if (!flag_chat && str_1.equalsIgnoreCase("Rev")) ReverseFile(1, FILES_SUM);
else if (!flag_chat && part_1 == 0) ReadMe ();
}



void CleanFile(const char * path)
{
Serial.println("Writing file");
File file = SD.open(path, FILE_WRITE);
if(!file){Serial.println("Failed to open file for writing"); return;}
file.seek(0, SeekSet); file.write (0);
file.seek(300, SeekSet); file.write (1);
for (int i = 1; i<=100; i++) {file.seek(i, SeekSet); uint8_t k = random(1, 6); file.write (k);}
for (int i = SIZE_STR_SPEECH; i<=SIZE_STR_SPEECH*NUMBER_OF_ROWS+SIZE_STR_SPEECH; i++) file.write('\0') ; 
file.close();
}



void CopyFile(const char * path1, const char * path2)
{
File file1 = SD.open(path1);
File file2 = SD.open(path2, FILE_WRITE);
int i = 1;
while(file1.available())
{
str_temp = file1.readStringUntil('\n');
if (str_temp.length() > SIZE_STR_SPEECH) str_temp = "Очень длинная фраза...";
file2.seek(SIZE_STR_SPEECH*i, SeekSet);
file2.print(str_temp + "\n");
i +=1;
}
file2.seek(0, SeekSet); file2.write (i-1);
file2.seek(300, SeekSet); file2.write (i);
file1.close();  
file2.close();  
}



void Dir()
{
File root = SD.open("/");
File file = root.openNextFile();
str_answer = "";
while(file)
 {     
  if(file.isDirectory()) str_temp = ""; else {str_temp = (String) file.name() + "\n"; str_temp.replace("/", ""); str_temp.replace(".txt", "");}
  if (str_answer.length() + str_temp.length() < SIZE_MIN - 32) str_answer += str_temp;
  else {sendMessage(chat_id); str_answer = ""; str_answer += str_temp;}
  file = root.openNextFile(); 
 }
sendMessage(chat_id); 
}




void Read(const char * path)
{
File file = SD.open(path, "r");
file.seek(0, SeekSet);    note = file.read();
file.seek(300, SeekSet);  point = file.read();
str_answer = "Записей: " + String(note) + "\n" + "Курсор: " + String(point); sendMessage(chat_id); str_answer = "";
for (int k=SIZE_STR_SPEECH; k <= note*SIZE_STR_SPEECH; k+=SIZE_STR_SPEECH)
{
file.seek(k, SeekSet); str_temp = file.readStringUntil('\n'); str_temp.replace("/", "\n ~ ");
if (str_answer.length() + str_temp.length() < SIZE_MIN - 64) str_answer += String (k/SIZE_STR_SPEECH) + " # " + str_temp + "\n\n";
else {sendMessage(chat_id); str_answer = ""; str_answer += String (k/SIZE_STR_SPEECH) + " # " + str_temp + "\n\n";}
}
file.close();
sendMessage(chat_id);
}




void Write(const char * path, int code, String str)
{
str.reserve(SIZE_STR_SPEECH);
if (str.length() > SIZE_STR_SPEECH) str = "Очень длинная фраза...";
File file = SD.open(path, "r+");
file.seek(0, SeekSet);    note = file.read();
file.seek(300, SeekSet);  point = file.read();
str_answer = "Записей: " + String(note) + "\n" + "Курсор: " + String(point) + "\n";
if      (code == 0)
{
file.seek(SIZE_STR_SPEECH*point, SeekSet); file.print(str + "\n");
file.seek(0, SeekSet); if (note < 100) file.write (note+1);
file.seek(300, SeekSet); if (point < 100) file.write (point+1); else file.write (1);
file.close();
}
else if (code >= 1 && code <= note) {file.seek(SIZE_STR_SPEECH*code, SeekSet); file.print(str + "\n"); file.close();}
else    {file.close(); str_answer += "Неправильный номер записи, уродец"; sendMessage(chat_id); return;}
Read(path);
}



void Write(const char * path, uint16_t part_2, uint8_t part_3)
{
File file = SD.open(path, "r+");
if      (part_2 == 300) {file.seek(300, SeekSet); file.write (part_3);}
else if (part_2 == 0)   {file.seek(0, SeekSet); file.write (part_3);}
file.close();
Read(path);
}



void Randomstr_speech(const char * path)
{
File file = SD.open(path, "r");
file.seek(0, SeekSet);    note = file.read();
randomSeed(micros());
uint8_t k = random(1, note + 1); 
file.seek(k*SIZE_STR_SPEECH, SeekSet); 
str_speech = file.readStringUntil('\n');
file.close();
}



void Searchstr_speech(const char * path)
{
File file = SD.open(path, "r+");
file.seek(0, SeekSet); note = file.read ();
for (int k = 1; k<=note; k++)
{
file.seek(k*SIZE_STR_SPEECH, SeekSet); 
str_temp = file.readStringUntil('\n');
if (str_temp.indexOf("/") >=0) {str_3 = str_temp.substring(0, str_temp.indexOf("/")); str_temp.remove(0, str_temp.indexOf("/") + 1);} else {str_temp.trim(); str_3 = str_temp; str_temp.remove(0, str_temp.length());}
if (str_temp.indexOf("/") >=0) {str_4 = str_temp.substring(0, str_temp.indexOf("/")); str_temp.remove(0, str_temp.indexOf("/") + 1);} else {str_temp.trim(); str_4 = str_temp; str_temp.remove(0, str_temp.length());}
if (str_temp.indexOf("/") >=0) {str_5 = str_temp.substring(0, str_temp.indexOf("/")); str_temp.remove(0, str_temp.indexOf("/") + 1);} else {str_temp.trim(); str_5 = str_temp; str_temp.remove(0, str_temp.length());}
if (str_temp.indexOf("/") >=0) {str_6 = str_temp.substring(0, str_temp.indexOf("/")); str_temp.remove(0, str_temp.indexOf("/") + 1);} else {str_temp.trim(); str_6 = str_temp; str_temp.remove(0, str_temp.length());}
if (str_temp.indexOf("/") >=0) {str_7 = str_temp.substring(0, str_temp.indexOf("/")); str_temp.remove(0, str_temp.indexOf("/") + 1);} else {str_temp.trim(); str_7 = str_temp; str_temp.remove(0, str_temp.length());}
if (str_temp.indexOf("/") >=0) {str_8 = str_temp.substring(0, str_temp.indexOf("/")); str_temp.remove(0, str_temp.indexOf("/") + 1);} else {str_temp.trim(); str_8 = str_temp; str_temp.remove(0, str_temp.length());}
str_speech = "";
str_3 = LowCase (str_3);
if (str_text.indexOf(str_3.c_str())>=0)
  { 
  file.seek(k, SeekSet); marker = file.read ();
  if      (marker == 1) {str_speech = str_4; file.seek(k, SeekSet); file.write (2); break;}
  else if (marker == 2) {str_speech = str_5; file.seek(k, SeekSet); file.write (3); break;}
  else if (marker == 3) {str_speech = str_6; file.seek(k, SeekSet); file.write (4); break;}
  else if (marker == 4) {str_speech = str_7; file.seek(k, SeekSet); file.write (5); break;}
  else if (marker == 5) {str_speech = str_8; file.seek(k, SeekSet); file.write (1); break;}
  else    {marker = 1;   str_speech = str_4; file.seek(k, SeekSet); file.write (1); break;}
  }
}
file.close();
}



void ReadMe()
{
File file = SD.open("/0.txt");
str_answer = "";
while(file.available())
{
str_temp = file.readStringUntil('\n') + "\n";
if (str_answer.length() + str_temp.length() < SIZE_MIN - 64) str_answer += str_temp;
else {sendMessage(chat_id); str_answer = ""; str_answer += str_temp ;}
}
file.close();
sendMessage(chat_id);
}



void ObverseFile(int num_1, int num_2)
{
xTimerChangePeriod(timerWatchDog,  pdMS_TO_TICKS(30000), 0);
str_answer = "";
for (int num = num_1; num <= num_2; num++ )
{
  CleanFile(String("/" + (String) num + ".txt").c_str());
  File file1 = SD.open(String("/" + (String) num + "A.txt").c_str());
  File file2 = SD.open(String("/" + (String) num + ".txt").c_str(), "r+");
  uint8_t i = 1;
  while(file1.available())
  {
  str_temp = file1.readStringUntil('\n');
  if (str_temp.length() > SIZE_STR_SPEECH) str_temp = "Очень длинная фраза...";
  file2.seek(SIZE_STR_SPEECH*i, SeekSet);
  file2.print(str_temp + "\n");
  i += 1;
  }
  file2.seek(0, SeekSet); file2.write (i-1);
  file2.seek(300, SeekSet); file2.write (i);
  file1.close();  
  file2.close();  
  str_answer += (String) "Obverse File: " + num + " pass" + "\n"; 
}
sendMessage(chat_id);
xTimerChangePeriod(timerWatchDog,  pdMS_TO_TICKS(10000), 0);
}



void ReverseFile(int num_1, int num_2)
{
str_answer = "";
for (int num = num_1; num <= num_2; num++ )
{
  File file1 = SD.open(String("/" + (String) num + ".txt").c_str());
  File file2 = SD.open(String("/" + (String) num + "A.txt").c_str(), FILE_WRITE);
  file1.seek(0, SeekSet); note = file1.read ();
  for (int k=SIZE_STR_SPEECH; k <= note*SIZE_STR_SPEECH; k+=SIZE_STR_SPEECH)
    {
    file1.seek(k, SeekSet); str_temp = file1.readStringUntil('\n');
    file2.print(str_temp + "\n");
    }
  file1.close();  
  file2.close();  
 str_answer += (String) "Reverse File: " + num + " pass" + "\n";
}
sendMessage(chat_id);
}



void Log()
{
File file = SD.open("/Log.txt", FILE_APPEND);
file.print((String) int64ToAscii(chat_id) + " " + str_first_name + " " + user_id + " " + message_id + "\n" + str_text + "\n\n"); 
file.close();
}
