/**
 * Internet Connected Logitech Z906 API.
 * Based on the works of Zarpli - Alejandro Zárate (https://github.com/zarpli/LOGItech-Z906/).
 */
#include <Arduino.h>
#include "./environment.h"
#include "./Z906.h"
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

  // Handle temperature monitoring.
  if (endpoint == "temperature") return server.send(200, "application/json", "{\"status\": \"connected\", \"temperature\": "+String(LOGI.main_sensor())+"}");

  // TODO: Setup other endpoints. e.g:
  // Power ON amplifier:    LOGI.on();
  // Select RCA 2.0 Input:  LOGI.input(SELECT_INPUT_2);
  // Disable Mute:          LOGI.cmd(MUTE_OFF);
  // Set Main Level to 15:  LOGI.cmd(MAIN_LEVEL, 15);

  // If not found, redirect to home.
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
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
