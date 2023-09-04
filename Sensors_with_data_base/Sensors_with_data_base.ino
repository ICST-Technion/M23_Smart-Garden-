//****************************************************************************************************************************************************************//
//########################################### created By: Omar_asadi, Haia_Hihi, Mohamad-alwan_aghbaria ##########################################################//
//############################################################   Date: 24/08/2023    #############################################################################//
//## in this file we are trying to read from the DHT 11 sensor the temprture and the humdity of the enviroment and also the Capactive soil moisture using ESP32 ##//
//#################### In addtion in this file we are connected to the firebase -real time database- and iploading the data to the data base #####################//
//****************************************************************************************************************************************************************//

#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>


#endif

#include <addons/TokenHelper.h>
#include <DFRobot_DHT11.h>
#include <addons/RTDBHelper.h>
const char* WIFI_SSID = "ICST";
const char* WIFI_PASSWORD = "arduino123";


/** Define the API Key **/
#define API_KEY "q5GRF4P379NZpHPnG2KmczcFzm43yw8NDEPpVmHg"

/** Define our RTDB URL **/
#define DATABASE_URL "https://temperature-sensor-test-46ad3-default-rtdb.firebaseio.com/"

/** Define Firebase Data object **/
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

int a, b, x, y;
int hdt_tempreture, hdt_humidity;
const int DHT11_PIN= 32;

DFRobot_DHT11 DHT;


void setup()
{
  Serial.begin(115200);
  delay(2000);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("The Wi-Fi status is "); 
    Serial.println(WiFi.status());
    delay(600);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(DATABASE_URL, API_KEY);
  Firebase.setDoubleDigits(5);
}
void loop()
{
  
   DHT.read(DHT11_PIN);
    
  if (Firebase.ready()) 
  {
    hdt_tempreture = DHT.temperature;
    hdt_humidity = DHT.humidity;
    Firebase.setInt(fbdo, "/test/tempreture",hdt_tempreture);
    Firebase.setInt(fbdo, "/test/humidity",hdt_humidity);
    delay(200);
    Serial.printf("Get tempreture--  %s\n", Firebase.getInt(fbdo, "/test/tempreture") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
   
    Serial.printf("Get humidity--  %s\n", Firebase.getInt(fbdo, "/test/humidity") ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());
   
	Serial.println();  
	Serial.print("tempreture:");
	Serial.print(hdt_tempreture);
	Serial.print("  humidity: ");
	Serial.print(hdt_humidity);
	
	Serial.println();
	Serial.println("------------------");
	Serial.println();
	
	delay(1000);
  }
}
