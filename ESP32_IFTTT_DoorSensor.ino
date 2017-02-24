/* 
  ESP32_IFTTT_DoorSensor (Arduino)
  Ben Rockwood <benr@cuddletech.com>
  
  This is a modified, working example of a simple IFTTT Maker project
  based on https://github.com/awende/ESP32_Water_Sensor

  Circuit has 2 LEDs with 220Ohm Resistors on Pins 16 and 17 as status lights.
  A MC-38 Door Sensor is on Pin 5 (one lead to pin, other to GND, doesn't matter which) 
*/


#include <WiFi.h>
WiFiClient client;

const char* myKey     = "xxxxxxxxxxxxxxxxxxxxxx";      // IFTTT maker key
const char* myTrigger = "OfficeDoor";                  // IFTTT Applet Trigger
const char* ssid      = "HOME-701D-2.4";               // WIFI SSID
const char* password  = "xxxxxxxxxxxxxxxx";            // WIFI Password 

// GPIO Pins
const byte RED_LED = 16;
const byte GRN_LED = 17;
const byte SENSOR = 5;

// State
String oldState = "unset";

void setup() {
  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(GRN_LED, OUTPUT);
  pinMode(SENSOR, INPUT_PULLUP);
  
  WiFi.mode(WIFI_STA);
  ConnectWifi();
}

void loop(){  
  if(WiFi.status() == WL_CONNECTED)
  {
    String doorState;

    // Read Sensor State
    if (digitalRead(SENSOR) == HIGH){
      //Serial.println("Door is open.");
      openSignal(true);
      doorState = "Open";
    } else { 
      //Serial.println("Door is closed.");
      openSignal(false);
      doorState = "Closed";
    }  

    // Initialize on first sensing
    if (oldState == "unset"){ 
      oldState = doorState;
    }

    // Evaluate for State Change & Report to IFTTT
    if ( doorState != oldState ) { 
      Serial.print("Door is ");
      Serial.println(doorState);
      
      Serial.println("connecting...");
      if(client.connect("maker.ifttt.com",80)){
        Serial.println("Connected to Maker");
        sendData(doorState);
        oldState = doorState;
      } else {
        Serial.println("Failed to connect to Maker.");
      }
    }
  }
  else  // Reconnect to wireless access point
  {
    delay(1000);    // Wait 1 second between connection attempts
    ConnectWifi();  // Connect to WiFi
  }
  delay(1000);
}

bool ConnectWifi(){
  bool timeOut = 0; // Change to 1 if connection times out
  byte count = 0;   // Counter for the number of attempts to connect to wireless AP
  
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password); // Connect to WiFi network

  while (WiFi.status() != WL_CONNECTED && (timeOut == 0)) // Test to see if we're connected
  {
    Serial.print('.');
    count++;
    
    if(count > 60) break; // Give up after ~30 seconds
    else delay(500);      // Check again after 500ms
  }
  
  if (WiFi.status() == WL_CONNECTED)  // We're connected
  {
    Serial.println("\r\nWiFi connected");
  }
  else  // Unable to connect
  {
    WiFi.disconnect();
    Serial.println("\r\nConnection Timed Out!\r\n");
  }
}


void sendData(String data){
  // Packet will end up looking similar to this:
  // 
  //  POST /trigger/ESP/with/key/(myKey) HTTP/1.1
  //  Host: maker.ifttt.com
  //  User-Agent: ESP32
  //  Connection: close
  //  Conent-Type: application/json
  //  Conent-Length: (postData length)
  //  
  //  {"value1":"(HIGH/LOW )"}
  
  String postData = "{\"value1\":\""; 
         postData.concat(data);
         postData.concat("\"}");
    
  client.print("POST /trigger/");
  client.print(myTrigger);
  client.print("/with/key/");
  client.print(myKey);
  client.println(" HTTP/1.1");

  client.println("Host: maker.ifttt.com");
  client.println("User-Agent: ESP32");
  client.println("Connection: close");

  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(postData.length());
  client.println();
  client.println(postData);
}

void openSignal(bool state){
  if (state){
    digitalWrite(RED_LED, LOW);
    digitalWrite(GRN_LED, HIGH);
  } else {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GRN_LED, LOW);
  }
}
