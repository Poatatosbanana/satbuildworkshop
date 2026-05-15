#include <Servo.h>

#define PIN_SERVO_PANEL 5
#define PIN_SUN_SENSOR_LEFT A3
#define PIN_SUN_SENSOR_RIGHT A4

Servo panelServo;

int panelAngle = 90;
String currentMode = "NADIR";

int constrainAngle(int angle) {
  if (angle < 0) return 0;
  if (angle > 180) return 180;
  return angle;
}

void applyPanelAngle(int angle) {
  panelAngle = constrainAngle(angle);
  panelServo.write(panelAngle);
}

String getStatus() {
  int sunLeft = analogRead(PIN_SUN_SENSOR_LEFT);
  int sunRight = analogRead(PIN_SUN_SENSOR_RIGHT);

  String status = "ADCS_STATUS|MODE=" + currentMode;
  status += "|ANGLE=" + String(panelAngle);
  status += "|SUNL=" + String(sunLeft);
  status += "|SUNR=" + String(sunRight);
  return status;
}

String pointSun() {
  int sunLeft = analogRead(PIN_SUN_SENSOR_LEFT);
  int sunRight = analogRead(PIN_SUN_SENSOR_RIGHT);
  int diff = sunLeft - sunRight;

  int correction = map(constrain(diff, -300, 300), -300, 300, -15, 15);
  long requestedAngle = (long)panelAngle + correction;
  applyPanelAngle((int)requestedAngle);
  currentMode = "SUN_TRACK";

  return "ADCS_POINT_SUN=OK|ANGLE=" + String(panelAngle);
}

String pointNadir() {
  applyPanelAngle(90);
  currentMode = "NADIR";
  return "ADCS_POINT_NADIR=OK|ANGLE=90";
}

String setAngleCommand(const String &command) {
  int separatorIndex = command.indexOf(':');
  if (separatorIndex < 0) {
    return "ERR=BAD_ADCS_SET_ANGLE";
  }

  String angleToken = command.substring(separatorIndex + 1);
  angleToken.trim();
  int requestedAngle = angleToken.toInt();

  applyPanelAngle(requestedAngle);
  currentMode = "MANUAL";
  return "ADCS_SET_ANGLE=OK|ANGLE=" + String(panelAngle);
}

String handleAdcsCommand(String command) {
  command.trim();

  if (command == "ADCS_STATUS") {
    return getStatus();
  }
  if (command == "ADCS_POINT_SUN") {
    return pointSun();
  }
  if (command == "ADCS_POINT_NADIR") {
    return pointNadir();
  }
  if (command.startsWith("ADCS_SET_ANGLE:")) {
    return setAngleCommand(command);
  }

  return "ERR=UNKNOWN_ADCS_COMMAND";
}

void setup() {
  Serial.begin(9600);
  panelServo.attach(PIN_SERVO_PANEL);
  applyPanelAngle(panelAngle);
  Serial.println("ADCS Pointing Controller Ready");
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    String response = handleAdcsCommand(command);
    Serial.println(response);
  }
}
