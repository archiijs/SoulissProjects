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

/*** All configuration includes should be above this line ***/ 
#include "Souliss.h"

// This identify the number of the LED logic
#define MYLEDLOGIC          0      
         
// **** Define here the right pin for your ESP module **** 
#define	OUTPUTPIN			          5
#define INPUTPIN                2

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
    
    Set_SimpleLight(MYLEDLOGIC);        // Define a simple LED light logic
    
    pinMode(INPUTPIN, INPUT);                  // Hardware pulldown required
    pinMode(OUTPUTPIN, OUTPUT);                 // Power the LED
}

void loop()
{ 
    // Here we start to play
    EXECUTEFAST() {                     
        UPDATEFAST();   
        
        FAST_50ms() 
        {   // We process the logic and relevant input and output every 50 milliseconds
           
            DigIn(INPUTPIN, Souliss_T1n_ToggleCmd, MYLEDLOGIC);            // Use the pin2 as ON/OFF toggle command
            Logic_SimpleLight(MYLEDLOGIC);                          // Drive the LED as per command
            DigOut(OUTPUTPIN, Souliss_T1n_Coil, MYLEDLOGIC);                // Use the pin9 to give power to the LED according to the logic
        } 
              
        // Here we handle here the communication with Android
        FAST_PeerComms();                                        
    }
} 
