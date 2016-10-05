/**************************************************************************
    Souliss - Hello World for Expressif ESP8266      
***************************************************************************/
// Let the IDE point to the Souliss framework
#include                       "SoulissFramework.h"

// Configure the framework
#include                       "bconf/MCU_ESP8266.h"            // Load the code directly on the ESP8266
#include                       "conf/Gateway.h"                 // The main node is the Gateway, we have just one node
#include                       "conf/IPBroadcast.h"

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID              "Paltes"
#define WiFi_Password          "virus.exe"    

// Include framework code and libraries
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

/*** All configuration includes should be above this line ***/ 
#include "Souliss.h"
#include "DHT.h"                                                // Include and Configure DHT21 SENSOR


// This identify the number of the LED logic
#define MYLEDLOGIC          0
#define TEMPERATURE         1
#define HUMIDITY            3             

#define DEADBAND            0.01                               // Deadband value 1%

// **** Ieejas un izejas pini ****
#define	BUTONIN 			      4                                  //sleedzis
#define RELAYOUT            5                                  //relejs
#define DHTIN               13                                 //DHT21 pins

#define DHTTYPE DHT21                                          // DHT21 
DHT dht(DHTIN, DHTTYPE, 15);                                   //DHT setup for esp

// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {192, 168, 1, 40};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 1, 1};
    
void setup()
{   
    Initialize();

    // Set network IP parameters
    Souliss_SetIPAddress(ip_address, subnet_mask, ip_gateway);
    SetAsGateway(40);  // Use the last byte of the IP address if using a static IP address
    
    // This is the vNet address for this node, used to communicate with other
	  // nodes in your Souliss network
    SetAddress(0xAB01, 0xFF00, 0x0000);
    SetAsPeerNode(0xAB02, 1);                                 //Koridors
    SetAsPeerNode(0xAB03, 2);                                 //Ars
    SetAsPeerNode(0xAB04, 3);                                 //TST
      
    Set_SimpleLight(MYLEDLOGIC);                              // Define a simple LED light logic
    Set_Temperature(TEMPERATURE);
    Set_Humidity(HUMIDITY);
	  
	  pinMode(BUTONIN, INPUT);
    pinMode(DHTIN, INPUT);
    pinMode(RELAYOUT, OUTPUT);                                // Use pin as output 

    dht.begin();

    // Init the OTA
    ArduinoOTA.setHostname("Gulamistaba (Gateway)");    
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
      	    DigIn2State(BUTONIN, Souliss_T1n_ToggleCmd, Souliss_T1n_ToggleCmd, MYLEDLOGIC);
            Logic_SimpleLight(MYLEDLOGIC);
            DigOut(RELAYOUT, Souliss_T1n_Coil,MYLEDLOGIC);
        }

        FAST_2110ms()
        {
           Logic_Temperature(TEMPERATURE);
           Logic_Humidity(HUMIDITY);
        } 
              
        // Here we handle here the communication with Android
        FAST_GatewayComms();                                        
    }
    
    EXECUTESLOW() 
    {
       UPDATESLOW();
  
       SLOW_10s() 
       {  
          // Read temperature and humidity from DHT every 10 seconds  
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
