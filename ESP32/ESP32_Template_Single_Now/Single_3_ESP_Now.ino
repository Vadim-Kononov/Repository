/*Строковые константы*/
const char * string_now [] PROGMEM = 
{
"Time:\t",                //0
"\t\t\tByte:\t",          //1
"\n\t\t\tMain:\t",        //2
"\n\t\t\tSlave:\t",       //3
"\n\t\t\tCount:\t",       //4
};



/*Функция малого цикла*/
void Small_Cycle_Function ()
{
/*Установка флага для выполнения дительных операций в цикле Loop*/
flag_Small_Cycle_Function = true;

/*Main отправка ESPNOW*/
#if   (ROLE == 1)
xTimerStart(timerMainSend,0);
#endif

/*Сброс сторожевого таймера*/
timerWrite(timerWDT, 0);
}
/**/



/*Функция большого цикла*/
void Big_Cycle_Function ()
{
/*Установка флага для выполнения дительных операций в цикле Loop*/
flag_Big_Cycle_Function = true;
}
/**/



/*Функция обратного вызова при отправке данных ESPNOW*/
void Now_Sending_Complete(const uint8_t *mac_addr, esp_now_send_status_t status) {}
/**/



/*Функция обратного вызова при приеме данных ESPNOW*/
void Now_Receiving_Complete(const uint8_t * mac, const uint8_t *incomingData, int len)
{
/*Заполнение структуры для приема*/
memcpy(&struct_Receiv, incomingData, sizeof(struct_Receiv));

#if   (ROLE == 1)
/*Запуск таймера Main_Print*/
xTimerStart(timerMainPrint, 0);
#elif (ROLE == 2)
/*Запуск таймера Slave_Send*/
xTimerStart(timerSlaveSend, 0);
#endif
}
/**/



/*Функция отправки данных платой Main через ESPNOW*/
void Main_Send ()
{
struct_Send.var1 = ESP.getFreeHeap();
struct_Send.var2 = memory.getInt("countReset");
struct_Send.var3 = memory.getInt("countWifi");
struct_Send.var4 = struct_Receiv.var4;
struct_Send.var5 = struct_Receiv.var5;
struct_Send.var6 = struct_Receiv.var6;
struct_Send.var7 = struct_Receiv.var7;

esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &struct_Send, sizeof(struct_Send));
}
/**/



/*Функция отправки данных платой Slave через ESPNOW*/
void Slave_Send ()
{
/*Модификация принятых данных*/
struct_Send.var1 = struct_Receiv.var1;
struct_Send.var2 = struct_Receiv.var2;
struct_Send.var3 = struct_Receiv.var3;
struct_Send.var4 = ESP.getFreeHeap();
struct_Send.var5 = memory.getInt("countReset");
struct_Send.var6 = memory.getInt("countWifi");
struct_Send.var7 = struct_Receiv.var7 + 1;

esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &struct_Send, sizeof(struct_Send));
}
/**/



/*Функция вывода принятых через через ESPNOW данных платой Main */
void Main_Print ()
{
DEBUG_PRINTLN(String(string_now [0]) + String(xTaskGetTickCount()/1000.0));

  if (flag_WiFi_connected)
  {
  /*Запуск таймера mqtt_Send*/
  xTimerStart(timerMqttSend, 0);
  }

DEBUG_PRINTLN(String (string_now [1]) + String(sizeof(struct_Send))
+ String(string_now [2]) + String(struct_Receiv.var1) + "\t" + String(struct_Receiv.var2) + "\t" + String(struct_Receiv.var3)
+ String(string_now [3]) + String(struct_Receiv.var4) + "\t" + String(struct_Receiv.var5) + "\t" + String(struct_Receiv.var6)
+ String(string_now [4]) + String(struct_Receiv.var7));
}
/**/
