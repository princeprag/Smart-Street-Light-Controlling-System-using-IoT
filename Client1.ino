//This is the code to be uploaded in Client NodeMCU for TCP-IP Communication with the NodeMCU Server.
//Author-Prince Prag
//Roll No-180102051
//College-Indian Institute of Technology, Guwahati
#include <ESP8266WiFi.h>

#include <ESPDateTime.h>

String str;
int PIR = 5; //d1
int LDR = A0;
int PIR_VALUE = 0;
int LDR_VALUE = 0;
int threshold_value = 300;
int BULB = 4; // PWM PIN d2
int BULB_VALUE = 0;
int BULB_STATE = 0;
long previous = 0;
int threshold = 400;
int day_threshold = 300;

const char * ssid = "NODEMCU_WIFI";
const char * password = "NODEMCU_PASSWORD";

int sensorValue0 = 0;
int sensorValue1 = 0;

void setup() {

  Serial.begin(115200);
  delay(10);

  pinMode(PIR, INPUT);
  pinMode(LDR, INPUT);
  pinMode(BULB, OUTPUT);

  //  This is a client NodeMCU that connects to the server NodeMCU
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

}

void loop() {

  Serial.println("Starting of loop");

  WiFiClient client;
  const char * host = "192.168.4.1"; //default IP address of the server NodeMCU
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    delay(2000);
    return;
  }

  if (millis() - previous * 1000 > 600000) {

    Serial.println("Performing C1 check");
    delay(10000);
    previous = millis() / 1000;
    digitalWrite(BULB, HIGH);
    delay(20000);
    LDR_VALUE = analogRead(LDR);
    String urrl = "/checking/";
    urrl += "?bulb_status=";
    urrl += "{\"CLIENT1_BULB_STATUS\":\"sensor0_value\"}";

    if (LDR_VALUE > threshold) {
      Serial.println("Client 1 Bulb not functioning properly");
      urrl.replace("sensor0_value", "0");
    } else {

      Serial.println("Client 1 Bulb functioning properly");
      urrl.replace("sensor0_value", "1");

    }

    client.print(String("GET ") + urrl + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  } else {

    LDR_VALUE = analogRead(LDR);
    PIR_VALUE = digitalRead(PIR);

    if (PIR_VALUE == HIGH && LDR_VALUE > day_threshold) {
      BULB_VALUE = map(LDR_VALUE, 0, 1023, 0, 255);
      analogWrite(BULB, BULB_VALUE);

    } else {
      BULB_VALUE = 0;
      analogWrite(BULB, LOW);

    }

    BULB_STATE = BULB_VALUE == 0 ? 0 : 1;

    // We now create a URI for the request. Something like /data/?sensor_reading=123, since now my client is connected to the server
    // Basically we send our sensor parameters in the body of the url as a HTTP request.

    String url = "/data/";
    url += "?sensor_reading=";
    url += "{\"LDR_READING\":\"sensor0_value\",\"PIR_READING\":\"sensor1_value\",\"BULB_READING\":\"output_value\",\"BULB_STATE\":\"output_state\"}";

    url.replace("sensor0_value", String(LDR_VALUE));
    url.replace("sensor1_value", String(PIR_VALUE));
    url.replace("output_value", String(BULB_VALUE));
    url.replace("output_state", String(BULB_STATE));

    // This will send the request to the server to get the specific information

    client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
    Serial.println("Sending following data to server");
    Serial.print("Client1 LDR Reading: ");
    Serial.println(LDR_VALUE);

    Serial.print("Client1 PIR Reading: ");
    Serial.println(PIR_VALUE);

    Serial.print("Client1 BULB Reading:  ");
    Serial.println(BULB_VALUE);

    Serial.print("Client1 BULB STATE:  ");
    Serial.println(BULB_STATE);

    Serial.println("Response Received from server: ");
    while (client.connected() && client.available()) {

      String line = client.readStringUntil('\n');
      Serial.println(line);
      delay(1000);
    }

    delay(2000);
    client.stop();
    Serial.println("");

  }

}
