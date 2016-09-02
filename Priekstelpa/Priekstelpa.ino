#include <WiFiClient.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>

WiFiManager wifi(0);
/**************************************************************************
    Souliss - Espressif ESP8266-Esp12
   
    This is the Souliss Gateway, it has DHT11 for Temp & Hum, and PIR sensor
    for Main Antitheft System, add a blink led for a sign if the Peer Node is 
    connected to wifi network.
    
***************************************************************************/
// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID               "mywifi"
#define WiFi_Password           "mypassword"  
 
// Configure the framework
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/DynamicAddressing.h"         // Use dynamic address
#include "conf/IPBroadcast.h"
#include "Souliss.h"
#include <DHT.h>

//PIN DHT
#define DHTPIN    13                           // Used GPI4 as digital input from dht11

//PIN for Blink as sign if the node is running
#define BLINK     5

// This identify the Slot number of typicals 
#define TEMPERATURE             0          // This is the memory slot used for DHT11 Temp
#define HUMIDITY                2          // This is the memory slot used for DHT11 Humidity
#define DEADBAND                0.01       // Deadband value 1%  
#define ANTITHEFT               4          // This is the memory slot used for the execution of the anti-theft
#define WATCHDOG                5

// Setup the DHT sensor
DHT dht(DHTPIN, DHT21, 15);

// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {192, 168, 1, 41};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 1, 1};


void setup(){  
    Serial.begin(115200);
    wifi.autoConnect("Souliss");  
    WiFi.mode(WIFI_STA);
        Initialize();

        // This board request an address to the gateway at runtime, no need
        // to configure any parameter here.
        // Set network IP parameters
        Souliss_SetIPAddress(ip_address, subnet_mask, ip_gateway);
        SetAsGateway(41);  // Use the last byte of the IP address if using a static IP address

        SetDynamicAddressing();
        GetAddress();                       
  
        
        // Set the typical to use
        Souliss_SetT52(memory_map, TEMPERATURE);
        Souliss_SetT53(memory_map, HUMIDITY);
 
        // Setup the anti-theft
        Set_T41(ANTITHEFT);
        
        
        pinMode(2, INPUT);                 // Use pin GPI02 as digital input for PIR  
        pinMode(BLINK, OUTPUT);            // use pin 5 as Blink Led
        dht.begin();
}
 
void loop(){

           
            // Here we start to play
            EXECUTEFAST() {                     
                UPDATEFAST(); 
                
                FAST_50ms(){
                if (WiFi.status() == WL_CONNECTED) { 
                digitalWrite(BLINK, !digitalRead(BLINK));
               
              }
                }
                
                FAST_510ms(){
                   // Retreive data from the MaCaco communication channel
                Souliss_CommunicationData(memory_map, &data_changed);
               
                // Compare the acquired input with the stored one, send the new value to the
                // user interface if the difference is greater than the deadband
       
                Souliss_Logic_T52(memory_map, TEMPERATURE, DEADBAND, &data_changed);
                Souliss_Logic_T53(memory_map, HUMIDITY, DEADBAND, &data_changed);
                
                // Input from anti-theft sensor
                LowDigIn(2, Souliss_T4n_Alarm, ANTITHEFT);
                   
                // Execute the anti-theft logic
                Logic_T41(ANTITHEFT);   
                   
                }
                      
                // Here we handle here the communication with Android
                FAST_PeerComms(); 
                // Execute the code every 2110ms          
 //               FAST_2110ms()   { 
             
              
                
                // Build a watchdog chain to monitor the nodes
 //               mInput(ANTITHEFT) = Watchdog(0xAB03, WATCHDOG, Souliss_T4n_Alarm);                
 //               }
         }
         
         EXECUTESLOW() {
         UPDATESLOW();
                       
         SLOW_10s() {  
             // Read temperature value from DHT sensor and convert from single-precision to half-precision
                    float temperature = dht.readTemperature();
                    
                    Souliss_ImportAnalog(memory_map, TEMPERATURE, &temperature);
                    Serial.print ("Temp  ");
                    Serial.print (dht.readTemperature());                    

                    // Read humidity value from DHT sensor and convert from single-precision to half-precision
                    float humidity = dht.readHumidity();
                    
                    Souliss_ImportAnalog(memory_map, HUMIDITY, &humidity);
                    Serial.print (" & Hum  ");    
                    Serial.println (dht.readHumidity());
                 
              
          } 
      // Here we periodically check for a gateway to join
        SLOW_PeerJoin();
      }
      
       START_PeerJoin(); //tell gateway that i am exist    
    }          
