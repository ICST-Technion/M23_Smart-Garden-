
//###########################################################################################################//
//############## Created by: Asadi_Omar, Hihi_Haya, Egbaria_Mohamad_Elwan. Date:03/09/23  #################//
// Descriptin: in the following file we are reading both tempreture and humadity sensor and capactive sensor// 
//#### after reading the sensors we are uploading the data to our real-time data base the we've created ####// 
//#########################################################################################################//

#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
//#include <WiFiManager.h>
#include <iostream>
#include <stdio.h>


#endif

//Provide the token generation process info.
#include <addons/TokenHelper.h>
#include <DFRobot_DHT11.h>
#include <WiFiManager.h>

//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
/* 1. Define the WiFi credentials */
//const char* WIFI_SSID = "ICST";
//const char* WIFI_PASSWORD = "arduino123";
//For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino
/* 2. Define the API Key */
#define API_KEY "q5GRF4P379NZpHPnG2KmczcFzm43yw8NDEPpVmHg"
/* 3. Define the RTDB URL */
#define DATABASE_URL "https://temperature-sensor-test-46ad3-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WiFiManager wm;
std::vector<const char *> menu = {"wifi","info"};

int hdt_tempreture, hdt_humidity;
int soil_moisture, soil_moisture_percent, soil_moisture_read;
const int SOIL_MOISTURE_PIN= 32;
const int DHT11_PIN= 33;
int count=0, i=0, soil_moisture_avg=0;
int last_values[5]={0};
int moisture_states=0; // the values represent to 4 states: 0-it's too wet , 1-wet, 2-meduim , 3-dry , 4-it's too dry
const float alpha=0.1;
double lastAvg=0.0;
DFRobot_DHT11 DHT;

//calibration variables
int max_too_wet=0,max_wet=0,max_meduim=0,min_dry=0,max_dry=0;
int perfect_plant_water_level=2;
int exec=0,prev_exec=0;
int manual_start_irrigation=0;

bool connection;


void setup()
{
  Serial.begin(115200);

  /// Connection to the wifi with wifimanager
  wm.resetSettings();
  wm.setMenu(menu);
  connection = wm.autoConnect("SmartGarden");
  if(!connection) {
    Serial.println("Failed to connect");
  } 
  //if you get here you have connected to the WiFi    
  Serial.println("Wifi Connected");
  


  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  /* Assign the api key (required) */
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;


  //////////////////////////////////////////////////////////////////////////////////////////////
  //Please make sure the device free Heap is not lower than 80 k for ESP32 and 10 k for ESP8266,
  //otherwise the SSL connection will fail.
  //////////////////////////////////////////////////////////////////////////////////////////////
  Firebase.begin(DATABASE_URL, API_KEY);
  //Comment or pass false value when WiFi reconnection will control by your code or third party library
 // Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
  pinMode(2, OUTPUT);

}

//return the state of the soil
int getState(int soil_moisture_percent, int max_wet, int max_meduim, int max_dry, int min_dry){
    if(soil_moisture_percent>max_wet){
      return 0; // means it's too wet
    }else if(soil_moisture_percent>max_meduim){
      return 1; //means it's wet
    }else if(soil_moisture_percent>max_dry){
      return 2; //means it's too meduim
    }else if(soil_moisture_percent>min_dry){
      return 3; //means it's too dry
    }else{
    return 4;
    }
}



void loop()
{
  if(WiFi.status() != WL_CONNECTED){
    delay(100);
    if(WiFi.status() != WL_CONNECTED){
      Serial.println("Wifi Disconnected");
      digitalWrite(2, LOW); //stop the irrigation until reconnection
      connection = wm.autoConnect("SmartGarden");
      if(!connection) {
        Serial.println("Failed to connect");
      } 
      //if you get here you have connected to the WiFi    
      Serial.println("Wifi Connected");
    }
    
  }
  DHT.read(DHT11_PIN);
  hdt_tempreture = DHT.temperature;
  hdt_humidity = DHT.humidity;
  delay(100);
  soil_moisture_read = analogRead(SOIL_MOISTURE_PIN);

  //exponential filter
  double curr_avg=0.0;
  curr_avg= (alpha*soil_moisture_read)+((1-alpha)*lastAvg);
  lastAvg=curr_avg;
  soil_moisture=int(curr_avg);
    
  //converting the  returned value from the capacitive soil mooisture to a percentage  
  soil_moisture_percent = ( 100 - ( (soil_moisture/4095.00) * 100 ) ); 

    
  if (Firebase.ready()){
    Firebase.setInt(fbdo, "/Robophone/5669122872442880/tempreture",hdt_tempreture);
    Firebase.setInt(fbdo, "/Robophone/5669122872442880/humidity",hdt_humidity);
    Firebase.setInt(fbdo, "/Robophone/5669122872442880/soil_moisture",soil_moisture);
    Firebase.setInt(fbdo, "/Robophone/5669122872442880/soil_moisture_percent",soil_moisture_percent);


    if(Firebase.getInt(fbdo, "/Robophone/5669122872442880/exec")){
      exec=fbdo.to<int>();
    }else{
      Serial.printf(fbdo.errorReason().c_str());
      exec=0;
    }
  
  //means it didn't start yet!
  if(exec==0){
    digitalWrite(2, LOW);
    prev_exec=0;
    max_too_wet=0;
    max_wet=0;
    max_meduim=0;
    max_dry=0;
    min_dry=0;
  }else{ //means the system has started and we should process the data (exec=1)

    //update the calibration values if we need to update
    //if exec != prev_exec it means that this is the first itteration after calibration or choosing new plant so we need to update the new values
    if(exec != prev_exec){
      int tmp_too_wet_min, tmp_too_wet_max , tmp_wet_min,tmp_wet_max,tmp_meduim_min,tmp_meduim_max,tmp_dry_min,tmp_dry_max;
      exec=0;
      while(exec == 0){ //if we entered after the first iteration means one or more of the reading from the DB has failed so try againg, after that we can keep moving
        exec=1;
        if(Firebase.getInt(fbdo, "/Robophone/5669122872442880/Calibration/dry/max")){
        tmp_dry_max=fbdo.to<int>();
        }else{
          Serial.printf(fbdo.errorReason().c_str());
          Serial.println(); 
          exec=0;
        }
        if(Firebase.getInt(fbdo, "/Robophone/5669122872442880/Calibration/dry/min")){
          tmp_dry_min=fbdo.to<int>();
        }else{
          Serial.printf(fbdo.errorReason().c_str());
          Serial.println(); 
          exec=0;
        }
        if(Firebase.getInt(fbdo, "/Robophone/5669122872442880/Calibration/medium/max")){
          tmp_meduim_max=fbdo.to<int>();
        }else{
          Serial.printf(fbdo.errorReason().c_str());
          Serial.println(); 
          exec=0;
        }
        if(Firebase.getInt(fbdo, "/Robophone/5669122872442880/Calibration/medium/min")){
          tmp_meduim_min=fbdo.to<int>();
        }else{
          Serial.printf(fbdo.errorReason().c_str());
          Serial.println(); 
          exec=0;
        }
        if(Firebase.getInt(fbdo, "/Robophone/5669122872442880/Calibration/wet/max")){
          tmp_wet_max=fbdo.to<int>();
        }else{
          Serial.printf(fbdo.errorReason().c_str());
          Serial.println(); 
          exec=0;
        }
        if(Firebase.getInt(fbdo, "/Robophone/5669122872442880/Calibration/wet/min")){
          tmp_wet_min=fbdo.to<int>();
        }else{
          Serial.printf(fbdo.errorReason().c_str());
          Serial.println(); 
          exec=0;
        }
        if(Firebase.getInt(fbdo, "/Robophone/5669122872442880/Calibration/tooWet/max")){
          tmp_too_wet_max=fbdo.to<int>();
        }else{
          Serial.printf(fbdo.errorReason().c_str());
          Serial.println(); 
          exec=0;
        }
        if(Firebase.getInt(fbdo, "/Robophone/5669122872442880/Calibration/tooWet/min")){
          tmp_too_wet_min=fbdo.to<int>();
        }else{
          Serial.printf(fbdo.errorReason().c_str());
          Serial.println(); 
          exec=0;
        }
        
      }
      //update the fields of the values
      //when the moirture percent is bigger than the field max, it means that we entered to the next field
      max_too_wet = ((tmp_too_wet_max + 5)<100)?tmp_too_wet_max:100; // check this command
      max_wet = (tmp_too_wet_min + tmp_wet_max)/2;
      max_meduim=(tmp_wet_min+tmp_meduim_max)/2;
      max_dry= (tmp_meduim_min+tmp_dry_max)/2;
      min_dry=(tmp_dry_min!=0)?(tmp_dry_min)/2 : 0;
      
     

      if(Firebase.getInt(fbdo, "/Robophone/5669122872442880/OptimalConditions/Recommended_soil_moist")){
        perfect_plant_water_level=fbdo.to<int>();
      }else{
        Serial.printf(fbdo.errorReason().c_str());
        Serial.println(); 
      }
    }
    prev_exec=1;
    moisture_states = getState(soil_moisture_percent, max_wet, max_meduim, max_dry, min_dry);
    Firebase.setInt(fbdo, "/Robophone/5669122872442880/moisture_states",moisture_states);
    
    // if the user started irrigation manualy
    if(Firebase.getInt(fbdo, "/Robophone/5669122872442880/start_irrigation")){
      manual_start_irrigation=fbdo.to<int>();
    }else{
      Serial.printf(fbdo.errorReason().c_str());
      Serial.println(); 
    }
    if(manual_start_irrigation == 1){ // it means that the plant need more water
      digitalWrite(2, HIGH); //means start irrigation
    }else{
      if(moisture_states <= perfect_plant_water_level)
        digitalWrite(2, LOW); //means stop irrigation, or we got to the wanted state, or we have more water so we need to wait it untile dry
    } 

    //do or stop irrigation according to perfect state
    if(moisture_states > perfect_plant_water_level){ // it means that the plant need more water
      digitalWrite(2, HIGH); //means start irrigation
    }else{
      if(manual_start_irrigation != 1){
        digitalWrite(2, LOW); //means stop irrigation, or we got to the wanted state, or we have more water so we need to wait it untile dry
      } 
    }   
  }
    
    Serial.printf("Get tempreture--  %s\n", Firebase.getInt(fbdo, "/Robophone/5669122872442880/tempreture") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    Serial.printf("Get humidity--  %s\n", Firebase.getInt(fbdo, "/Robophone/5669122872442880/humidity") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    Serial.printf("Get soil moisture--  %s\n", Firebase.getInt(fbdo, "/Robophone/5669122872442880/soil_moisture") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    Serial.printf("Get soil moisture state--  %s\n", Firebase.getInt(fbdo, "/Robophone/5669122872442880/moisture_states") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    Serial.println();  



    Serial.print("tempreture:");
    Serial.print(hdt_tempreture);
    Serial.print("  humidity: ");
    Serial.print(hdt_humidity);
    Serial.print("  soil moisture: ");
    Serial.print(soil_moisture);
    Serial.print("  soil moisture percent: ");
    Serial.print(soil_moisture_percent);
    Serial.println("%");
    Serial.println("------------------");
    Serial.println();
    
  
  }
  delay(300);
}
