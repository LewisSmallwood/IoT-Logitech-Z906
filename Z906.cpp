#include "Z906.h"

// Constructor for Z906 class.
Z906::Z906(HardwareSerial &serial) {
  dev_serial = &serial;
	dev_serial->begin(BAUD_RATE,SERIAL_CONFIG);
}

/**
 * Calculate the Longitudinal Redundancy Check (LRC) for {-1,-1}.
 *
 * @param pData Pointer to the data array.
 * @param length Length of the data array.
 * @return Calculated LRC value.
 */
uint8_t Z906::LRC(uint8_t* pData, size_t length) {
    uint8_t lrc = 0;
    for (size_t i = 1; i < length - 1; i++)lrc -= pData[i];
    return lrc;
}

/**
 * Turn the Z906 unit on.
 *
 * This function sends the command to turn on the Z906 unit.
 */
void Z906::on() {
    write(PWM_ON);
}

/**
 * Turn the Z906 unit off and reset power-up time.
 *
 * This function sends the command to turn off the Z906 unit, resets the power-up time,
 * and discards the acknowledgment (ACK) message for a clean serial buffer.
 */
void Z906::off() {
    // Send the command to turn off the Z906 unit
    write(PWM_OFF);

    // Reset the power-up time and save changes to EEPROM
    uint8_t cmd[] = {RESET_PWR_UP_TIME, 0x37, EEPROM_SAVE};
    write(cmd, sizeof(cmd));

    // Discard the acknowledgment (ACK) message for a clean serial buffer
    flush();
}

/**
 * Change the input on the Z906 unit specifying an effect.
 *
 * This function changes the input on the Z906 unit and specifies an effect if provided.
 * If no effect is selected, it uses the default effect (same as console default).
 *
 * @param input The input to be set on the Z906 unit.
 * @param effect The effect to be applied to the input.
 */
void Z906::input(uint8_t input, uint8_t effect) {
    // If no effect is selected, use the default (same as console default)
    if (effect == 0xFF) {
        if (input == SELECT_INPUT_2 || input == SELECT_INPUT_AUX) {
            effect = SELECT_EFFECT_3D;
        } else {
            effect = SELECT_EFFECT_NO;
        }
    }

    // Send the command to change the input with specified effect
    uint8_t cmd[] = {MUTE_ON, input, effect, MUTE_OFF};
    write(cmd, sizeof(cmd));

    // Discard the acknowledgment (ACK) message for a clean serial buffer
    flush();
}

/**
 * Flush the serial communication buffers.
 *
 * This function introduces a deadtime delay to avoid UART TX collisions and clears the RX buffer.
 */
void Z906::flush() {
    // Introduce a deadtime delay to avoid UART TX collisions
    delay(SERIAL_DEADTIME);

    // Clear the RX buffer by reading any available data
    while (dev_serial->available() > 0) {
        dev_serial->read();
    }
}

/**
 * Write a single-byte command to the Z906 TX port.
 *
 * This function writes a specified single-byte command to the Z906 TX port, flushing the TX buffer afterward.
 *
 * @param cmd The single-byte command to be written.
 */
void Z906::write(uint8_t cmd) {
    // Flush the communication buffers to ensure a clean start
    flush();

    // Write the specified single-byte command to the Z906 TX port
    dev_serial->write(cmd);

    // Flush the TX buffer to ensure the command is sent to the device
    dev_serial->flush();
}

/**
 * Write a command (byte array) to the Z906 TX port.
 *
 * This function writes a command represented by a byte array to the Z906 TX port,
 * flushing the TX buffer afterward.
 *
 * @param pCmd A pointer to the byte array representing the command.
 * @param cmd_len The length of the command byte array.
 */
void Z906::write(uint8_t* pCmd, size_t cmd_len) {
    // Flush the communication buffers to ensure a clean start
    flush();

    // Write each byte of the command byte array to the Z906 TX port
    for (size_t i = 0; i < cmd_len; i++) {
        dev_serial->write(pCmd[i]);
    }

    // Flush the TX buffer to ensure the command is sent to the device
    dev_serial->flush();
}

/**
 * Update the status of the Z906 device.
 * This function sends a command to retrieve the current status of the Z906,
 * reads the response from the serial interface, and validates the received data.
 *
 * @return 1 if the update is successful, 0 otherwise.
 */
int Z906::update() {
    // Send command to request device status
    write(GET_STATUS);

    // Record the current time for timeout monitoring
    unsigned long currentMillis = millis();

    // Wait until the expected status data is available in the serial buffer
    while (dev_serial->available() < STATUS_LENGTH + 1) {
        // Check for timeout
        if (millis() - currentMillis > SERIAL_TIME_OUT) {
            return 0;
        }
    }

    // Read the status data into the buffer
    for (int i = 0; i <= STATUS_LENGTH; i++) {
        status[i] = dev_serial->read();
    }

    // Extract payload size and calculate the total buffer size
    size_t payload_len = status[STATUS_LENGTH];     // Size of the payload
    status_len = payload_len + 4;                   // Size of full status buffer in RAM

    // Wait until the full payload is available in the serial buffer
    while (dev_serial->available() < payload_len + 1) {
        // Check for timeout
        if (millis() - currentMillis > SERIAL_TIME_OUT) {
            return 0;
        }
    }

    // Read payload and checksum into the status buffer
    for (int i = 0; i <= payload_len; i++) {
        status[i + STATUS_LENGTH + 1] = dev_serial->read();
    }
    STATUS_CHECKSUM = status_len - 1;

    // Validate the received status data
    if (status[STATUS_STX] != EXP_STX || status[STATUS_MODEL] != EXP_MODEL_STATUS ||
        status[STATUS_CHECKSUM] != LRC(status, status_len)) {
        return 0;
    }

    // Update successful
    return 1;
}

/**
 * Request data from the Z906 unit based on the specified command.
 *
 * This function updates the internal status buffer by querying the device,
 * and then returns specific data based on the provided command.
 *
 * @param cmd The command indicating the type of data to request from the Z906 unit.
 * @return The requested data, or 0 if the update operation fails or the command is invalid.
 */
int Z906::request(uint8_t cmd) {
    if (update()) {
        // Return specific data based on the provided command
        if (cmd == VERSION) {
            // Combine version bytes to form a single integer
            return status[STATUS_VER_C] + 10 * status[STATUS_VER_B] + 100 * status[STATUS_VER_A];
        }
        
        if (cmd == CURRENT_INPUT) {
            // Increment the current input value by 1
            return status[STATUS_CURRENT_INPUT] + 1;
        }
        
        if (cmd == MAIN_LEVEL || cmd == REAR_LEVEL || cmd == CENTER_LEVEL || cmd == SUB_LEVEL) {
            // Normalize volume data to the range 0...255
            return static_cast<uint8_t>((static_cast<uint16_t>(status[cmd]) * 255) / MAX_VOL);
        }
        
        // Return the requested data based on the command
        return status[cmd];
    }

    // Return 0 if the update operation fails
    return 0;
}

/**
 * Send a command to the Z906 device and return the response.
 *
 * This function sends a specified command to the Z906 device, waits for the response,
 * and returns the received data.
 *
 * @param cmd The command to be sent to the Z906 device.
 * @return The response received from the Z906 device, or 0 if the operation times out.
 */
int Z906::cmd(uint8_t cmd) {
    // Send the specified command to the device
    write(cmd);

    // Record the current time for timeout monitoring
    unsigned long currentMillis = millis();

    // Wait until response data is available in the serial buffer
    while (dev_serial->available() == 0) {
        // Check for timeout
        if (millis() - currentMillis > SERIAL_TIME_OUT) {
            return 0;
        }
    }

    // Return the received response
    return dev_serial->read();
}

/**
 * Send a command to update a specific parameter on the Z906 device.
 *
 * This function updates a specified parameter on the Z906 device with the provided value,
 * normalizes the volume if necessary, and sends the updated status to the device.
 * Additionally, it discards the acknowledgment (ACK) message to maintain a clean serial buffer.
 *
 * @param cmd_a The command representing the parameter to be updated (e.g., MAIN_LEVEL, REAR_LEVEL, etc.).
 * @param cmd_b The value to be set for the specified parameter.
 */
int Z906::cmd(uint8_t cmd_a, uint8_t cmd_b) {
    // Update the internal status buffer with the current device status
    update();

    // Normalize volume to the range 0...255 if applicable
    if (cmd_a == MAIN_LEVEL || cmd_a == REAR_LEVEL || cmd_a == CENTER_LEVEL || cmd_a == SUB_LEVEL) {
        cmd_b = static_cast<uint8_t>((static_cast<uint16_t>(cmd_b) * MAX_VOL) / 255);
    }

    // Update the specified parameter in the internal status buffer
    status[cmd_a] = cmd_b;

    // Update the checksum in the status buffer
    status[STATUS_CHECKSUM] = LRC(status, status_len);

    // Send the updated status buffer to the Z906 device
    write(status, status_len);

    // Discard the acknowledgment (ACK) message to maintain a clean serial buffer
    flush();
}

/**
 * Print the current status of the Z906 device to the serial monitor.
 *
 * This function updates the internal status buffer by querying the device,
 * and then prints each byte of the status buffer in hexadecimal format to the serial monitor.
 */
void Z906::print_status() {
    // Update the internal status buffer with the current device status
    update();

    // Print each byte of the status buffer in hexadecimal format
    for (int i = 0; i < status_len; i++) {
        Serial.print(status[i], HEX);
        Serial.print(" ");
    }
    
    Serial.print("\n");
}

/**
 * Retrieve the temperature reading from the main sensor of the Z906 device.
 *
 * This function sends a command to request the temperature reading from the main sensor,
 * waits for the response, and returns the temperature value if the operation is successful.
 *
 * @return The temperature reading from the main sensor, or 0 if the operation times out or the response is invalid.
 */
uint8_t Z906::main_sensor() {
    // Send command to request temperature from the main sensor
    write(GET_TEMP);

    // Record the current time for timeout monitoring
    unsigned long currentMillis = millis();

    // Wait until the full temperature response is available in the serial buffer
    while (dev_serial->available() < TEMP_TOTAL_LENGTH) {
        // Check for timeout
        if (millis() - currentMillis > SERIAL_TIME_OUT) return 0;
    }

    // Read the temperature response into a temporary buffer
    uint8_t temp[TEMP_TOTAL_LENGTH];
    for (int i = 0; i < TEMP_TOTAL_LENGTH; i++) {
        temp[i] = dev_serial->read();
    }

    // Validate the temperature response
    if (temp[2] != EXP_MODEL_TEMP) return 0;

    // Return the temperature reading from the main sensor
    return temp[7];
}
