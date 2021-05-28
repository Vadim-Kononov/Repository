/*Функция малого цикла*/
void Small_Cycle ()
{
flag_Small_Cycle = true;
/*Сброс сторожевого таймера*/
timerWrite(timerWDT, 0);
}
/**/



/*Функция обратного вызова при приеме данных ESPNOW*/
void Receiv_Now(const uint8_t * mac, const uint8_t *incomingData, int len)
{
/*Заполнение структуры приема*/
memcpy(&now_Get, incomingData, sizeof(now_Get));
DEBUG_PRINTLN("Receiv unit: " + String(now_Get.unit));
/*Сохранение приятой структуры в элементе массива соответсвующему адресу платы, от которой поступили данные*/
now_Store[now_Get.unit] = now_Get;

}
/**/



/*Функция большого цикла*/
void Big_Cycle ()
{
/*Установка флага для выполнения дительных операций в цикле Loop*/
flag_Big_Cycle = true;
}
/**/



/*Функция отправки данных через ESPNOW*/
void Send_Now ()
{
esp_err_t result = esp_now_send(NULL, (uint8_t *) &now_Store[ROLE], sizeof(now_Store[ROLE]));
}
/**/



/*Функция обратного вызова при отправке данных ESPNOW*/
void Sending_Complete_Now(const uint8_t *mac_addr, esp_now_send_status_t status) {}
/**/
