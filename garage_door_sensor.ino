
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include "secrets.h"

// defines pins numbers
const int trigPin = 2;  //D4
const int echoPin = 0;  //D3

// defines variables
long duration;
int distance;

void blink(int onDuration, int offDuration, int total) {
  int timer = 0;
  while(timer < total) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(onDuration);
    digitalWrite(LED_BUILTIN, HIGH);

    timer += onDuration;
    
    if(timer + offDuration <= total) {
      delay(offDuration);
    }

    timer += offDuration;
  }
}

void setup() {
  Serial.begin(115200); // Starts the serial communication
  delay(10);

  pinMode(LED_BUILTIN, OUTPUT);

  // Connect to WAP
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    blink(100, 100, 500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  

  // Set time via NTP, as required for x.509 validation
  configTime(0 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    blink(200, 100, 500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.println("Setup complete");
  blink(50, 30, 200);
}

void loop() {
  Serial.println("Measuring...");
  // Clears the trigPin
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(2);
  
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, LOW);
  delayMicroseconds(10);
  digitalWrite(trigPin, HIGH);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculating the distance (cm)
  distance= duration*0.034/2;

  char distanceStr[33];
  itoa(distance, distanceStr, 10);
  
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distanceStr);


  const char * host = "p0z5aenv1f.execute-api.us-east-1.amazonaws.com";
  const int httpsPort = 443;
  
  WiFiClientSecure client;
  Serial.print("Connecting to ");
  Serial.println(host);
 
  client.setInsecure();
  
  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    return;
  }

  String url = "/dev/door";
  Serial.print("Requesting URL: ");
  Serial.println(url);

  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "X-API-Key: " + apiKey + "\r\n" +
               "doorstatus: " + distanceStr + "\r\n" +
               "Content-Type: application/json\r\n" +
               "User-Agent: ESP8266Tester\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("Request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.print("Got line: ");
    Serial.println(line);
    if (line == "\r") {
      Serial.println("Headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  Serial.println("Reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("Closing connection");  
  
  delay(60000);

  
}
