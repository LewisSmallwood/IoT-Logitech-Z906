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

ESP8266WebServer server(80);
ESP8266WiFiMulti WiFiMulti;

// Instantiate a Z906 object and attach to Serial
Z906 LOGI(Serial);

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
    connectToWiFi();
}

/**
 * Connect to the WiFi.
 */
void connectToWiFi() {
    // Wait for connection.    
    while (WiFiMulti.run() != WL_CONNECTED) {
      delay(1000);
    }
    
    onConnected();
}

/**
 * When connection is restored, log the details.
 */
void onConnected() {
    // Set the hostname,
    WiFi.hostname("LOGITECH-Z906");
    
    // Configure MDNS.
    MDNS.begin("logitech-z906");
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
        int parsedValue;
        bool isValid = validateInputValue("value", parsedValue);
    
        if (isValid) {
            LOGI.cmd(endpoint.action, parsedValue);
            return server.send(200, "application/json", "{\"status\": \"connected\", \"success\": true }");
        }

        // Invalid
        return server.send(400, "application/json", "{\"status\": \"connected\", \"success\": false, \"message\": \"Invalid value. Value must be between 0 and 255.\" }");
              
    } else if (endpoint.type == GetValue) {
        return server.send(200, "application/json", "{\"status\": \"connected\", \"success\": true, \"value\": "+String(LOGI.request(endpoint.action))+" }");

    } else if (endpoint.type == RunFunction) {
        if (endpoint.path == "temperature") return handleGetTemperature();
        if (endpoint.path == "power/on") return handlePowerToggle(true);
        if (endpoint.path == "power/off") return handlePowerToggle(false);
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
 * Gracefully turn the power on or off.
 */
void handlePowerToggle(bool turnOn) {
    if (turnOn) {
        LOGI.on();
    } else {
        LOGI.off();
    }
    
    return server.send(200, "application/json", "{\"status\": \"connected\", \"success\": true }");
}

/**
 * Validate and parse the input value.
 * Returns true if the value is valid, false otherwise.
 * If valid, the parsed value is stored in the 'result' parameter.
 */
bool validateInputValue(const String& argName, int& result) {
    if (server.hasArg(argName)) {
        String valueStr = server.arg(argName);
        int value = valueStr.toInt();

        // Check if the conversion was successful
        if (value != 0 || valueStr.equals("0")) {
            // Valid value, store the result.
            result = value;

            if (result >= 0 && result <= 255) {
                return true;  // Value is valid.
            } else {
                // Value is not in the valid range.
                return false;
            }
        } else {
            // Invalid value format.
            return false;
        }
    } else {
        // No value provided in the request.
        return false;
    }
}

/**
 * Setup
 */
void setup() {
  initWiFi();
  initWebServer();
}

/**
 * Loop
 */
void loop() {
  if (WiFi.status() != WL_CONNECTED) connectToWiFi();
  server.handleClient();
}
