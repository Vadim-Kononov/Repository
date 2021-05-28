#include <SoftwareSerial.h>

#define Pin_HC12_RX       9                                       
#define Pin_HC12_TX      10                                       

SoftwareSerial HC12(Pin_HC12_TX,Pin_HC12_RX);

char    Stroka[128];  

void setup()
{
  Serial.begin(9600);                                   
  HC12.begin(9600);
}

void loop()
{
   if(HC12.available())
  {
  HC12.readBytes(Stroka,sizeof(Stroka));
  Serial.print (Stroka);
  }
}
