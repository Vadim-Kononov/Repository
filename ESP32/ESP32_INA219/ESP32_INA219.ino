#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

void setup(void) 
{
Serial.begin(115200); 

//By default the initialization will use the largest range (32V, 2A).  
//ina219.setCalibration_32V_1A();
//ina219.setCalibration_16V_400mA();

ina219.begin();
Serial.println("Measuring voltage and current with INA219 ...");
}

void loop(void) 
{
  float busvoltage = 0;
  float current_mA = 0;
 
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
   
  Serial.print("Voltage: "); Serial.print(busvoltage); Serial.print(" V        ");
  Serial.print("Current: "); Serial.print(current_mA); Serial.println(" mA");
  
  delay(1000);
}
