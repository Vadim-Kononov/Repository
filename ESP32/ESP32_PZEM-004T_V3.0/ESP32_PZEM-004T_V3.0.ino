/*Измерение мгновенных значений напряжения, силы тока, потребляемой мощности, косинуса фи, расход за час и за месяц электроэнергии и денег*/
/*Значения выводятся на экран 240*240 в режиме скроллинга и отправляются в облачное хранилище*/
#include <Adafruit_GFX.h>    
#include <Adafruit_ST7789.h> 
#include <SPI.h>

#include <PZEM004Tv30.h>
PZEM004Tv30 pzem(&Serial2);

//Пины
//TFT ST7789
#define TFT_CS    0     //Значение для работы библиотеки, не подключается
#define TFT_MISO  0     //Значение для работы библиотеки, не подключается
#define TFT_RST   5     //RES
#define TFT_DC    23    //DC
#define TFT_MOSI  21    //SDA
#define TFT_SCLK  22    //SCL

//PZEM          ESP32
//+5V           +3.3V
//Rx            IO17 (u2TxD)
//Tx            IO16 (u2RxD)
//GND           GND

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

//Набор цветов
#define BLACK       0x0000
#define NAVY        0x000F
#define DARKGREEN   0x03E0
#define DARKCYAN    0x03EF
#define MAROON      0x7800
#define PURPLE      0x780F
#define OLIVE       0x7BE0
#define LIGHTGREY   0xC618
#define DARKGREY    0x7BEF
#define BLUE        0x001F
#define GREEN       0x07E0
#define CYAN        0x07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define WHITE       0xFFFF
#define ORANGE      0xFD20
#define GREENYELLOW 0xAFE5
#define PINK        0xF81F

//Флаг начала измерения
bool flag_update = true;                                                                            
//Цвет заливки экрана, позиции и размер текста для единиц измерения и значений параметров, цвет подчеркивания, номер измеряемого параметра
int color, y_pos = 270, x_pos_1, x_pos_2, text_size_1, text_size_2, color_line, parameter = 0;
//Напряжение, сила тока, мощность, расход электроэнергии, частота, косинус фи
float voltage, current, power, energy, frequency, pf;             
//Вспомогательный символьный массив
char myStr[10];
//Строковые значения параметров и единиц измерения
String value, unit;



void setup(void)
{
Serial.begin(115200);
//Инициализация экрана
SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI);
tft.init(240, 240, SPI_MODE3);  
tft.setRotation(2);
tft.setTextWrap(0);
}



void loop()
{
if (flag_update)
{
  flag_update = false;
  if (parameter>=4) parameter =0;
  switch (parameter)
  {
    //Напряжение
    case 0:
      Preset (0, 0, 16, GREEN);
      voltage = round(pzem.voltage());
      //Значение для отладки
      //voltage = round(220.5);
      if (voltage >= 100 && voltage <= 400) {value = String(int(voltage)); text_size_2 = 8; x_pos_2 = 55;} else Print_NaN();
      unit = "V";   
      text_size_1 = 8;
      x_pos_1 = 100;
      break;
    
    //Сила тока
    case 1:
      Preset (0, 0, 0, PINK);
      current = round(pzem.current());
      //Значение для отладки
      //current = round(0.9*10)/10.0;
      if (current >= 0.0 && current <= 99.0) {dtostrf(current, 3, 1, myStr);  value = String(myStr);  text_size_2 = 8; (current < 10.0) ? x_pos_2 = 55 : x_pos_2 = 30;} else Print_NaN();   
      unit = "A";     
      text_size_1 = 8;
      x_pos_1 = 100;
      break;
    
    //Мгновенная мощность
    case 2:
      Preset (0, 0, 16, CYAN);
      power = round(pzem.power());
      //Значение для отладки
      //power = round(9000);
      if (power >= 0 && power < 10000) {value = String(int(power)); text_size_2 = 8; if (power < 10.0) x_pos_2 = 105; else if (power < 100.0) x_pos_2 = 80; else if (power < 1000.0) x_pos_2 = 55; else x_pos_2 = 35;} else Print_NaN();
      unit = "W";
      text_size_1 = 8;
      x_pos_1 = 100;
      break;
      
    //Косинус фи
    case 3:
      Preset (0, 0, 0, YELLOW);
      pf = pzem.pf();
      //Значение для отладки
      //pf = 0.98;
      if (pf >= 0.0 && pf <= 2.0) {value = pf; text_size_2 = 8; x_pos_2 = 30;} else Print_NaN(); 
      unit = "cos";
      text_size_1 = 8;
      x_pos_1 = 50;
      break;
  }
}



/*Скроллинг на один пиксель вверх за один цикл loop()*/
//Единица измерения
tft.setTextSize(text_size_1);
tft.setCursor(x_pos_1, y_pos);
tft.print (unit);
//Линия подчеркивания
tft.drawFastHLine(30, y_pos + 95, 180, color_line);
//Значение параметра
tft.setTextSize(text_size_2);
tft.setCursor(x_pos_2, y_pos + 140);
tft.print (value);
//Стирающий снизу прямоугольник
tft.drawRect(10, y_pos + 98, 210, 5, color);
//Уменьшение координаты по y
y_pos--;
//Сброс координаты вниз, начало нового цикла измерения и скроллинга
if (y_pos < -200) {y_pos = 240; parameter ++; flag_update = true;}



}

void Preset (int r, int g, int b, int line)
  {
  color = tft.color565(r, g, b);
  tft.fillScreen(color);
  tft.setTextColor(line, color);
  color_line = line;
  }

void Print_NaN ()
  {
  value = "NaN";
  text_size_2 = 7;
  x_pos_2 = 60;
  }
