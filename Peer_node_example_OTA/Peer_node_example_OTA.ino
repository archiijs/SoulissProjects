/**************************************************************************
    Souliss - Hello World for Expressif ESP8266
    
    This is the basic example, create a software push-button on Android
    using SoulissApp (get it from Play Store).  
    
    Load this code on ESP8266 board using the porting of the Arduino core
    for this platform.

***************************************************************************/

// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"

// Configure the framework
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "Paltes"
#define WiFi_Password           "virus.exe"    

// Include framework code and libraries
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

/*** All configuration includes should be above this line ***/ 
#include "Souliss.h"
#include "DHT.h"                      //Include and Configure DHT21 SENSOR

// This identify the number of the LED logic
#define MYLEDLOGIC              0
#define MYPIRLOGIC              1      
#define TEMPERATURE             2
#define HUMIDITY                4

#define DEADBAND                0.01       // Deadband value 1%
         
// **** Define here the right pin for your ESP module **** 
#define	RELAYOUT			          D0    
#define BUTONIN                 D1    
#define DHTIN                   D2    //DHT21
#define PIRIN                   D3    //PIRIN

#define DHTTYPE DHT21                 //DHT21 
DHT dht(DHTIN, DHTTYPE, 15);

// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {192, 168, 1, 42};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 1, 1};

void setup()
{   
    Initialize();

    // Connect to the WiFi network and get an address from DHCP
    Souliss_SetIPAddress(ip_address, subnet_mask, ip_gateway);                          

    // This is the vNet address for this node, used to communicate with other
	// nodes in your Souliss network
    SetAddress(0xAB03, 0xFF00, 0xAB00);
    
    Set_SimpleLight(MYLEDLOGIC);               // RELAY
    Set_SimpleLight(MYPIRLOGIC);               // PIR
    Set_Temperature(TEMPERATURE);
    Set_Humidity(HUMIDITY);
    
    pinMode(RELAYOUT, OUTPUT);                 // Relay out
    pinMode(BUTONIN, INPUT);                   // Hardware pulldown required
    pinMode(DHTIN, INPUT);
    pinMode(PIRIN, INPUT);
    
    dht.begin();

    // Init the OTA
    ArduinoOTA.setHostname("Paltes");    
    ArduinoOTA.begin();
    
}   

void loop()
{  
    // Here we start to play
    EXECUTEFAST() 
    {                     
        UPDATEFAST();   
        
        FAST_50ms() 
        {   // We process the logic and relevant input and output every 50 milliseconds
            DigIn2State(BUTONIN, Souliss_T1n_ToggleCmd, Souliss_T1n_ToggleCmd, MYLEDLOGIC);       // Use the pin2 as ON/OFF toggle command
            Logic_SimpleLight(MYLEDLOGIC);                                                        // Drive the LED as per command
            DigOut(RELAYOUT, Souliss_T1n_Coil, MYLEDLOGIC);                                       // Use the pin9 to give power to the LED according to the logic

            DigIn(PIRIN, Souliss_T1n_ToggleCmd, MYPIRLOGIC);                                      //LED logic for PIR
            Logic_SimpleLight(MYPIRLOGIC);
        
        }

        FAST_2110ms()
        {
           Logic_Temperature(TEMPERATURE);
           Logic_Humidity(HUMIDITY);
        } 
              
        // Here we handle here the communication with Android
        FAST_PeerComms();                                        
    }

    EXECUTESLOW() 
    {
        UPDATESLOW();

        SLOW_10s() 
        {  // Read temperature and humidity from DHT every 10 seconds  
           float h = dht.readHumidity();
           // Read temperature as Celsius
           float t = dht.readTemperature();
              
           // Check if any reads failed and exit early (to try again).
           if (isnan(h) || isnan(t)) 
             {
             //return;
             }
              
           Souliss_ImportAnalog(memory_map, TEMPERATURE, &t);
           Souliss_ImportAnalog(memory_map, HUMIDITY, &h); 
        } 
    } 
     
    // Look for a new sketch to update over the air
    ArduinoOTA.handle();
} 
