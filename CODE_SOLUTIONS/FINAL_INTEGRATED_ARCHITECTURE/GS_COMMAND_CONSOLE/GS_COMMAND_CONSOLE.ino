#include <SPI.h>
#include <LoRa.h>

const long LORA_FREQUENCY_HZ = 434E6;

void printHelp() {
  Serial.println("GS COMMANDS:");
  Serial.println("  HELP");
  Serial.println("  TIMEOBC");
  Serial.println("  RTIMEOBC");
  Serial.println("  TEMPOBC");
  Serial.println("  TEMPEPS");
  Serial.println("  TEMPCOMPARISON");
  Serial.println("  VBAT");
  Serial.println("  SAT_STATUS");
  Serial.println("  ADCS_STATUS");
  Serial.println("  ADCS_POINT_SUN");
  Serial.println("  ADCS_POINT_NADIR");
  Serial.println("  ADCS_SET_ANGLE:<0-180>");
  Serial.println("");
}

void transmitCommand(const String &command) {
  LoRa.beginPacket();
  LoRa.print("CMD:");
  LoRa.print(command);
  LoRa.endPacket();

  Serial.print("TX> CMD:");
  Serial.println(command);
}

void readIncomingPackets() {
  int packetSize = LoRa.parsePacket();
  if (!packetSize) {
    return;
  }

  String message = "";
  while (LoRa.available()) {
    message += (char)LoRa.read();
  }

  Serial.print("RX> ");
  Serial.println(message);
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Ground Station Command Console");
  if (!LoRa.begin(LORA_FREQUENCY_HZ)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  printHelp();
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.length() > 0) {
      if (command == "HELP") {
        printHelp();
      } else {
        transmitCommand(command);
      }
    }
  }

  readIncomingPackets();
}
