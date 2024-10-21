# 2425_ESE_Project_TagBot
Welcome to the TagBot project created by the amazing team consisted of Lucas Chapart, Charlotte Fricot and Marie Caronello.
You may choose the language for the project description.


  ## Introduction
  
  You are currently in the TagBot project (or cat robot), which is part of a class project where each team must design a robot. These robots must be able to play tag together, with a cat and mice, on a table with no edges.
  
  This project involves designing a cat robot which must:
  - Move on a table without edges (without falling off)
  - Be able to change state (cat or mouse)
    - If the robot is a cat, it must be able to catch the mouse
    - If the robot is a mouse, it must be able to escape from the cat
  
  We have several levels of objectives to achieve:
  Level 0: Robot moves + does not fall off the table
  Level 1: Detects another robot and approaches it (cat) or moves away (mouse)
  Level 2: Changes behavior after contact + operates with multiple robots
  Level 3: Capable of self-localization + not affected by obstacles outside the table
  
  ## Microprocessor System
  
  The first 10 sessions are dedicated to creating the PCB, which must be ordered before the All Saints' Day holidays.
  
  Session 1 -> Architectural Diagram / BOM
  Session 2 and 3 -> Annotated Electronic Diagram
  Session 4 -> Corrections to Diagram / Final BOM
  Session 5 -> Placement
  Session 6 -> Corrected Placement
  Session 7 and 8 -> Routing
  Session 9 and 10 -> Routing Corrections, Export
  
  # Code Documentation

[View the documentation](./Documents/Doxygen_Documentation/html/index.html)

  # Lidar X4 Driver
  
  The file `X4_driver.c` provides an implementation to interface with the X4 device via UART communication. This driver supports various functionalities, including starting and stopping scans, retrieving device information and status, and processing scan data. The driver uses a protocol defined in the header file `X4_driver.h` and handles responses from the X4 device.
  
  - **STM32 HAL Library** for UART communication.
  - **Standard C Libraries** (string.h, math.h, stdlib.h, stdbool.h).
  
  <details>
    <summary><strong>Header File X4_driver.h</strong></summary>
  
    ## Header File: `X4_driver.h`
  
    ### Macros
  
    - **Command Bytes:**
      - `X4_CMD_START` - Start of a command sequence.
      - `X4_CMD_START_SCAN` - Command to start a scan.
      - `X4_CMD_STOP_SCAN` - Command to stop a scan.
      - `X4_CMD_GET_INFO` - Command to get device information.
      - `X4_CMD_GET_HEALTH` - Command to get device status.
      - `X4_CMD_SOFT_RESTART` - Command for a soft restart.
  
    - **Response Protocol:**
      - `X4_RESPONSE_START_SIGN` - Expected start signature of a response.
      - `X4_RESPONSE_SINGLE_MODE` - Single response mode.
      - `X4_RESPONSE_CONTINUOUS_MODE` - Continuous response mode.
  
    - **Response Sizes:**
      - `X4_RESPONSE_HEADER_SIZE` - Size of the response header.
      - `X4_MAX_RESPONSE_SIZE` - Maximum size of the response content.
      - `X4_SERIAL_NUMBER_SIZE` - Size of the serial number.
      - `X4_SERIAL_FIRMWARE_SIZE` - Size of the firmware version.
  
    ### Structures
  
    - **`X4_ResponseMessage`**
      - Contains fields for parsing the response message, including start signature, response length, mode, type code, and content.
  
    - **`X4_DeviceInfo`**
      - Contains fields for device model, firmware version, hardware version, and serial number.
  
    - **`X4_ScanData`**
      - Contains fields for packet header, packet type, sample count, angles, checksum, and dynamically allocated sample data, distances, and angles.
  
    ### Function Prototypes
  
    - **`void X4_StartScan(void);`**
      - Starts a scan on the X4 device.
  
    - **`void X4_StopScan(void);`**
      - Stops the ongoing scan.
  
    - **`void X4_GetDeviceInfo(void);`**
      - Retrieves information about the device.
  
    - **`void X4_GetHealthStatus(void);`**
      - Retrieves the device's status.
  
    - **`void X4_SoftRestart(void);`**
      - Performs a soft restart of the device.
  
    - **`void X4_HandleResponse(void);`**
      - Handles the response received from the X4 device.
  
    - **`void X4_ParseMessage(const uint8_t *raw_data, X4_ResponseMessage *response);`**
      - Parses a raw message into a structured response.
  
    - **`uint16_t convertBytesToUint16(const uint8_t* byte_array);`**
      - Converts a 2-byte array into a 16-bit unsigned integer.
  
  </details>
  
  <details>
    <summary><strong>Source File: X4_driver.c</strong></summary>
  
    ## Key Functions
  
    - **`static void X4_SendCommand(uint8_t command)`**
      - Sends a command to the X4 device via UART.
  
    - **`void X4_StartScan(void)`**
      - Sends the start scan command and handles the response.
  
    - **`void X4_StopScan(void)`**
      - Sends the stop scan command.
  
    - **`void X4_SoftRestart(void)`**
      - Sends the soft restart command.
  
    - **`void X4_GetDeviceInfo(void)`**
      - Sends the command to get device information and processes the response.
  
    - **`void X4_GetDeviceHealth(void)`**
      - Sends the command to get device status and processes the response.
  
    - **`void X4_HandleDeviceInfoResponse(const X4_ResponseMessage* response)`**
      - Handles and parses the device information response.
  
    - **`void X4_HandleDeviceHealthResponse(const X4_ResponseMessage* response)`**
      - Handles and parses the device status response.
  
    - **`void X4_HandleScanResponse(const X4_ResponseMessage* response)`**
      - Handles and parses the scan data response.
  
    - **`void X4_HandleResponse(void)`**
      - Receives and processes UART data, sending it to the appropriate handler.
  
    - **`void X4_ParseMessage(const uint8_t *raw_data, X4_ResponseMessage *response)`**
      - Parses a raw message into a structured response.
  
    - **`void X4_HandleScanDataDistances(X4_ScanData *scan_data)`**
      - Processes and calculates distances from scan data.
  
    - **`void X4_HandleScanDataAngles(X4_ScanData *scan_data)`**
      - Processes and calculates angles from scan data.
  
    - **`uint16_t calculateXOR(const X4_ScanData *scan_data, size_t packet_length)`**
      - Calculates the XOR of all bytes in the packet, excluding the checksum.
  
    - **`bool verifyCheckCode(const X4_ScanData *scan_data, size_t packet_length)`**
      - Verifies the integrity of the scan data using the checksum.
  
    - **`uint16_t convertBytesToUint16(const uint8_t* byte_array)`**
      - Converts a 2-byte array into a 16-bit unsigned integer.
  </details>
  
  <details>
    <summary><strong>Usage</strong></summary>
    
    1. **Initialization:**
       - Ensure UART is properly initialized and configured in your main application.
  
    2. **Sending Commands:**
       - Use functions like `X4_StartScan()`, `X4_StopScan()`, and `X4_GetDeviceInfo()` to interact with the X4 device.
  
    3. **Handling Responses:**
       - Call `X4_HandleResponse()` to receive and process responses. Implement appropriate handlers for device information, device status, and scan data.
  
    4. **Processing Scan Data:**
       - Use `X4_HandleScanDataDistances()` and `X4_HandleScanDataAngles()` to process scan data and calculate distances and angles.
  </details>
  
  <details>
    <summary><strong>Notes</strong></summary>
    ## Notes
  
    - Ensure that the `HAL_UART_Receive` function is configured with an appropriate timeout and error handling as required by your application.
    - Adjust `X4_MAX_RESPONSE_SIZE` in the header file if necessary based on the expected response size from the device.
  </details>
  
  ## Hardware
  
  Most of the hardware has been specified for us, but we need to find our own method to detect the edges to avoid falling off the table. We would like to use IR sensors for this purpose. The sensor will be placed under the robot and will detect the voltage which determines its distance from the ground. When this signal decreases significantly (i.e., the distance increases), it indicates that we are near an edge. Since it will be placed under the robot, we need a sensor capable of detecting small distances, such as the Sharp GP2Y0A21SK0F sensor.
  
  We also want to add a visible LED to indicate the state of our cat robot.

