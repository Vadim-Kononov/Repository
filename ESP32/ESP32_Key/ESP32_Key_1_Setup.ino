void setup()
{
//Отключение brownout detector
WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

//Настройка пинов
pinMode(LED_ONBOARD, OUTPUT);
pinMode(RELAY_PIN, OUTPUT);
pinMode(LED_RED, OUTPUT);
pinMode(LED_GREEN, OUTPUT);
pinMode(LED_BLUE, OUTPUT);

//Выключение реле
digitalWrite(RELAY_PIN, LOW);
relay_flag = false;

//Выключение светодиодов
digitalWrite(LED_RED, LOW);
digitalWrite(LED_GREEN, LOW);
digitalWrite(LED_BLUE, LOW);

//Инициализация EEPROM
memory.begin("memory", false);

//Подключение к BTSerial
SerialBT.begin(NAME);

//Настройка WDT
timer = timerBegin(0, 80, true);                  //Timer 0, div 80
timerAttachInterrupt(timer, &ResetModule, true);  //Attach callback
timerAlarmWrite(timer, wdtTimeout * 1000, false); //Set time in us
timerAlarmEnable(timer);                          //Enable interrupt

//Загрузка таблицы из EEPROM
memory.getBytes("mem", (detector*)detect_buf, sizeof(detect_buf));

//Включение приемника 433 МГц
receiver.enableReceive(RECEIVER_PIN);

//Установка 2 сек. задержки на получение кода
time_ms_Code = 2000;
time_ms_Led = 125;
}
