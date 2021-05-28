//Core graphics library by Adafruit
#include <Adafruit_GFX.h>    
//Hardware-specific library for ST7789 (with or without CS pin)
#include <Arduino_ST7789.h> 
#include <SPI.h>

//Подключение шрифтов
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>

//Соответсвие пинов 
#define TFT_DC    23    //DC
#define TFT_RST   5     //RES
#define TFT_MOSI  21    //SDA
#define TFT_SCLK  22    //SCL

//Для дисплея без CS пина
Arduino_ST7789 tft = Arduino_ST7789(TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK);

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

//Начало координат вращающегося треугольника, задание заливки эерана
const int X0 = 200, Y0 = 200, L = 30, color = tft.color565(0, 0, 7);
//Коэффициент для перевода градусов в радианы
const float rad = 3.1416/180;



void setup(void)
{
Serial.begin(115200);
//initialize a ST7789 chip, 240x240 pixels
tft.init(240, 240);   
//Поворот экрана корректно работают 2 и 3
tft.setRotation(2);
//Заливка экрана
tft.fillScreen(color);
//Размер шрифта
tft.setTextSize(3);
//Горизонтальная линия
tft.drawFastHLine(40, 120, 160, tft.color565(0, 0, 255));
//Вертикальная линия
tft.drawFastVLine(120, 40, 160, tft.color565(0, 0, 255));
//Круг
tft.fillCircle(120, 120, 8, color);
//Окружность
tft.drawCircle(X0, Y0, 10, WHITE);
tft.drawCircle(X0, Y0, 36, WHITE);



//Линия
//tft.drawLine(x1, y1, x2, y2, WHITE);
//Пиксель
//tft.drawPixel(x, y, WHITE);
//Окружность
//tft.drawCircle(x, y, r, WHITE);
//Круг
//tft.fillCircle(x, y, r, WHITE);
//Прямоугольник
//tft.drawRect(x, y, w, h, WHITE);
//Прямоугольник залитый
//tft.fillRect(x, y, w, h, WHITE);
//Прямоугольник с скругленными углами
//tft.drawRoundRect(x, y, w, h, r, WHITE);
//Прямоугольник залитый с скругленными углами
//tft.fillRoundRect(x, y, w, h, r, WHITE);
//Треугольник, шесть координат трех углов
//tft.drawTriangle(x1, y1, x2, y2, x3, y3, WHITE);
//Треугольник залитый, шесть координат трех углов
//tft.fillTriangle(x1, y1, x2, y2, x3, y3, WHITE);
//Символ
//tft.drawChar(120, 120, '@', WHITE, BLACK, 10);
//Установка курсора
//tft.setCursor(0, 0);
//Цвет шрифта, фон прозрачный
//tft.setTextColor(WHITE);
//Цвет шрифта, цвет фона
//tft.setTextColor(WHITE, BLACK);
//Перенос строк
//tft.setTextWrap(0);
//Инверсия экрана
//tft.invertDisplay(1);
//Задание шрифта
//tft.setFont(&FreeMono9pt7b);
//tft.setFont(&FreeSerif9pt7b);
//Задание шрифта по умолчанию
//tft.setFont();
}



void loop()
{
int i, x1, y1, x2, y2, x3, y3, col = 1;
for (int i = 0; i<=360; i += 15)
{
//Мигание кругом
col == 1 ? col = 0 : col = 1;
tft.fillCircle(X0, Y0, 3, tft.color565(col*255, col*255 , col*255));
//Текст
tft.setTextColor(GREEN, color);
tft.setCursor(30, 60);
tft.println(String(i) + "  ");
//Вращающийся треугольник
tft.drawTriangle(x1, y1, x2, y2, x3, y3, color);
x1 = cos(i*rad)*L+X0;
y1 = sin(i*rad)*L+Y0;
x2 = cos((i+120)*rad)*L+X0;
y2 = sin((i+120)*rad)*L+Y0;
x3 = cos((i-120)*rad)*L+X0;
y3 = sin((i-120)*rad)*L+Y0;
tft.drawTriangle(x1, y1, x2, y2, x3, y3, WHITE);
//delay (100);
}
}
