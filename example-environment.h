struct Wifi {
  const char* ssid;
  const char* password;
};

struct Settings {
  const char* hostname;
  const char* mdnsname;
};

// Add multiple WiFi Networks (SSIDs) as required.
const Wifi wifi_credentials[] = {
  { "YOUR_SSID_1", "YOUR_PASSWORD" },
  { "YOUR_SSID_2", "YOUR_PASSWORD" }
};

// Set the network hostname and MDNS name.
const Settings network_settings = {
  .hostname = "LOGITECH-Z906",
  .mdnsname = "logitech-z906"
};
