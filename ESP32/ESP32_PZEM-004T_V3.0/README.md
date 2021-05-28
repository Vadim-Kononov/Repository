# ESP32_PZEM-004T_V3.0
### IoT Electricity meter with ESP32 and TFT ST7789 240*240
### Подключение китайского модуля для измерения параметров электрической сети к контроллеру ESP32 и экрану IPS 1,3 TFT ST7789 240*240

PZEM-004T V3.0  |	WEMOS D1 Mini ESP32
----------------|--------------------------
+5V             |	+3.3V
Rx              |	IO17 (u2TxD)
Tx              |	IO16 (u2RxD)
GND             |	GND


TFT ST7789      |	WEMOS D1 Mini ESP32
----------------|--------------------------
GND             |	GND
VCC		|	+3.3V
SCL             |	IO22 (Wire SCL)
SDA             |	IO21 (Wire SDA)
RES             |	IO5
DC              |	IO23
BLK		|	Не подключен

При запуске модуля **AC Digital Multifunction Meter Watt Power Voltage Current Test Module PZEM-004T For Arduino 0-100A 80-260V**.
* Использовал библиотеку https://github.com/mandulaj/PZEM-004T-v30.
* Выяснил, что +5V модуля это вход питания низковольтной части, модуль работает при подаче на вход 3,3V от ESP32 и, тем самым решается вопрос согласования уровней.
* При подключении Rx и Tx в соответствии с приведенной таблицей в библиотеке нужно заменить **&Serial3** на **&Serial2** в примере библиотеки.
* Обмен с контроллером происходит только при подаче напряжения на высоковольтную часть.
* Разница показаний с дешевым мультиметром составила 2В при 240В, 3мА при 300мА, 60мА при 4А.

При запуске экрана **IPS 1.3 inch 3.3V 12PIN SPI HD Full Color TFT Display Screen ST7789 Drive IC 240х240**
* Использовал библиотеки https://github.com/adafruit/Adafruit-GFX-Library и https://github.com/adafruit/Adafruit-ST7735-Library последней версии.
* Так как у модуля отсутствует вывод CS потребовалось внести изменения в пример библиотеки Adafruit-ST7735-Library: 
  * Использовал свои номера пинов ESP32, 
  * `SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI)`, 
  * tft.init(240, 240, `SPI_MODE3`),
* Собирал все в Arduino IDE 1.8.12
