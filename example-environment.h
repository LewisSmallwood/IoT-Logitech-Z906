struct Wifi {
  const char* ssid;
  const char* password;
};

//Add multiple ssid's if required
const Wifi wifi_credentials[] = {
  { "YOUR_SSID_1", "YOUR_PASSWORD_1" }//,
  //{ "YOUR_SSID_2", "YOUR_PASSWORD_2" }
};

struct Settings {
  const char* hostname;
  const char* mdnsname;
};

//Set network hostname and mdns name (Use only compliant scheme, no checks beeing performed!)
const Settings network_settings = {
  { "LOGITECH-Z906", "logitech-z906" }
};
