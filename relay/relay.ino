#include "config.h"
#include "EEPROM.h"
#include <Wire.h>

// Relay pin definition for XIAO ESP32S3
#define RELAY_PIN 0  // D0 pin to control the relay

// Variables to control the relay timer
bool timerActive = false;
uint32_t relayOffTime = 0;
uint32_t relayTimerDuration = 0;  // Duration in milliseconds

// regional choices: EU868, US915, AU915, AS923, IN865, KR920, CN780, CN500
const LoRaWANBand_t Region = EU868;
const uint8_t subBand = 0; // For US915 and AU915

// SX1262 pin order: Module(NSS/CS, DIO1, RESET, BUSY);
SX1262 radio = new Module(41, 39, 42, 40);

// create the LoRaWAN node
LoRaWANNode node(&radio, &Region, subBand);

uint64_t joinEUI =   RADIOLIB_LORAWAN_JOIN_EUI;
uint64_t devEUI  =   RADIOLIB_LORAWAN_DEV_EUI;
uint8_t appKey[] = { RADIOLIB_LORAWAN_APP_KEY };
uint8_t nwkKey[] = { RADIOLIB_LORAWAN_NWK_KEY };

#define LORAWAN_DEV_INFO_SIZE 36
uint8_t deviceInfo[LORAWAN_DEV_INFO_SIZE] = {0};

#define SERIAL_DATA_BUF_LEN  64
uint8_t serialDataBuf[SERIAL_DATA_BUF_LEN] = {0};
uint8_t serialIndex = 0;

#define UPLINK_PAYLOAD_MAX_LEN  256
uint8_t uplinkPayload[UPLINK_PAYLOAD_MAX_LEN] = {0};
uint16_t uplinkPayloadLen = 0;

uint32_t previousMillis = 0;

// Functions to control the relay
void relayOn() {
  digitalWrite(RELAY_PIN, HIGH);
  Serial.println(F("Relay turned ON"));
}

void relayOff() {
  digitalWrite(RELAY_PIN, LOW);
  Serial.println(F("Relay turned OFF"));
  // Deactivate the timer when the relay is manually turned off
  timerActive = false;
}

void toggleRelay() {
  if (digitalRead(RELAY_PIN) == HIGH) {
    relayOff();
  } else {
    relayOn();
  }
}

// New function to turn on the relay with a timer
void relayOnWithTimer(uint32_t durationSeconds) {
  relayOn();
  timerActive = true;
  relayTimerDuration = durationSeconds * 1000; // Convert seconds to milliseconds
  relayOffTime = millis() + relayTimerDuration;
  
  Serial.print(F("Relay turned ON with timer for "));
  Serial.print(durationSeconds);
  Serial.println(F(" seconds"));
}

// Function to check and manage the relay timer
void checkRelayTimer() {
  if (timerActive && millis() >= relayOffTime) {
    relayOff();
    timerActive = false;
    Serial.println(F("Relay turned OFF by timer"));
  }
}

// Function to process commands received via LoRaWAN
void processDownlink(uint8_t* payload, uint8_t payloadLength, uint8_t port) {
  if (payloadLength > 0) {
    Serial.print(F("Received downlink on port "));
    Serial.print(port);
    Serial.print(F(": "));
    for (uint8_t i = 0; i < payloadLength; i++) {
      Serial.print(payload[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    
    // Process commands for the relay
    if (payloadLength == 1) {
      // Basic commands with a single byte
      switch(payload[0]) {
        case 0x01: // Turn ON relay - Command: 0x01
          relayOn();
          break;
        case 0x00: // Turn OFF relay - Command: 0x00
          relayOff();
          break;
        case 0x02: // Toggle relay state - Command: 0x02
          toggleRelay();
          break;
        default:
          Serial.println(F("Unknown command"));
          break;
      }
    } 
    else if (payloadLength == 2 && payload[0] == 0x03) {
      // Command 0x03 + 1 byte: Turn ON relay with timer for 0-255 seconds
      // Example: 0x03 0x1E = Turn on for 30 seconds
      uint8_t timerDuration = payload[1];
      relayOnWithTimer(timerDuration);
    }
    else if (payloadLength == 3 && payload[0] == 0x04) {
      // Command 0x04 + 2 bytes: Turn ON relay with timer for 0-65535 seconds
      // Example: 0x04 0x01 0x2C = Turn on for 300 seconds (5 minutes)
      // First byte is high byte (MSB), second byte is low byte (LSB)
      uint16_t timerDuration = (payload[1] << 8) | payload[2];
      relayOnWithTimer(timerDuration);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Configure the relay pin as output and initially off
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  Serial.println(F("Relay initialized"));

  if(!EEPROM.begin(LORAWAN_DEV_INFO_SIZE))
  {
    Serial.println("Failed to initialize EEPROM");
    while(1);
  }

  uint32_t now = millis();
  while(1)
  {
    deviceInfoSet();
    if(millis() - now >= 5000) break;
  }

  deviceInfoLoad();
  Serial.println(F("\nSetup... "));  
  Serial.println(F("Initialise the radio"));
  int16_t state = radio.begin();
  debug(state!= RADIOLIB_ERR_NONE, F("Initialise radio failed"), state, true);

  // SX1262 rf switch order: setRfSwitchPins(rxEn, txEn);
  radio.setRfSwitchPins(38, RADIOLIB_NC);

  // Setup the OTAA session information
  node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);
  Serial.println(F("Join ('login') the LoRaWAN Network"));

  while(1)
  {
    state = node.activateOTAA(LORAWAN_UPLINK_DATA_RATE);
    if(state == RADIOLIB_LORAWAN_NEW_SESSION) break;
    debug(state!= RADIOLIB_LORAWAN_NEW_SESSION, F("Join failed"), state, true);
    delay(15000);
  }

  // Disable the ADR algorithm (on by default which is preferable)
  node.setADR(false);

  // Set a fixed datarate
  node.setDatarate(LORAWAN_UPLINK_DATA_RATE);

  // Manages uplink intervals to the TTN Fair Use Policy
  node.setDutyCycle(false);
  
  Serial.println(F("Ready!\n"));

  Wire.begin();
}

void loop() {
  // Current time for interval management
  uint32_t currentMillis = millis();
  
  // Check if it's time to send a LoRaWAN message
  if(currentMillis - previousMillis >= LORAWAN_UPLINK_PERIOD)
  {
    previousMillis = currentMillis;
    
    // Prepare data to send - including the current relay state and timer status
    uplinkPayload[0] = digitalRead(RELAY_PIN); // Relay state (0 = OFF, 1 = ON)
    uplinkPayload[1] = timerActive ? 1 : 0;    // Timer status (0 = inactive, 1 = active)
    
    // If timer is active, include remaining time in seconds
    if (timerActive) {
      uint32_t remainingTime = 0;
      if (relayOffTime > currentMillis) {
        remainingTime = (relayOffTime - currentMillis) / 1000; // Convert to seconds
      }
      // Send remaining time as 2 bytes (up to 65535 seconds)
      uplinkPayload[2] = (remainingTime >> 8) & 0xFF;  // High byte
      uplinkPayload[3] = remainingTime & 0xFF;         // Low byte
      uplinkPayloadLen = 4;
    } else {
      uplinkPayloadLen = 2;
    }
    
    Serial.println(F("Sending uplink"));
    int16_t state = node.sendReceive(uplinkPayload, uplinkPayloadLen, LORAWAN_UPLINK_USER_PORT);
    
    // Process received response (if any)
    if(state == RADIOLIB_ERR_NONE) {
      // Get the received data
      uint8_t downlinkPayload[256];
      size_t downlinkPayloadLen = 0;
      uint8_t downlinkPort = 0;
      
      // Read the received data
      node.getDownlinkData(downlinkPayload, &downlinkPayloadLen, &downlinkPort);
      
      // Process the received commands
      processDownlink(downlinkPayload, downlinkPayloadLen, downlinkPort);
    }
    
    debug((state!= RADIOLIB_LORAWAN_NO_DOWNLINK) && (state!= RADIOLIB_ERR_NONE), F("Error in sendReceive"), state, false);
    uplinkPayloadLen = 0;
  }
  
  // Check if the timer has expired and turn off the relay if needed
  checkRelayTimer();
  
  delay(1000);
}

void deviceInfoLoad() {
  uint16_t checkSum = 0, checkSum_ = 0;
  for(int i = 0; i < LORAWAN_DEV_INFO_SIZE; i++) deviceInfo[i] = EEPROM.read(i);
  for(int i = 0; i < 32; i++) checkSum += deviceInfo[i];
  memcpy((uint8_t *)(&checkSum_), deviceInfo + 32, 2);

  if(checkSum == checkSum_)
  {
    memcpyr((uint8_t *)(&joinEUI), deviceInfo, 8);
    memcpyr((uint8_t *)(&devEUI), deviceInfo + 8, 8);
    memcpy(appKey, deviceInfo + 16, 16);

    Serial.println("Load device info:");
    Serial.print("JoinEUI:");
    Serial.println(joinEUI, HEX);
    Serial.print("DevEUI:");
    Serial.println(devEUI, HEX);
    Serial.print("AppKey:");
    arrayDump(appKey, 16);
    Serial.print("nwkKey:");
    arrayDump(nwkKey, 16);
  }
  else
  {
    Serial.println("Use the default device info as LoRaWAN param");
  }
}

void deviceInfoSet() {
  if(Serial.available())
  {
    serialDataBuf[serialIndex++] = Serial.read();
    if(serialIndex >= SERIAL_DATA_BUF_LEN) serialIndex = 0;
    if(serialIndex > 2 && serialDataBuf[serialIndex - 2] == '\r' && serialDataBuf[serialIndex-1] == '\n')
    {
      Serial.println("Get serial data:");
      arrayDump(serialDataBuf, serialIndex);
      if(serialIndex == 34) // 8 + 8 + 16 + 2
      {
        uint16_t checkSum = 0;
        for(int i = 0; i < 32; i++) checkSum += serialDataBuf[i];
        memcpy(deviceInfo, serialDataBuf, 32);
        memcpy(deviceInfo + 32, (uint8_t *)(&checkSum), 2);
        for(int i = 0; i < 34; i++) EEPROM.write(i, deviceInfo[i]);
        EEPROM.commit();
        Serial.println("Save serial data, please reboot...");
      }
      else
      {
        Serial.println("Error serial data length");
      }

      serialIndex = 0;
      memset(serialDataBuf, sizeof(serialDataBuf), 0);
    }
  }
}
