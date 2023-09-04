//***************************************************************************************************************************//
//############################## created By: Omar_asadi, Haia_Hihi, Mohamad-alwan_aghbaria ##################################//
//###############################################   Date: 24/08/2023    #####################################################//
//## in this file we are trying to read from the DHT 11 sensor the temprture and the humdity of the enviroment using ESP32 ##//
//***************************************************************************************************************************//


#include <DFRobot_DHT11.h>
DFRobot_DHT11 DHT;
#define DHT11_PIN 32

void setup(){
  Serial.begin(115200);
}

void loop(){
  DHT.read(DHT11_PIN);
  Serial.print("temp:");
  Serial.print(DHT.temperature);
  Serial.print("humi:");
  Serial.println(DHT.humidity);
  delay(1000);
}
