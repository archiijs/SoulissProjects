/**************************************************************************
    Souliss - Hello World for Expressif ESP8266
***************************************************************************/

// Let the IDE point to the Souliss framework
#include                        "SoulissFramework.h"

// Configure the framework
#include                        "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include                        "conf/IPBroadcast.h"

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               ""
#define WiFi_Password           ""    

// Include framework code and libraries
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

/*** All configuration includes should be above this line ***/ 
#include "Souliss.h"
#include "DHT.h"                                                   //Include and Configure DHT21 SENSOR

// This identify the number of the LED logic
#define MYLEDLOGIC              0
#define MYPIRLOGIC              1
#define MYDOORLOGIC             2      
#define TEMPERATURE             3
#define HUMIDITY                5

#define DEADBAND                0.01                               // Deadband value 1%
         
// **** Define here the right pin for your ESP module **** 
#define	RELAYOUT			          D0                                 //Output for light relay
#define BUTONIN                 D1                                 //Light switch
#define DHTIN                   D2                                 //DHT21
#define PIRIN                   D3                                 //Input for PIR sensor (Hi and Lo)
#define DOORIN                  D4                                 //Input for Door open sensor (Hi and Lo)

#define DHTTYPE DHT21                                              //DHT21 setup
DHT dht(DHTIN, DHTTYPE, 15);                                       //DHT setup for esp

// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {192, 168, 1, 43};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 1, 1};

void setup()
{   
    Initialize();

    // Connect to the WiFi network and get an address from DHCP
    Souliss_SetIPAddress(ip_address, subnet_mask, ip_gateway);                          

    // This is the vNet address for this node, used to communicate with other
	  // nodes in your Souliss network
    SetAddress(0xAB04, 0xFF00, 0xAB00);
    
    Set_SimpleLight(MYLEDLOGIC);                                 // RELAY
    Set_DigitalInput(MYPIRLOGIC);                                // PIR
    Set_DigitalInput(MYDOORLOGIC);                               // Door open/close (reed switch)
    Set_Temperature(TEMPERATURE);
    Set_Humidity(HUMIDITY);
    
    pinMode(RELAYOUT, OUTPUT);                                   // Relay out
    pinMode(BUTONIN, INPUT);                                     // Hardware pulldown required
    pinMode(DHTIN, INPUT);
    pinMode(PIRIN, INPUT);
    pinMode(DOORIN, INPUT);
    
    dht.begin();

    // Init the OTA
    ArduinoOTA.setHostname("TST");    
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

            DigIn2State(PIRIN, Souliss_T1n_OnCmd, Souliss_T1n_OffCmd, MYPIRLOGIC);                //PIR input (High on, Low off)
            Logic_DigitalInput(MYPIRLOGIC);

            DigIn2State(DOORIN, Souliss_T1n_OnCmd, Souliss_T1n_OffCmd, MYDOORLOGIC);              //DOOR state input (High on, Low off)
            Logic_DigitalInput(MYDOORLOGIC);
        
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
