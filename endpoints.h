enum EndpointType { SelectInput, RunCommand, SetValue, GetValue, RunFunction };

struct Endpoint {
  const String path;
  const EndpointType type;
  const uint8_t action;
};

const Endpoint endpoints[] = {
  { "input", GetValue, CURRENT_INPUT },                     // Get the currently selected input
  { "input/1", SelectInput, SELECT_INPUT_1 },               // Swap to the TRS 5.1 input
  { "input/2", SelectInput, SELECT_INPUT_2 },               // Swap to the RCA 2.0 input
  { "input/3", SelectInput, SELECT_INPUT_3 },               // Swap to the Optical 1 input
  { "input/4", SelectInput, SELECT_INPUT_4 },               // Swap to the Optical 2 input
  { "input/5", SelectInput, SELECT_INPUT_5 },               // Swap to the Coaxial input

  { "input/enable", RunCommand, NO_BLOCK_INPUTS },          // Enable signal input
  { "input/disable", RunCommand, BLOCK_INPUTS },            // Disable signal input
  
  { "volume/main", GetValue, MAIN_LEVEL },                  // Get the current Main Level
  { "volume/main/set", SetValue, MAIN_LEVEL },              // Set the Main Level to the parameter value
  { "volume/main/up", RunCommand, LEVEL_MAIN_UP },          // Increase Main Level by one unit
  { "volume/main/down", RunCommand, LEVEL_MAIN_DOWN },      // Decrease Main Level by one unit

  { "volume/subwoofer", GetValue, SUB_LEVEL },              // Get the current Subwoofer Level
  { "volume/subwoofer/set", SetValue, SUB_LEVEL },          // Set the Subwoofer Level to the parameter value
  { "volume/subwoofer/up", RunCommand, LEVEL_SUB_UP },      // Increase Subwoofer Level by one unit
  { "volume/subwoofer/down", RunCommand, LEVEL_SUB_DOWN },  // Decrease Subwoofer Level by one unit

  { "volume/center", GetValue, CENTER_LEVEL },              // Get the current Center Level
  { "volume/center/set", SetValue, CENTER_LEVEL },          // Set the Center Level to the parameter value
  { "volume/center/up", RunCommand, LEVEL_CENTER_UP },      // Increase Center Level by one unit
  { "volume/center/down", RunCommand, LEVEL_CENTER_DOWN },  // Decrease Center Level by one unit

  { "volume/main", GetValue, MAIN_LEVEL },                  // Get the current Main Level
  { "volume/main/set", SetValue, MAIN_LEVEL },              // Set the Main Level to the parameter value
  { "volume/main/up", RunCommand, LEVEL_MAIN_UP },          // Increase Main Level by one unit
  { "volume/main/down", RunCommand, LEVEL_MAIN_DOWN },      // Decrease Main Level by one unit

  { "volume/rear", GetValue, REAR_LEVEL },                  // Get the current Rear Level
  { "volume/rear/set", SetValue, REAR_LEVEL },              // Set the Rear Level to the parameter value
  { "volume/rear/up", RunCommand, LEVEL_REAR_UP },          // Increase Rear Level by one unit
  { "volume/rear/down", RunCommand, LEVEL_REAR_DOWN },      // Decrease Rear Level by one unit

  { "input/decode/on", RunCommand, SELECT_EFFECT_51 },      // Enable Decode Mode
  { "input/decode/off", RunCommand, DISABLE_EFFECT_51 },    // Disable Decode Mode

  { "input/effect/3d", RunCommand, SELECT_EFFECT_3D },      // Enable 3D Effect on the current input
  { "input/effect/4.1", RunCommand, SELECT_EFFECT_41 },     // Enable 4.1 Effect on the current input
  { "input/effect/2.1", RunCommand, SELECT_EFFECT_21 },     // Enable 2.1 Effect on the current input
  { "input/effect/off", RunCommand, SELECT_EFFECT_NO },     // Disable all Effects on the current input

  { "save", RunCommand, EEPROM_SAVE },                      // Save current settings to EEPROM*
  { "mute/on", RunCommand, MUTE_ON },                       // Enable mute
  { "mute/off", RunCommand, MUTE_OFF },                     // Disable mute
  { "temperature", RunFunction, 0 },                        // Get the system temperature
  { "version", GetValue, VERSION },                         // Get the system firmware version
  { "power", GetValue, GET_STATUS },                        // Get the system standby status
  { "power/on", RunFunction, 0 },                           // Turn the system on
  { "power/off", RunFunction, 0 },                          // Turn the system off
};
