#include <SPI.h>
#include <LoRa.h>
#include <SoftwareSerial.h>

#define PIN_OBCTEMP A1
#define PIN_EPSTEMP A2
#define PIN_VBAT A0

// OBC <-> ADCS UART bus (connect to ADCS RX/TX pins)
const int ADCS_RX_PIN = 8; // OBC reads from ADCS TX
const int ADCS_TX_PIN = 9; // OBC writes to ADCS RX
SoftwareSerial adcsBus(ADCS_RX_PIN, ADCS_TX_PIN);

const long LORA_FREQUENCY_HZ = 434E6;
const unsigned long ADCS_TIMEOUT_MS = 1200;

unsigned long startMillis = 0;

float readTempFromPin(const int pin) {
  float reading = analogRead(pin);
  return (reading * (5100.00 / 1024.00)) / 10.00;
}

float readBatteryVoltage() {
  float vbatReading = analogRead(PIN_VBAT);
  return vbatReading * (5.10 / 1023.00);
}

String getElapsedTimeString() {
  unsigned long elapsedSeconds = (millis() - startMillis) / 1000;
  unsigned long minutes = elapsedSeconds / 60;
  unsigned long seconds = elapsedSeconds % 60;

  String out = "";
  out += minutes;
  out += "m ";
  out += seconds;
  out += "s";
  return out;
}

String queryADCS(const String &command) {
  unsigned long flushStartedAt = millis();
  int flushedBytes = 0;
  while (adcsBus.available() && flushedBytes < 128 && (millis() - flushStartedAt) < 50) {
    adcsBus.read();
    flushedBytes++;
  }

  adcsBus.println(command);

  unsigned long startedAt = millis();
  String response = "";
  while ((millis() - startedAt) < ADCS_TIMEOUT_MS) {
    if (adcsBus.available()) {
      response = adcsBus.readStringUntil('\n');
      response.trim();
      if (response.length() > 0) {
        return response;
      }
    }
  }

  return "ADCS_TIMEOUT";
}

String buildLocalStatus() {
  float tempObc = readTempFromPin(PIN_OBCTEMP);
  float tempEps = readTempFromPin(PIN_EPSTEMP);
  float vbat = readBatteryVoltage();

  String status = "SAT_STATUS|TIME=" + getElapsedTimeString();
  status += "|TEMPOBC=" + String(tempObc, 2);
  status += "|TEMPEPS=" + String(tempEps, 2);
  status += "|VBAT=" + String(vbat, 2);
  return status;
}

String handleCommand(String command) {
  command.trim();

  if (command == "TIMEOBC") {
    return "TIMEOBC=" + getElapsedTimeString();
  }

  if (command == "RTIMEOBC") {
    startMillis = millis();
    return "RTIMEOBC=OK";
  }

  if (command == "TEMPOBC") {
    return "TEMPOBC=" + String(readTempFromPin(PIN_OBCTEMP), 2) + "C";
  }

  if (command == "TEMPEPS") {
    return "TEMPEPS=" + String(readTempFromPin(PIN_EPSTEMP), 2) + "C";
  }

  if (command == "TEMPCOMPARISON") {
    float tempObc = readTempFromPin(PIN_OBCTEMP);
    float tempEps = readTempFromPin(PIN_EPSTEMP);
    float delta = abs(tempObc - tempEps);
    return "TEMPCOMPARISON|DELTA=" + String(delta, 2) + "C";
  }

  if (command == "VBAT") {
    return "VBAT=" + String(readBatteryVoltage(), 2) + "V";
  }

  if (command == "SAT_STATUS") {
    return buildLocalStatus();
  }

  if (command == "ADCS_STATUS" || command == "ADCS_POINT_SUN" || command == "ADCS_POINT_NADIR") {
    return queryADCS(command);
  }

  if (command.startsWith("ADCS_SET_ANGLE:")) {
    return queryADCS(command);
  }

  return "ERR=UNKNOWN_COMMAND";
}

void sendLoRaResponse(const String &response) {
  LoRa.beginPacket();
  LoRa.print("DATA:");
  LoRa.print(response);
  LoRa.endPacket();

  Serial.print("TX> DATA:");
  Serial.println(response);
}

void processIncomingLoRaCommand() {
  int packetSize = LoRa.parsePacket();
  if (!packetSize) {
    return;
  }

  String incoming = "";
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
  incoming.trim();

  Serial.print("RX> ");
  Serial.println(incoming);

  if (!incoming.startsWith("CMD:")) {
    sendLoRaResponse("ERR=BAD_FRAME");
    return;
  }

  String command = incoming.substring(4);
  String response = handleCommand(command);
  sendLoRaResponse(response);
}

void setup() {
  Serial.begin(9600);
  adcsBus.begin(9600);
  startMillis = millis();

  if (!LoRa.begin(LORA_FREQUENCY_HZ)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("OBC Mission Controller Ready");
}

void loop() {
  processIncomingLoRaCommand();
}
