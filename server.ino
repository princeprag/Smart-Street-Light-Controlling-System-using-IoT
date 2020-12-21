#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <ESPDateTime.h>


#define FIREBASE_HOST "**********"
#define FIREBASE_AUTH "***************"
char serverr[] = "api.thingspeak.com";

#define D6 12
#define D5 14
#define led0 16 //D0
#define led1 5 //D1
#define led2 4 //D2
#define led3 0 //D3

int client1_LDR_READING = 0;
int client1_PIR_READING = 0;
int client1_BULB_READING = 0;
int client1_BULB_STATE = 0;
int client1_BULB_STATUS;

int client2_LDR_READING = 0;
int client2_PIR_READING = 0;
int client2_BULB_READING = 0;
int client2_BULB_STATE = 0;
int client2_BULB_STATUS;

DynamicJsonBuffer jsonBuffer, jjsonBuffer;
SoftwareSerial s(D6, D5);

const char * ssid = "NODEMCU_WIFI";
const char * password = "NODEMCU_PASSWORD";

const String apiKey = "**********";
const String sendNumber = "************";

const char * host = "maker.ifttt.com";
const int httpsPort = 443;

String sensor_values, bulbs_values;

ESP8266WebServer server(80);

void setupDateTime() {
  DateTime.setTimeZone(5);

  DateTime.begin();
  if (!DateTime.isTimeValid()) {
    Serial.println("Failed to get time from server.");
    return;
  }
}

// FUNCION FOR LISTENING TO HTTP REQUEST FROM CLIENT 1 AND SENDING THE RESPONSE
void handleSentVarClient1() {

  if (server.hasArg("sensor_reading")) {
    sensor_values = server.arg("sensor_reading");

  }

  JsonObject & root = jsonBuffer.parseObject(sensor_values);

  String senttime;

  if (root.success()) {
    client1_LDR_READING = root["LDR_READING"].as < int > ();
    client1_PIR_READING = root["PIR_READING"].as < int > ();
    client1_BULB_READING = root["BULB_READING"].as < int > ();
    client1_BULB_STATE = root["BULB_STATE"].as < int > ();
    //  senttime= root["TIME"].as<String>();

    Serial.println(" ");

    Serial.print("Client1 LDR Reading: ");
    Serial.println(client1_LDR_READING);

    Serial.print("Client1 PIR Reading: ");
    Serial.println(client1_PIR_READING);

    Serial.print("Client1 BULB Reading:  ");
    Serial.println(client1_BULB_READING);

    Serial.print("Client1 BULB STATE:  ");
    Serial.println(client1_BULB_STATE);

    Serial.println("Received data from client 1 successfully");

    //  SensorReadings s1=new SensorReadings(client1_LDR_READING,client1_PIR_READING,client1_BULB_READING,client1_BULB_STATE);

    Firebase.setInt("/SENSOR_READINGS/Client_1/ldr_READING", client1_LDR_READING);
    Firebase.setInt("/SENSOR_READINGS/Client_1/pir_READING", client1_PIR_READING);
    Firebase.setInt("/SENSOR_READINGS/Client_1/bulb_READING", client1_BULB_READING);
    Firebase.setInt("/SENSOR_READINGS/Client_1/bulb_STATE", client1_BULB_STATE);

    Serial.println(" ");

  } else {
    Serial.println(" ");
    Serial.println("Failure in getting data from client 1");
    Serial.println(" ");
  }

  

}


// FUNCTION FOR RECEIVING CLIENT 1 PERIODIC CHECK DATA BY LISTENING TO HTTP REQUEST AND SENDING THE SMS
void handleClient1Check() {

  Serial.println(" ");
  Serial.println("CHECKING CLIENT 1 STATUS:");

  if (server.hasArg("bulb_status")) {
    bulbs_values = server.arg("bulb_status");

  }

  JsonObject & root2 = jsonBuffer.parseObject(bulbs_values);

  String p;
  if (root2.success()) {
    client1_BULB_STATUS = root2["CLIENT1_BULB_STATUS"].as < int > ();

  }

  if (client1_BULB_STATUS == 0) {

    p = "Bad!! NOT Functioning Properly!";
    Serial.println("Bad!! NOT Functioning Properly!");
    //SMS SENDER
    sendSMS(sendNumber, URLEncode("Hi, Authority!!  CLIENT 1: There is a failure detected in Client1 Blub! Thanks!!"));
    sendMessageC1();

  } else {

    p = "All Good!! Functioning Properly!";
    Serial.println("All Good!! Functioning Properly!");
    sendSMS(sendNumber, URLEncode("Hi, Authority!! CLIENT 1: All Good!! Functioning Properly!"));
    

  }
  Serial.println(" ");

  Firebase.setString("/BULB_STATUS/Client_1/" + DateTime.toString(), DateTime.toString() + " : " + p);

}

// Function for sending the SMS to Mobile Number
void sendSMS(String number, String message) {

  WiFiClient client;

  if (client.connect(serverr, 80)) {

    //should look like this...
    //https://api.thingspeak.com/apps/thinghttp/send_request?api_key=RWN78RXG18O6OBM2&number=+919661268236&message=hello

    String urrl = "/apps/thinghttp/send_request?api_key=";
    urrl += apiKey;
    urrl += "&number=";
    urrl += number;
    urrl += "&message=";
    urrl += message;

    client.print(String("GET ") + urrl + " HTTP/1.1\r\n" + "Host: " + serverr + "\r\n" + "Connection: close\r\n\r\n");

  } else {
    Serial.println(F("Connection failed"));
  }

  // Check for a response from the server, and route it
  // out the serial port.
  int p = millis() / 1000;
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      Serial.print(c);
    } else break;

    if (millis() - p * 1000 > 5000) break;

  }
  Serial.println();
  client.stop();

}

String URLEncode(const char * msg) {
  const char * hex = "0123456789abcdef";
  String encodedMsg = "";

  while ( * msg != '\0') {
    if (('a' <= * msg && * msg <= 'z') ||
      ('A' <= * msg && * msg <= 'Z') ||
      ('0' <= * msg && * msg <= '9')) {
      encodedMsg += * msg;
    } else {
      encodedMsg += '%';
      encodedMsg += hex[ * msg >> 4];
      encodedMsg += hex[ * msg & 15];
    }
    msg++;
  }
  return encodedMsg;
}


// Function to send data to webpage
void handle_OnConnect() // sending to webpage
{

  server.send(200, "text/html", SendHTML(client1_BULB_STATUS, client2_BULB_STATUS, client1_LDR_READING, client1_PIR_READING, client1_BULB_READING, client1_BULB_STATE, client2_LDR_READING, client2_PIR_READING, client2_BULB_READING, client2_BULB_STATE));
  Serial.println(" ");
  Serial.println("Webpage sent successfully");
  Serial.println(" ");

}

String SendHTML(int client1_BULB_STATUS, int client2_BULB_STATUS, uint8_t client1_LDR_READING, uint8_t client1_PIR_READING, uint8_t client1_BULB_READING, uint8_t led1state, uint8_t client2_LDR_READING, uint8_t client2_PIR_READING, uint8_t client2_BULB_READING, uint8_t led2state) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>LED Control</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #1abc9c;}\n";
  ptr += ".button-on:active {background-color: #16a085;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>ESP8266 Web Server</h1>\n";
  ptr += "<h3>Using Access Point(AP) Mode</h3>\n";

  ptr += "<h3>Client 1 BULB STATUS: </h3>";
  ptr += String(client1_BULB_STATUS);

  ptr += "<h3>Client 2 BULB STATUS: </h3>";
  ptr += String(client2_BULB_STATUS);

  ptr += "<h3>Client 1 LDR Reading: </h3>";
  ptr += String(client1_LDR_READING);

  ptr += "<h3>Client 1 PIR Reading: </h3>";
  ptr += String(client1_PIR_READING);

  ptr += "<h3>Client 1 BULB Reading: </h3>";
  ptr += String(client1_BULB_READING);

  if (led1state) {
    ptr += "<h3>LED1 Status: ON</h3>";
  }
  //<a class=\"button button-off\" href=\"/led1off\">OFF</a>\n";
  else {
    ptr += "<h3>LED1 Status: OFF</h3>";
  }
  //<a class=\"button button-on\" href=\"/led1on\">ON</a>\n";

  ptr += "<h3>Client 2 LDR Reading: </h3>";
  ptr += String(client2_LDR_READING);

  ptr += "<h3>Client 2 PIR Reading: </h3>";
  ptr += String(client2_PIR_READING);

  ptr += "<h3>Client 2 BULB Reading: </h3>";
  ptr += String(client2_BULB_READING);

  if (led2state) {
    ptr += "<h3>LED2 Status: ON</h3>";
  }
  //<a class=\"button button-off\" href=\"/led2off\">OFF</a>\n";
  else {
    ptr += "<h3>LED2 Status: OFF</h3>";
  }
  //<a class=\"button button-on\" href=\"/led2on\">ON</a>\n";

  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;

}


// Function to send notification for client 2 with IFTTT app
void sendMessageC2() {

  WiFiClientSecure client;
  client.setInsecure();
  Serial.print("connecting to ");
  Serial.println(host);
  int tt = millis();
  while (!client.connect(host, httpsPort)) {
    Serial.println("trying to connect");
    if ((millis() - tt) > 5000) break;
  }

  //https://maker.ifttt.com/trigger/client2_Failure/with/key/dYrNNBBU8XQ-jB1svMQse0zFEkxU080nnu5xf6lDgJ4
  String url = "/trigger/client2_Failure/with/key/dYrNNBBU8XQ-jB1svMQse0zFEkxU080nnu5xf6lDgJ4";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "User-Agent: BuildFailureDetectorESP8266\r\n" +
    "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');

  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");

  delay(10000);

  return;
}

// Function to send notification for client 1 with IFTTT app
void sendMessageC1() {

  WiFiClientSecure client;
  client.setInsecure();
  Serial.print("connecting to ");
  Serial.println(host);
  int tt = millis();
  while (!client.connect(host, httpsPort)) {
    Serial.println("trying to connect");
    if ((millis() - tt) > 5000) break;
  }

  String url = "/trigger/client1_Failure/with/key/dYrNNBBU8XQ-jB1svMQse0zFEkxU080nnu5xf6lDgJ4";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "User-Agent: BuildFailureDetectorESP8266\r\n" +
    "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');

  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");

  delay(10000);

  return;

}

/////////////////////////////////////////////////////////////////////////////////////
//SETUP STARTS HERE
////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  s.begin(9600);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);
  WiFi.begin("******", "********");
  IPAddress myIP = WiFi.softAPIP();

  Serial.println(WiFi.localIP());
  Serial.println(WiFi.softAPIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  pinMode(led0, OUTPUT);
  pinMode(led1, OUTPUT);

  server.on("/data/", HTTP_GET, handleSentVarClient1); // when the server receives a request with /data/ in the string then run the handleSentVar function
  server.on("/display/", HTTP_GET, handle_OnConnect);
  server.on("/checking/", HTTP_GET, handleClient1Check);
  server.begin();

}


/////////////////////////////////////////////////////////////////////////////////////
//LOOP STARTS HERE
////////////////////////////////////////////////////////////////////////////////////

void loop() {

  setupDateTime();
  Serial.println("STARTING THE LOOP: " + DateTime.toString());
  Serial.println(" ");

  server.handleClient(); // listen to HTTP request from the client 1

  String msg = s.readString();
  if (msg[0] == '*')

  {

    Serial.println("CHECKING CLIENT 2 STATUS:");
    int lt = msg.length();
    client2_BULB_STATUS = msg.substring(1, lt - 1).toInt();
    String p;

    if (client2_BULB_STATUS == 0) {
      
      p = "Bad!! NOT Functioning Properly!";
      Serial.println("Bad!! NOT Functioning Properly!");
      sendSMS(sendNumber, URLEncode("Hi, Authority!!  CLIENT 2: There is a failure detected in Client2 Blub! Thanks!!"));
      sendMessageC2();

    } else {

      p = "All Good!! Functioning Properly!";
      Serial.println("All Good!! Functioning Properly!");
      sendSMS(sendNumber, URLEncode("Hi, Authority!!  CLIENT 2: All Good!! Functioning Properly!"));

    }
    Serial.println(" ");

    Firebase.setString("/BULB_STATUS/Client_2/" + DateTime.toString(), DateTime.toString() + " : " + p);

  } else {

    int lth = msg.length();

    String key = "", value = "", s1 = "";
    bool k = true, v = false;
    Serial.println(" ");
    
    for (int i = 0; i < lth; i++) {

      //Serial.print(msg[i]);
      if (msg[i] == '?' && s1.length() > 0 && k) {
        key = s1;
        k = false;
        v = true;
        s1 = "";

      } else if (msg[i] == '?' && s1.length() > 0 && v) {
        value = s1;
        k = true;
        v = false;
        s1 = "";

        if (key == "LDR_READING") client2_LDR_READING = value.toInt();
        else if (key == "PIR_READING") client2_PIR_READING = value.toInt();
        else if (key == "BULB_READING") client2_BULB_READING = value.toInt();
        else if (key == "BULB_STATE") client2_BULB_STATE = value.toInt();

      } else if (k) {
        s1 += msg[i];
      } else if (v) {
        s1 += msg[i];
      }

    }

    Serial.println(" ");
    Serial.print("Client2 LDR Reading: ");
    Serial.println(client2_LDR_READING);

    Serial.print("Client2 PIR Reading: ");
    Serial.println(client2_PIR_READING);

    Serial.print("Client2 BULB Reading:  ");
    Serial.println(client2_BULB_READING);

    Serial.print("Client2 BULB STATE:  ");
    Serial.println(client2_BULB_STATE);

    Firebase.setInt("/SENSOR_READINGS/Client_2/ldr_READING", client2_LDR_READING);
    Firebase.setInt("/SENSOR_READINGS/Client_2/pir_READING", client2_PIR_READING);
    Firebase.setInt("/SENSOR_READINGS/Client_2/bulb_READING", client2_BULB_READING);
    Firebase.setInt("/SENSOR_READINGS/Client_2/bulb_STATE", client2_BULB_STATE);

    Serial.println("Received data from client 2 successfully");
    Serial.println(" ");

    Serial.println("ENDING THE LOOP");
    Serial.println(" ");

  }
  delay(5000);

}
