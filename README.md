# IoT Logitech Z906
The Logitech Z906 is a 5.1 THX-certified surround sound system. It is a powerful system with the ability to decode Dolby Digital and DTS-encoded soundtracks, but it lacks WiFi/network control.

<p align="center"><img src=/images/logitech_z906.webp></p>

Therefore, this project is a hardware and software solution that allows the unit to be controlled over the internet, adding smart WiFi control capabilities.

The [Logitech Z906](datasheet/Z906_User_Manual.pdf) usually cannot operate without the console connected, however, this solution removes the need for the console, replacing it with an ESP8266 control board.

The serial control logic for this project is based on the works of [zarpli](https://github.com/zarpli/Logitech-Z906).

## Hardware
To enable the control of the unit over the internet, an ESP8266 microcontroller was used which is a low-cost WiFi connected microcontroller.

The controller was wired up to the unit by using the DE-15 console port, which makes use of TTL serial communication at 3.3V.

The following table illustrates the pinout.

|Pin|Description|Pin|Description|
|---|---|---|---|
|3|GND|6|GND|
|11|3.3V @ 250mA|12|TX|
|13|RX|15|Console Enable|

D2-71583 **NO** is 5V tolerant. If you have Arduino UNO, MEGA, etc use a **Logic Level Converter.**

|Serial settings||
|---|---|
|Baud rate|57600|
|Data|8 bit|
|Parity|Odd|
|Stop|1 bit|

Here is the DE-15 male as viewed from the front of the Main Board.

<img src=/images/DE-15-M.jpg width="200">

# Enable Console

Connect pin 15 to GND.

<p align="center"><img src=/images/Z906-ECON.jpg></p>


## Software
### Environment Variables
An environment variables file is included in the project.

An example of this file is provided in the repository (called `example-environment.h`).
This file should be renamed to `environment.h` and populated in the project.

It contains specific configuration details for your project, including WiFi passwords.

To specify the network credentials you wish to use, the network credentials list can be modified in the `environment.h` file as follows:

```
const Network network_credentials[] = {
  { "YOUR_SSID_1", "YOUR_PASSWORD_1" },
  { "YOUR_SSID_2", "YOUR_PASSWORD_2" },
};
```

### Control API
The project makes the following HTTP endpoints available to control all the functions of the Logitech Z906 over the API.

When making a web request to any of these endpoints, the microcontroller will run the desired action, interfacing with the Logitech Z906.

#### Endpoints
|Endpoint|Parameters|Serial Command|Description|
|---|---|---|---|
|/input/1|-|SELECT_INPUT_1|Swap to the TRS 5.1 input|
|/input/2|-|SELECT_INPUT_2|Swap to the RCA 2.0 input|
|/input/3|-|SELECT_INPUT_3|Swap to the Optical 1 input|
|/input/4|-|SELECT_INPUT_4|Swap to the Optical 2 input|
|/input/5|-|SELECT_INPUT_5|Swap to the Coaxial input|
|/input/disable|-|BLOCK_INPUTS|Disable signal input|
|/input/enable|-|NO_BLOCK_INPUTS|Enable signal input|
||
|/volume/main|-|MAIN_LEVEL|Return the current Main Level|
|/volume/main/set|value: 0-255|MAIN_LEVEL|Set the Main Level to the parameter value|
|/volume/main/up|-|LEVEL_MAIN_UP|Increase Main Level by one unit|
|/volume/main/down|-|LEVEL_MAIN_DOWN|Decrease Main Level by one unit|
|/volume/subwoofer|-|SUB_LEVEL|Return the current Subwoofer Level|
|/volume/subwoofer/set|value: 0-255|SUB_LEVEL|Set the Subwoofer Level to the parameter value|
|/volume/subwoofer/up|-|LEVEL_SUB_UP|Increase Subwoofer Level by one unit|
|/volume/subwoofer/down|-|LEVEL_SUB_DOWN|Decrease Subwoofer Level by one unit|
|/volume/center|-|CENTER_LEVEL|Return the current Center Level|
|/volume/center/set|value: 0-255|CENTER_LEVEL|Set the Center Level to the parameter value|
|/volume/center/up|-|LEVEL_CENTER_UP|Increase Center Level by one unit|
|/volume/center/down|-|LEVEL_CENTER_DOWN|Decrease Subwoofer Level by one unit|
|/volume/rear|-|REAR_LEVEL|Return the current Rear Level|
|/volume/rear/set|value: 0-255|REAR_LEVEL|Set the Rear Level to the parameter value|
|/volume/rear/up|-|LEVEL_REAR_UP|Increase Rear Level by one unit|
|/volume/rear/down|-|LEVEL_REAR_DOWN|Decrease Rear Level by one unit|
||
|/input/decode/on|-|SELECT_EFFECT_51|Enable Decode Mode|
|/input/decode/off|-|DISABLE_EFFECT_51|Disable Decode Mode|
||
|/input/effect/3D|-|SELECT_EFFECT_3D|Enable 3D Effect in current input|
|/input/effect/4.1|-|SELECT_EFFECT_41|Enable 4.1 Effect in current input|
|/input/effect/2.1|-|SELECT_EFFECT_21|Enable 2.1 Effect in current input|
|/input/effect/off|-|SELECT_EFFECT_NO|Disable all Effects in current input|
||
|/save|-|EEPROM_SAVE|Save current settings to EEPROM*|
||
|/mute/on|-|MUTE_ON|Enable Mute|
|/mute/off|-|MUTE_OFF|Disable Mute|
||
|/temperature|-|GET_TEMP|Gets the system temperature|
|/version|-|VERSION|Gets the system firmware version|
|/power|-|STATUS_STBY|Get the current standby status|
|/power/on|-|PWM_ON|Turn the system on|
|/power/off|-|PWM_OFF|Turn the system off|

*Please note, use the **EEPROM_SAVE** function with caution. Each EEPROM has a limited number of write cycles (~100,000) per address. If you write excessively to the EEPROM, you will reduce the lifespan.
