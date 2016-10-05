/**************************************************************************
    Souliss - Espressif ESP8266-Esp12
   
    This is the Souliss Gateway, it has DHT11 for Temp & Hum, and PIR sensor
    for Main Antitheft System, add a blink led for a sign if the Peer Node is 
    connected to wifi network.
    
***************************************************************************/

// Let the IDE point to the Souliss framework
#include "SoulissFramework.h"

// Configure the framework
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "bconf/MCU_ESP8266.h"              // Load the code directly on the ESP8266
#include "conf/IPBroadcast.h"
#include "Souliss.h"
#include <DHT.h>

boolean start;

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
DHT dht(DHTPIN, DHT11, 15);

enum {
  APPLICATION_WEBSERVER = 0,
  ACCESS_POINT_WEBSERVER
};

MDNSResponder mdns;
ESP8266WebServer server(80);
const char* ssid = "Souliss";  // Use this as the ssid as well
                                // as the mDNS name
const char* passphrase = "souliss8";
String st;
String content;

void setup(){  
    Serial.begin(115200);
    start = setup_esp();
    if(start) Serial.println("TRUE"); else Serial.println("FALSE");
  
    if(start){     
    WiFi.mode(WIFI_STA);
        Initialize();

        // This board request an address to the gateway at runtime, no need
        // to configure any parameter here.
        GetIPAddress();
        SetAddress(0xAB03, 0xFF00, 0x0000); //Antitheft & Temp/Hum esp12                         
  
        
        // Set the typical to use
        Souliss_SetT52(memory_map, TEMPERATURE);
        Souliss_SetT53(memory_map, HUMIDITY);
 
        // Setup the anti-theft
        Set_T41(ANTITHEFT);
        
        } else {
        setupAccessPoint();  // No WiFi yet, enter configuration mode
        }
        pinMode(2, INPUT);                 // Use pin GPI02 as digital input for PIR  
        pinMode(BLINK, OUTPUT);            // use pin 5 as Blink Led
        dht.begin();
}
 
void loop(){

            if (!start) {
            server.handleClient();  // In this example we're not doing too much
            }  
            else 
            { 
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
                FAST_2110ms()   { 
             
              
                
                // Build a watchdog chain to monitor the nodes
                mInput(ANTITHEFT) = Watchdog(0xAB03, WATCHDOG, Souliss_T4n_Alarm);                
                }
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
}

//Function for web interface SSID
bool setup_esp() {
  
  WiFi.mode(WIFI_STA);  // Assume we've already been configured
  //  Serial.setDebugOutput(true);
  //  WiFi.printDiag(Serial);
  if (testWifi()) {
    setupApplication();  // WiFi established, setup application
    return 1;
  } else {
    setupAccessPoint(); // No WiFi yet, enter configuration mode
    return 0;
  }

}

bool testWifi(void) {
  int c = 0;
  Serial.println("\nWaiting for Wifi to connect...");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
    Serial.print(WiFi.status());
    c++;
  }
  Serial.println("\nConnect timed out, opening AP");
  return false;
}

void setupApplication() {
  if (mdns.begin(ssid, WiFi.localIP())) {
    Serial.println("\nMDNS responder started");
  }
  launchWeb(APPLICATION_WEBSERVER); // In this example just launch a
  // web server
}

void setupAccessPoint(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, passphrase, 6);
  launchWeb(ACCESS_POINT_WEBSERVER);
}

void launchWeb(int webservertype) {
  Serial.println("\nWiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  setupWebServerHandlers(webservertype);
  // Start the server
  server.begin();
  Serial.print("Server type ");
  Serial.print(webservertype);
  Serial.println(" started");
  //  WiFi.printDiag(Serial);
}

void setupWebServerHandlers(int webservertype)
{
  if ( webservertype == ACCESS_POINT_WEBSERVER ) {
    server.on("/", handleDisplayAccessPoints);
    server.on("/setap", handleSetAccessPoint);
    server.onNotFound(handleNotFound);
  } else if (webservertype == APPLICATION_WEBSERVER) {
    server.on("/", handleRoot);
    server.on("/setap", handleAccessPointAlreadySet);
    server.onNotFound(handleNotFound);
  }
}

void handleDisplayAccessPoints() {
  IPAddress ip = WiFi.softAPIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String macStr = macToStr(mac);
  content = "<!DOCTYPE HTML>\n<html>Hello from ";
  content += ssid;
  content += " at ";
  content += ipStr;
  content += " (";
  content += macStr;
  content += ")";
  content += "<p>";
  content += st;
  content += "<p><form method='get' action='setap'><label>SSID: </label>";
  content += "<input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
  content += "<p>We will attempt to connect to the selected AP and reset if successful.";
  content += "<p>Wait a bit and try connecting to http://";
  content += ssid;
  content += ".local";
  content += "</html>";
  server.send(200, "text/html", content);
}

void handleSetAccessPoint() {
  int httpstatus = 200;
  String qsid = server.arg("ssid");
  String qpass = server.arg("pass");
  if (qsid.length() > 0 && qpass.length() > 0) {
    for (int i = 0; i < qsid.length(); i++)
    {
      // Deal with (potentially) plus-encoded ssid
      qsid[i] = (qsid[i] == '+' ? ' ' : qsid[i]);
    }
    for (int i = 0; i < qpass.length(); i++)
    {
      // Deal with (potentially) plus-encoded password
      qpass[i] = (qpass[i] == '+' ? ' ' : qpass[i]);
    }
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(qsid.c_str(), qpass.c_str());
    if (testWifi()) {
      Serial.println("\nGreat Success!");
      delay(3000);
      abort();
    }
    content = "<!DOCTYPE HTML>\n<html>";
    content += "Failed to connect to AP ";
    content += qsid;
    content += ", try again.</html>";
  } else {
    content = "<!DOCTYPE HTML><html>";
    content += "Error, no ssid or password set?</html>";
    Serial.println("Sending 404");
    httpstatus = 404;
  }
  server.send(httpstatus, "text/html", content);
}

void handleRoot() {
  IPAddress ip = WiFi.localIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String macStr = macToStr(mac);
  content = "<!DOCTYPE HTML>\n<html>Hello from ";
  content += ssid;
  content += " at ";
  content += ipStr;
  content += " (";
  content += macStr;
  content += ")";
  content += "</html>";
  server.send(200, "text/html", content);
}

void handleAccessPointAlreadySet() {
  content = "<!DOCTYPE HTML>\n<html>";
  content += "You already set up the access point and it is working if you got this far.";
  content += "</html>";
  server.send(200, "text/html", content);
}

void handleNotFound() {
  content = "File Not Found\n\n";
  content += "URI: ";
  content += server.uri();
  content += "\nMethod: ";
  content += (server.method() == HTTP_GET) ? "GET" : "POST";
  content += "\nArguments: ";
  content += server.args();
  content += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    content += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", content);
}
String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}
