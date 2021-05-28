#include <Wire.h>
 
 
void setup()
{
  Wire.begin();                       
 
  Serial.begin(115200);                 // Задаем скорость обмена - 9600 бод
  while (!Serial);                    // В цикле проверяем сериал порт
  Serial.println("\nI2C Scanner");    // Выводим надпись в терминал "nI2C Scanner"
}
 
 
void loop()
{
  byte error, address;                // 8-ми битное беззнаковое целое число, в диапазоне 0..255
  int nDevices;                       // создаем переменную nDevices
  
  Serial.println("Scanning...");      // Выводим в терминал "Scanning..."
 
  nDevices = 0;                               // Обнуляем переменную nDevices
  for(address = 1; address < 127; address++ ) // В цикле перебираем адрес 1...127
  {
    
    Wire.beginTransmission(address);          // Забираем значение адреса в переменную - address
    error = Wire.endTransmission();           // Забираем значение функции в переменную - error
 
    if (error == 0)                                    // проверяем переменную error, равна ли она - 0
    {                                                  // Если да,то все ок - выполняем код
      Serial.print("I2C device found at address 0x");  // Выводим в терминал "I2C device found at address 0x"
      if (address<16)                                  // Проверяем значение переменной, меньше оно 16?
        Serial.print("0");                             // Выводим 0
      Serial.print(address,HEX);                       // Выводим наш адрес, при этом переводим его в 16 форму исчисления (HEX)  
      Serial.println("  !");                           // Выводим подряд несколько пробелов и ! 
 
      nDevices++;                                      // (инкремент)увеличиваем значение переменной 
    }
    else if (error==4)                                 // Если error равен 4, выполняем следующий код
    {
      Serial.print("Unknow error at address 0x");      // Выводим на экран "Unknow error at adress"
      if (address<16)                                  // И повторяется то, что было выше ... 
        Serial.print("0");                             // Вывод - 0
      Serial.println(address,HEX);                     // Вывод адреса..
    }    
  }
  if (nDevices == 0)                                   // Если же переменная nDevices равна - 0 
    Serial.println("No I2C devices found\n");          // Выводим что никаких устройств обнаружено не было
  else                      
    Serial.println("done\n");                          // Пишем что все хорошо, переводим каретку на новую строку             
 
  delay(5000);                                         // Задержка 5 секунд (задержка нужна, чтобы данные кучей не летели в порт)
}
