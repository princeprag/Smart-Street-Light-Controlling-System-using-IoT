//This is the code to be uploaded in Arduino UNO for Rx-Tx Communication with the NodeMCU Server.
//Author-Prince Prag
//Roll No-180102051
//College-Indian Institute of Technology, Guwahati

#include <ArduinoJson.h>

String str;
int PIR = 4;
int LDR = A0;
int PIR_VALUE = 0;
int LDR_VALUE = 0;
int threshold_value = 300;
int BULB = 8;
int BULB_VALUE = 0;
long previous = 0;
int threshold = 400;
int day_threshold = 300;

void setup() {
  Serial.begin(9600);
  pinMode(PIR, INPUT);
  pinMode(LDR, INPUT);
  pinMode(BULB, OUTPUT);
  pinMode(ipt, INPUT);
  pinMode(opt, OUTPUT);
  pinMode(kk, INPUT);

}

void loop() {

  if (millis() - (previous * 1000) > 60000) {
    // start periodic checking of streetlights
    previous = millis() / 1000;
    digitalWrite(BULB, HIGH);
    delay(10000);

    LDR_VALUE = analogRead(LDR);
    String sr;
    if (LDR_VALUE > threshold) {
      sr = "0";
    } else sr = "1";

    String msgg = "*" + sr;
    // sending data about periodic check to the server using the key="*" at the start of the string
    Serial.println(msgg);
  } else {

    LDR_VALUE = analogRead(LDR);
    PIR_VALUE = digitalRead(PIR);

    if (PIR_VALUE == HIGH && LDR_VALUE > day_threshold) {
      BULB_VALUE = map(LDR_VALUE, 0, 1023, 0, 255);
      analogWrite(BULB, BULB_VALUE);

    } else {
      BULB_VALUE = 0;
      analogWrite(BULB, BULB_VALUE);

    }

    String sss = (BULB_VALUE == 0) ? "0" : "1";
    String msg = "LDR_READING?" + String(LDR_VALUE) + "?PIR_READING?" + String(PIR_VALUE) + "?BULB_READING?" + String(BULB_VALUE) + "?BULB_STATE?" + sss + "?";

    //sending data about the sensors readings and bulb status to the server NodeMCU
    Serial.println(msg);

  }

  delay(5000);

}
