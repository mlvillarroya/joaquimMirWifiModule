/*
  Complete project details: https://RandomNerdTutorials.com/esp8266-nodemcu-https-requests/ 
  Based on the BasicHTTPSClient.ino Created on: 20.08.2018 (ESP8266 examples)
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
//#include <WiFi.h>

typedef enum {
    STATUS_READING_INSTRUCTIONS   = 0,
    STATUS_READY_TO_CONNECT       = 1,
    STATUS_CONNECTING             = 2,
    STATUS_CONNECTED              = 3,
    STATUS_READY_TO_SEND_REQUEST  = 4
} system_status;

String ssid = "";
String password = "";
String url = "";
const String SSID = "SSID";
const String PASSWORD = "PASSWORD";
const String RESET = "RESET";
const String GET = "GET";
const String HELP = "HELP";
const String ACK = "ACK";
const String NOACK = "NOACK";
const int MAX_INTENTS = 3;
system_status generalStatus;

bool canSerialReceive() {
  return (generalStatus == STATUS_READING_INSTRUCTIONS || generalStatus == STATUS_CONNECTED || generalStatus == STATUS_READY_TO_SEND_REQUEST);
}

void sendACK() {
  Serial.println(ACK);
}

void sendNOACK() {
  Serial.println(NOACK);
}

void WiFiEvent(WiFiEvent_t event) {

    switch(event) {
        case WIFI_EVENT_STAMODE_GOT_IP:
            Serial.println("WiFi connected");
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());
            generalStatus = STATUS_CONNECTED;
            break;
        case WIFI_EVENT_STAMODE_DISCONNECTED:
            Serial.println("WiFi disconnected, please try introducing SSID and password again");
            ssid = "";
            password = "";
            url = "";
            generalStatus = STATUS_READING_INSTRUCTIONS;
            WiFi.disconnect();   
            break;
    }
}

void connectToWIFI() {
  Serial.println("Connection to Wifi starting...");
  //Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (generalStatus == STATUS_CONNECTING) {
    Serial.print(".");
    delay(1000);
  }
  if (generalStatus == STATUS_READING_INSTRUCTIONS) sendNOACK();
  if (generalStatus == STATUS_CONNECTED) sendACK();
}

void sendRequest() {
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    // Ignore SSL certificate validation
    client->setInsecure();
    
    //create an HTTPClient instance
    HTTPClient https;
    
    //Initializing an HTTPS communication using the secure client
    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, url)) {  // HTTPS
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
          sendACK();
        }
        else sendNOACK();
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        sendNOACK();
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
      sendNOACK();
    }
    url = "";
    generalStatus = STATUS_CONNECTED;
  }

void executeInstruction(String command, String parameter) {
  if (command == SSID) {
      ssid = parameter;
      Serial.println("SSID set to " + ssid);
    }
  else if (command == PASSWORD) {
      password = parameter;
      Serial.println("Password set to " + password);

    }
  else if (command == GET) {
      url = parameter;
      Serial.println("Sending GET request to " + url);
    }
  else if (command == RESET) {
      WiFi.disconnect();
      ssid = "";
      password = "";
      url = "";
      Serial.println("System resetted");
      sendACK();
    }
  else if (command == HELP) {
      Serial.println("COMMANDS ACCEPTED");
      Serial.println("SSID --- to insert the WiFi network's SSID");
      Serial.println("PASSWORD --- to insert the WiFi network's password");
      Serial.println("GET --- send a GET request to the specified URL");
      Serial.println("RESET --- to close connection and reset the WiFi information");
  }
  else {
      Serial.println("COMMAND NOT ACCEPTED");
      Serial.println("Send HELP to know the accepted commands");
  }
}

void checkSerialCreateInstruction() {
  while(Serial.available()) {
      String completeInstruction = Serial.readString();// read the incoming data as string
      completeInstruction.trim();
      String command = completeInstruction.substring(0,completeInstruction.indexOf(" "));
      Serial.println("command " + command);
      String parameter = "";
    if (completeInstruction.indexOf(" ") > 0) {
      	parameter = completeInstruction.substring(completeInstruction.indexOf(" ")+1);
        Serial.println("parameter " + parameter);
    }      
      executeInstruction(command, parameter);
  }
}

void setup() {
  Serial.begin(9600);
  //Serial.setDebugOutput(true);
  generalStatus = STATUS_READING_INSTRUCTIONS;
  WiFi.onEvent(WiFiEvent);
}

void loop() {
  if (canSerialReceive()) checkSerialCreateInstruction();

  switch (generalStatus) {
    case STATUS_READING_INSTRUCTIONS: {
      if ((ssid != "")&&(password != "")) generalStatus = STATUS_READY_TO_CONNECT;
      break;
    }
    case STATUS_READY_TO_CONNECT: {
      generalStatus = STATUS_CONNECTING;
      connectToWIFI();
      break;
    }
    case STATUS_CONNECTED: {
      if (url != "") generalStatus = STATUS_READY_TO_SEND_REQUEST;
      break;
    }
    case STATUS_READY_TO_SEND_REQUEST: {
      sendRequest();
      break;
    }
  }
}
