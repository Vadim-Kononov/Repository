/*Функция малого цикла*/
void Cycle_A ()
{
/*Вывод информации о системном времени*/
DEBUG_PRINTLN(); //DEBUG_PRINTLN (String(xTaskGetTickCount()/1000.0));
/*Если код отправленный платой Main изменился, то прием успешен*/
back_hash != now_Store[0].hash ? flag_Now_connected[0] = true :  flag_Now_connected[0] = false;
/*Сохранения старого значения проверочного кода*/
back_hash = now_Store[0].hash;
/*Установка флага для выполнения дительных операций в цикле Loop*/
flag_Cycle_A = true;
}
/**/



/*Функция большого цикла*/
void Cycle_B ()
{
/*Установка флага для выполнения дительных операций в цикле Loop*/
flag_Cycle_B = true;
}
/**/



/*Функция обратного вызова при приеме данных ESPNOW*/
void Receiv_Now(const uint8_t * mac, const uint8_t *incomingData, int len)
{
digitalWrite(LED_BLUE, HIGH); xTimerStart(timerBlue_Off, 0);

/*Заполнение структуры приема*/
memcpy(&now_Get, incomingData, sizeof(now_Get));
/*Сохранение приятой структуры в элементе массива соответсвующему адресу платы, от которой поступили данные*/
now_Store[now_Get.unit] = now_Get;
//DEBUG_PRINTLN (String(xTaskGetTickCount()/1000.0) + "\tunit= " + String(now_Get.unit) + "\tnow_Store.unit= " + String(now_Store[now_Get.unit].unit)+ "\tnow_Store.hash= " + String(now_Store[now_Get.unit].hash));
/*Перенос принятого от Main проверочного кода для подтверждения приема */
now_Store[ROLE].hash = now_Store[0].hash;
/*Запуск таймера передачи с периодом времени соответсвующим плате*/
if (flag_send_now) xTimerChangePeriod(timerSendNow, pdMS_TO_TICKS(TACT_time * ROLE), 0);
}
/**/



/*Функция отправки данных через ESPNOW*/
void Send_Now ()
{
digitalWrite(LED_GREEN, HIGH); xTimerStart(timerGreen_Off, 0);
esp_err_t result = esp_now_send(NULL, (uint8_t *) &now_Store[ROLE], sizeof(now_Store[ROLE]));
DEBUG_PRINTLN (String("Send_Now ()"));
}
/**/



/*Функция обратного вызова при отправке данных ESPNOW*/
void Sending_Complete_Now(const uint8_t *mac_addr, esp_now_send_status_t status) {}
/**/
