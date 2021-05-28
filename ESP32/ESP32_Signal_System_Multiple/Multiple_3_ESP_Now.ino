/*Функция малого цикла*/
void Cycle_A ()
{
/*Вывод информации о системном времени*/
DEBUG_PRINTLN(); DEBUG_PRINTLN (String(xTaskGetTickCount()/1000.0));
/*>-----------< Выбор Роли >-----------<*/
/*Изменение отправляемого кода, запуск таймера MQTT на три периода задержки, отправка Now сообщения*/
#if   (ROLE == 0)
if (now_Store[1].hash == now_Store[0].hash) flag_Now_connected[1] = true ; else {flag_Now_connected[1] = false; board_count [1] ++;}
if (now_Store[2].hash == now_Store[0].hash) flag_Now_connected[2] = true ; else {flag_Now_connected[2] = false; board_count [2] ++;}
board_count [0] ++;
now_Store[0].hash  = random(100, 1000);
xTimerChangePeriod(timerSendNow,  pdMS_TO_TICKS(TACT_time)                ,   0);
xTimerChangePeriod(timerMqttSend, pdMS_TO_TICKS(TACT_time*(BOARD_COUNT+1)),   0);
/*>-----------< Выбор Роли >-----------<*/
#elif (ROLE == 1 || ROLE == 2)
/*Если код отправленный платой Main изменился, то прием успешен*/
back_hash != now_Store[0].hash ? flag_Now_connected[0] = true :  flag_Now_connected[0] = false;
/*Сохранения старого значения проверочного кода*/
back_hash = now_Store[0].hash;
#endif
/*>-----------< Выбор Роли >-----------<*/
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
/*Заполнение структуры приема*/
memcpy(&now_Get, incomingData, sizeof(now_Get));
/*Сохранение приятой структуры в элементе массива соответсвующему адресу платы, от которой поступили данные*/
now_Store[now_Get.unit] = now_Get;

DEBUG_PRINTLN (String(xTaskGetTickCount()/1000.0) + "\tunit= " + String(now_Get.unit) + "\tnow_Store.unit= " + String(now_Store[now_Get.unit].unit)+ "\tnow_Store.hash= " + String(now_Store[now_Get.unit].hash));
/*>-----------< Выбор Роли >-----------<*/
/*Для Slave плат*/
#if (ROLE == 1 || ROLE == 2)
/*Перенос принятого от Main проверочного кода для подтверждения приема */
now_Store[ROLE].hash = now_Store[0].hash;
/*Запуск таймера передачи с периодом времени соответсвующим плате*/
xTimerChangePeriod(timerSendNow, pdMS_TO_TICKS(TACT_time * ROLE), 0);
#endif
/*>-----------< Выбор Роли >-----------<*/
}
/**/



/*Функция отправки данных через ESPNOW*/
void Send_Now ()
{
esp_err_t result = esp_now_send(NULL, (uint8_t *) &now_Store[ROLE], sizeof(now_Store[ROLE]));
DEBUG_PRINTLN (String(xTaskGetTickCount()/1000.0) + "\tSend_Now ()");
}
/**/



/*Функция обратного вызова при отправке данных ESPNOW*/
void Sending_Complete_Now(const uint8_t *mac_addr, esp_now_send_status_t status) {}
/**/
