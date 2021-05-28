## Electricity Meter with PZEM-004T v3.0, ESP32 and display ST7789

## Счетчик электроэнергии на модулях PZEM-004T v3.0, ESP32 и экране ST7789

### Измерение параметров энергопотребления, отправка на смартфон по MQTT, отправка в облако на ThingSpeak, оповещения через IFTTT, управление через Bluetooth Terminal

### Сборка в Arduino IDE

* Функциональность
  * Измерение и вычисление
    * расхода электроэнергии с коррекцией показаний под основной счётчик
    * расхода электроэнергии за месяц, с заданием числа месяца для начала отсчёта
    * стоимости электроэнергии за месяц, с заданием разных цен для потребления в объеме социального лимита и превышающего этот объём значения
    * текущей мощности
    * текущего напряжения
    * текущей силы тока
    * коэффициента мощности
    * частоты сети
  * Вывод параметров
    * на экран счетчика
    * на экран смартфона
    * в облачную базу данных ThingSpeak, с возможностью видеть графики параметров
  * Управление, изменение настроек по WiFi и Bluetooth каналам
  * Получение уведомлений
    * при превышении заданной мощности
    * при превышение заданной стоимости

* При сборке столкнулся со следующими, не очевидными для меня, моментами
  * При запуске модуля **PZEM-004T v3.0**
    * выяснил, что +5V модуля это вход питания низковольтной части, модуль работает при подаче на этот вход 3,3V от ESP32 и, тем самым, решается вопрос согласования уровней
    * при подключении Rx и Tx в соответствии с приведенной ниже таблицей в библиотеке  https://github.com/mandulaj/PZEM-004T-v30 нужно заменить **&Serial3** на **&Serial2**
    * обмен с контроллером происходит только при подаче напряжения на высоковольтную часть
    * разница показаний с дешевым мультиметром составила 2В при 240В, 3мА при 300мА, 60мА при 4А

  * При запуске экрана **IPS 1.3 inch 3.3V SPI ST7789 Drive IC 240х240**
    * использовал библиотеки https://github.com/adafruit/Adafruit-GFX-Library и https://github.com/adafruit/Adafruit-ST7735-Library последних версий
    * так как у модуля отсутствует вывод CS и для использования “своих” пинов потребовалось внести изменение в пример библиотеки Adafruit-ST7735-Library: 
      * `SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI)`
      * tft.init(240, 240, `SPI_MODE3`)
      
* Таблица соединений

PZEM-004T V3.0	|WEMOS D1 Mini ESP32
----------------|--------------------------
+5V             |+3.3V
Rx              |IO17 (u2TxD)
Tx              |IO16 (u2RxD)
GND             |GND


TFT ST7789     	|WEMOS D1 Mini ESP32
----------------|--------------------------
GND             |GND
VCC             |+3.3V
SCL             |IO22 (Wire SCL)
SDA             |IO21 (Wire SDA)
RES             |IO5
DC              |IO23
BLK             |Не подключен

* Команды, используемые для проверки состояния или изменения настроек, передаваемые Bluetooth Terminal


Команда                  |Действие
-------------------------|------------------------------------------------------------------
Password                 |Ввод пароля, отключение введенного пароля
Login/[SSID]/[Password]  |Запись в память логина и пароля WiFi, ввод-вывод, String/String
Reconnect                |Переподключение WiFi по логину и паролю хранящимися в памяти
Reset                    |Перезагрузка
Count                    |Вывод счетчиков перезагрузок и реконнектов
CountRes                 |Сброс счетчиков перезагрузок и реконнектов
WiFi                     |Вывод текущих параметров WiFi
Scan                     |Сканирование каналов WiFi
Time                     |Вывод текущего времени
Mem                      |Вывод количества свободной памяти
OTA                      |Разрешение загрузки по воздуху
PZEM/Main/[kWh]          |Ввод текущего значения счетчика, ввод-вывод, float
PZEM/Before/[kWh]        |Ввод значения счетчика на день начала отсчета месячного расхода, ввод-вывод, float
PZEM/Correct/[kWh]       |Коррекция расхода электроэнергии приведенная к одному кВт*ч, "-" если счетчик спешит, "+" если отстает, ввод-вывод, float
PZEM/Date/[day]/[month]  |Ввод дня и месяца начала отсчета расхода, ввод-вывод, int/int
PZEM/Delay/[sec]         |Ввод задержки скроллинга на экране счетчика, ввод-вывод, float
PZEM/PriceLow/[RUR]      |Ввод цены киловатта в пределах социального лимита, ввод-вывод, float
PZEM/PriceHigh/[RUR]     |Ввод цены киловатта за пределом социального лимита, ввод-вывод, float  
PZEM/LowLimit/[kWh]      |Ввод порога социального лимита, ввод-вывод, float 
PZEM/AlarmPower/[kWh]    |Ввод значения мощности, при превышении которого присылается оповещение, ввод-вывод, float 
PZEM/AlarmRate/[RUR]     |Ввод значения стоимости, при превышении которого присылается оповещение, ввод-вывод, float 
Help                     |Вывод списка команд
?                        |Вывод списка команд
  
  
  
### Используемое ПО, с благодарностью к авторам
Приложение                | Ссылка
--------------------------|--------------------------
mandulaj/PZEM-004T-v30    |https://github.com/mandulaj/PZEM-004T-v30
IoT MQTT Panel Pro        |https://play.google.com/store/apps/details?id=snr.lab.iotmqttpanel.prod.pro
Serial Bluetooth Terminal |https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal
IFTTT                     |https://play.google.com/store/apps/details?id=com.ifttt.ifttt)
ThingView Full            |https://play.google.com/store/apps/details?id=com.cinetica_tech.thingview.full

-------------------------------------------------------------------------------------------------------------------------------------
### Посмотреть на результат:
Сервис     | Ссылка
-----------|--------------------
ThingSpeak |https://thingspeak.com/channels/1061956
YouTube    |https://youtu.be/sH5VCsjLUSs    

  
  