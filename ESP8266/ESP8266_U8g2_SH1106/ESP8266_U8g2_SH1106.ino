
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>


//U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);
U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   

String Str1  = "Str1 ";
String Str2  = "Строка 2 ";
String Str3  = "Привет ";
String Str4  = "Str4 ";
int i = 255, j = 90, k =35, l = 1, n = 0, p = 40, q = 4;
boolean m = true, r = true;

void setup(void)
{
   u8g2.begin();  
   u8g2.enableUTF8Print(); 
   
}

void loop(void)
{
  
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_profont22_tf);
    u8g2.setCursor(0, 15);
    u8g2.print(Str1+i);
    
    u8g2.setFont(u8g2_font_profont11_tf);
    if (l>=10) u8g2.setCursor(j-4, k+4); else u8g2.setCursor(j-2, k+4);
    u8g2.print(l);  
    u8g2.drawCircle(j, k, l, U8G2_DRAW_ALL);
    
    u8g2.setFont(u8g2_font_cu12_t_cyrillic);
    if (m)  u8g2.setCursor(0, 40); else u8g2.setCursor(10, 40);
    u8g2.print(Str2);
    
    u8g2.setFont(u8g2_font_10x20_t_cyrillic);
    if (n==2) u8g2.setCursor(10, 50); else u8g2.setCursor(0, 60);
    u8g2.setFontDirection(n);
    u8g2.print(Str3);
    u8g2.setFontDirection(0);
    
   
    
    
  } while ( u8g2.nextPage() );
  
  delay(25);
    
  i--;
  
  q--; if (q<=0) {q = 4; r =!r;}
  
  if (i<=100 && !r) u8g2.setContrast(0); if (i<=100 && r) u8g2.setContrast(255);
    
  if (l>=20) m = false;
  if (l<=0)  m = true;
  if (m) l++; 
  if (!m) l--;
  if (i<=0) {i = 255; u8g2.setContrast(255);}

  p = p-1;
  if (p==0){p=40; if (n==2) n=0; else n=2; }
}
