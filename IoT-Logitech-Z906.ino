/**
 * Internet Connected Logitech Z906 API.
 * Based on the works of Zarpli - Alejandro Zárate (https://github.com/zarpli/LOGItech-Z906/).
 */
#include <Arduino.h>
#include "./environment.h"
#include "./Z906.h"
#include "./endpoints.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

int timer = 0;
ESP8266WebServer server(80);
ESP8266WiFiMulti WiFiMulti;

// Instantiate a Z906 object and Attach to Serial1
Z906 LOGI(Serial1);

/**
 * Setup the serial communication with the Z906.
 */
void initSerial() {
    Serial.begin(9600);
    while(!Serial);
}

/**
 * Setup and connect to a WiFi network.
 */
void initWiFi() {
    // Setup station mode.
    WiFi.mode(WIFI_STA);
  
    // Stored WiFi credentials.
    for (Network network : network_credentials) WiFiMulti.addAP(network.ssid, network.password);
    delay(2000);
  
    // Connect to the WiFi.
    connectToWiFi("Connecting");
}

/**
 * Connect to the WiFi.
 */
void connectToWiFi(const String& message) {
    timer = 1;
    Serial.printf("\n[SETUP] %s (%d second)...\n", message.c_str(), timer);
    
    while (WiFiMulti.run() != WL_CONNECTED) {
      timer++;
      Serial.printf("[SETUP] %s (%d seconds)...\n", message.c_str(), timer);
      delay(1000);
      Serial.flush();
    }
    
    onConnected();
}

/**
 * When connection is restored, log the details.
 */
void onConnected() {
    timer = 0;
    Serial.flush();
    Serial.print("[SETUP] Connected to ");
    Serial.print(WiFi.SSID());
    Serial.print(" (");
    Serial.print(WiFi.localIP());
    Serial.println(").");
    
    // Set the hostname,
    WiFi.hostname("LOGITECH-Z906");
    
    // Configure MDNS.
    if (MDNS.begin("logitech-z906")) {
        Serial.println("[SETUP] MDNS responder started.");
    }
  
    Serial.println();
}

/**
 * Setup the web server.
 */
void initWebServer() {
    server.onNotFound(handleRequest);
    server.begin();
}

/**
 * Handle a HTTP request.
 */
void handleRequest() {
    String endpoint = server.uri();;
      
    if (endpoint == "/") return server.send(200, "text/plain", "This is the Logitech Z906 API.");
    if (endpoint.length() > 1) endpoint = endpoint.substring(1);
  
    // Check if the Z906 is connected before processing any commands.
    if (LOGI.request(VERSION) == 0) return server.send(200, "application/json", "{\"status\": \"disconnected\"}");
  
    // Handle defined endpoints.
    for (Endpoint e : endpoints) {
        if (e.path == endpoint) return respondToRequest(e);
    }
  
    // If not found, redirect to home.
    server.sendHeader("Location", String("/"), true);
    server.send(302, "text/plain", "");
}

/**
 * Respond to a HTTP request for the given endpoint.
 */
void respondToRequest(Endpoint endpoint) {
    if (endpoint.type == SelectInput) {
        LOGI.input(endpoint.action);
        return server.send(200, "application/json", "{\"status\": \"connected\", \"success\": true }");
        
    } else if (endpoint.type == RunCommand) {
        LOGI.cmd(endpoint.action);
        return server.send(200, "application/json", "{\"status\": \"connected\", \"success\": true }");
        
    } else if (endpoint.type == SetValue) {
        // TODO: Get and validate the inputted value - fail if invaild.
        LOGI.cmd(endpoint.action, 15);
        return server.send(501, "application/json", "{\"status\": \"connected\", \"success\": false, \"message\": \"Under construction.\" }");
        
    } else if (endpoint.type == GetValue) {
        return server.send(200, "application/json", "{\"status\": \"connected\", \"success\": true, \"value\": "+String(LOGI.request(endpoint.action))+" }");

    } else if (endpoint.type == RunFunction) {
        if (endpoint.path == "temperature") return handleGetTemperature();
        if (endpoint.path == "power/off") return handlePowerOff();
    }

    return server.send(405, "application/json", "{\"status\": \"connected\", \"success\": false, \"message\": \"Your action was recognised, but it is not supported.\" }");
}

/**
 * Handle the getTemperature function.
 */
void handleGetTemperature() {
    uint8_t value = LOGI.main_sensor();
    if (value > 0) return server.send(200, "application/json", "{\"status\": \"connected\", \"success\": true, \"value\": "+String(value)+" }");
    return server.send(200, "application/json", "{\"status\": \"connected\", \"success\": false }");
}

/**
 * Gracefully turn the power off, saving the last used state.
 */
void handlePowerOff() {
    LOGI.off();
    return server.send(200, "application/json", "{\"status\": \"connected\", \"success\": true }");
}

/**
 * Setup
 */
void setup() {
  initSerial();
  initWiFi();
  initWebServer();
}

/**
 * Loop
 */
void loop() {
  if (WiFi.status() != WL_CONNECTED) connectToWiFi("Reconnecting");
  server.handleClient();
}
