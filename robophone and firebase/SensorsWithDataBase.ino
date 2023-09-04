
//###########################################################################################################//
//############## Created by: Asadi_Omar, Hihi_Haya, Alwan_aghbarieh_Mohamad. Date:03/09/23  #################//
// Descriptin: in the following file we are reading both tempreture and humadity sensor and capactive sensor// 
//#### after reading the sensors we are uploading the data to our real-time data base the we've created ####// 
//#########################################################################################################//

#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

#endif

//Provide the token generation process info.
#include <addons/TokenHelper.h>
#include <DFRobot_DHT11.h>
//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
/* 1. Define the WiFi credentials */
const char* WIFI_SSID = "Haya Hihi";
const char* WIFI_PASSWORD = "haya1240";
//For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino
/* 2. Define the API Key */
#define API_KEY "q5GRF4P379NZpHPnG2KmczcFzm43yw8NDEPpVmHg"
/* 3. Define the RTDB URL */
#define DATABASE_URL "https://temperature-sensor-test-46ad3-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

int a, b, x, y;
int hdt_tempreture, hdt_humidity;
int soil_moisture;
const int SOIL_MOISTURE_PIN= 32;
const int DHT11_PIN= 33;


DFRobot_DHT11 DHT;


void setup()
{
  Serial.begin(115200);
  delay(1000);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    //Serial.print("The Wi-Fi status is "); 
    //Serial.println(WiFi.status());
    delay(600);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
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
void loop()
{
  //DFRobot_DHT11 DHT;
  
   DHT.read(DHT11_PIN);
   hdt_tempreture = DHT.temperature;
   hdt_humidity = DHT.humidity;
   soil_moisture = analogRead(SOIL_MOISTURE_PIN);
  if (Firebase.ready()) 
  {
    Firebase.setInt(fbdo, "/Robophone/5669122872442880/tempreture",hdt_tempreture);
    Firebase.setInt(fbdo, "/Robophone/5669122872442880/humidity",hdt_humidity);
    Firebase.setInt(fbdo, "/Robophone/5669122872442880/soil_moisture",soil_moisture);
    delay(200);
    Serial.printf("Get tempreture--  %s\n", Firebase.getInt(fbdo, "/Robophone/5669122872442880/tempreture") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    // a=fbdo.to<int>();
    Serial.printf("Get humidity--  %s\n", Firebase.getInt(fbdo, "/Robophone/5669122872442880/humidity") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
    // b=fbdo.to<int>();
    Serial.printf("Get soil moisture--  %s\n", Firebase.getInt(fbdo, "/Robophone/5669122872442880/soil_moisture") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
  
    Serial.println();  
    if(soil_moisture>3500){
      digitalWrite(2, HIGH);  // turn the LED on (HIGH is the voltage level)
      Serial.println("Oops! it's too dry");
    }else{
      digitalWrite(2, LOW);
    }
    Serial.print("tempreture:");
    Serial.print(hdt_tempreture);
    Serial.print("  humidity: ");
    Serial.print(hdt_humidity);
    Serial.print("  soil moisture: ");
    Serial.print(soil_moisture);
    
    Serial.println();
    Serial.println("------------------");
    Serial.println();
    
    delay(1000);
  }
}
