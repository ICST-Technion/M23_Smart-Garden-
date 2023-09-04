//********************************************************************************************************************************************//
//##################################### created By: Omar_asadi, Haia_Hihi, Mohamad-alwan_aghbaria ############################################//
//######################################################   Date: 24/08/2023    ###############################################################//
//## in this file we are trying to read from the Capactive soil moisture sensor the temprture and the humdity of the enviroment using ESP32 ##//
//********************************************************************************************************************************************//


const int analogInPin= 32;
int sensorvalue;
int outputvalue;



void setup() {
  Serial.begin(9600);

}

void loop() {
  sensorvalue=analogRead(analogInPin);
  Serial.print("Moisture value is:");
  Serial.println(sensorvalue);
  delay(500);
}
