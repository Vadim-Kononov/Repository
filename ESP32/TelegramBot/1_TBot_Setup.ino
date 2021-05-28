void setup()
{
//Отключение brownout detector
WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
Serial.begin(115200);
if(!SD.begin()){Serial.println("Card Mount Failed"); return;}
Serial.print("Connecting to ");
WiFi.begin("One", "73737373");
while (WiFi.status() != WL_CONNECTED) {delay(500);  Serial.print(".");}
Serial.println();
telegramClient.setCACert(rootCACertificate);

str_response.reserve(SIZE_MAX);
str_first_name.reserve(64);
str_chat_title.reserve(64);
str_text.reserve(SIZE_MIN);
str_speech.reserve(SIZE_STR_SPEECH);
str_answer.reserve(SIZE_MIN);
str_temp.reserve(SIZE_STR_SPEECH);
str_1.reserve(32);
str_2.reserve(32);
str_3.reserve(SIZE_STR_SPEECH);
str_4.reserve(SIZE_STR_SPEECH);
str_5.reserve(SIZE_STR_SPEECH);
str_6.reserve(SIZE_STR_SPEECH);
str_7.reserve(SIZE_STR_SPEECH);
str_8.reserve(SIZE_STR_SPEECH);

//CleanFile("/6.txt");
//CopyFile("/test.txt", "/6.txt");

memory.begin("memory", false);
/*Удаление лишних ключей c именем name*/
//memory.remove("message_id");
/*Удаление всех ключей*/
//memory.clear();
time_waiting = millis();

/*Запуск сторожевого таймера*/
xTimerStart(timerWatchDog, 10);

}
