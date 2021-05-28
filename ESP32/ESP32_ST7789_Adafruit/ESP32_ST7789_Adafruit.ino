#include <Adafruit_GFX.h>    
#include <Adafruit_ST7789.h> 
#include <SPI.h>

//Пины 
#define TFT_CS    0     //Значение для работы библиотеки, не подключается
#define TFT_MISO  0     //Значение для работы библиотеки, не подключается
#define TFT_RST   5   //RES
#define TFT_DC    23    //DC
#define TFT_MOSI  21    //SDA
#define TFT_SCLK  22    //SCL

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

bool flag_update = true;
int color = tft.color565(0, 0, 32), y_pos = 270, x_pos_1, x_pos_2, text_size_1, text_size_2, color_line, parameter = 0;
String value, unit;


void setup(void)
{
Serial.begin(115200);
//initialize a ST7789 chip, 240x240 pixels
SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI);
tft.init(240, 240, SPI_MODE3);  
//Поворот экрана корректно работают 2 и 3
tft.setRotation(2);
//Заливка экрана
tft.fillScreen(color);
//Размер шрифта
tft.setTextSize(7);
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
  case 0:
    color = tft.color565(0, 0, 32);
    tft.fillScreen(color);
    tft.setTextColor(GREEN, color);
    color_line = GREEN;
    text_size_1 = 10;
    text_size_2 = 7;
    value = "220.5";
    unit = "V";
    x_pos_1 = 100;
    x_pos_2 = 20;
    break;
  case 1:
    color = tft.color565(0, 0, 0);
    tft.fillScreen(color);
    tft.setTextColor(RED, color);
    color_line = RED;
    text_size_1 = 10;
    text_size_2 = 8;
    value = "5.6";
    unit = "A";
    x_pos_1 = 100;
    x_pos_2 = 55;
    break;
   
  case 2:
    color = tft.color565(0, 16, 16);
    tft.fillScreen(color);
    tft.setTextColor(CYAN, color);
    color_line = CYAN;
    text_size_1 = 8;
    text_size_2 = 8;
    value = "12";
    unit = "kWh";
    x_pos_1 = 50;
    x_pos_2 = 70;
    break;

  case 3:
    color = tft.color565(8, 0, 8);
    tft.fillScreen(color);
    tft.setTextColor(YELLOW, color);
    color_line = YELLOW;
    text_size_1 = 8;
    text_size_2 = 7;
    value = "0.56";
    unit = "cos";
    x_pos_1 = 50;
    x_pos_2 = 40;
    break;

}
}




tft.setTextSize(text_size_1);
tft.setCursor(x_pos_1, y_pos);
tft.print (unit);
tft.drawFastHLine(40, y_pos + 85, 160, color_line);
tft.setTextSize(text_size_2);
tft.setCursor(x_pos_2, y_pos + 110);
tft.print (value);

//delay (10);

tft.drawRect(10, y_pos+90, 210, 5, color);
//tft.fillScreen(color);
y_pos--;
if (y_pos < -180) {y_pos = 240; parameter ++; flag_update = true;}



}
