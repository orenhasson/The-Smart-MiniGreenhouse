// Libraries included in project
#include <WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// WiFi credentials
const char* ssid = "agrotech";          // Replace with your WiFi SSID
const char* password = "1Afuna2gezer";  // Replace with your WiFi Password

//Pin Configuration
const int LIGHT_PIN = D10;      // Pin for light
const int tempSensorPin = A4;  // Pin for DS18B20 temperature sensor
const int fanPin = D2;         // Pin controlling the fan
const int soilSensorPin = A0;  // Pin for moisture sensor in soil


// Irrigation Parameters
const int humidityThreshold = 35;
const int irrigationDuration = 20000; // 20 seconds
const int checkInterval = 20000;      // 20 seconds between checks


// Set time
const long utcOffsetInSeconds = 3600 * 2;  // UTC +2
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String formattedTime;


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


unsigned long previousMillis = 0; // Stores the last time the action was performed
const unsigned long printInterval = 1800000;  // 30 minutes in milliseconds for intervals between actions

// Temperature threshold for fan control
const float temperatureThreshold = 30.0;  // Set your desired temperature threshold (in °C)

// DS18B20 sensor setup
OneWire oneWire(tempSensorPin);  // Initialize OneWire communication on the given pin
DallasTemperature sensors(&oneWire);  // Initialize DallasTemperature with the OneWire instance


// MQTT broker details
const char* mqtt_server = "192.168.0.102";    // IP address of Home Assistant
const int mqtt_port = 1883;                   // Default MQTT port
const char* mqtt_user = "mqtt-user";          // MQTT username
const char* mqtt_password = "1234";           // MQTT password
const char* mqtt_irrigation_topic = "/greenhouse/outside/irrigation/solenoid2"; // MQTT topic
const char* mqtt_control_topic = "/greenhouse/outside/irrigation/control"; // MQTT control

// MQTT client and WiFi client
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  int attempts = 0;
  while (!client.connected() && attempts < 5) { // Retry 5 times
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      client.subscribe(mqtt_control_topic);
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("MQTT connection failed, attempt ");
      Serial.println(attempts + 1);
      delay(5000); // Wait before retrying
    }
    attempts++;
  }
}

void performIrrigation() {
  client.publish(mqtt_irrigation_topic, "1"); // Send MQTT message to open solenoid
  
  delay(irrigationDuration);
  
  client.publish(mqtt_irrigation_topic, "0"); // Send MQTT message to close solenoid
 
}

void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  
  if (String(topic) == mqtt_control_topic) {
    if (messageTemp == "IRRIGATE") {
      performIrrigation();
    }
  }
}


void PrintTimeAndStatus() {
  if (timeClient.update()) {
    Serial.print("Time now is: ");
    Serial.print(daysOfTheWeek[timeClient.getDay()]);
    Serial.print(" ");
    Serial.println(timeClient.getFormattedTime());
    Serial.print("Light status: ");
    Serial.println(digitalRead(LIGHT_PIN) == LOW ? "ON" : "OFF");
  } else {
    Serial.println("Failed to update time.");
  }
}
void setup() {

  Serial.begin(115200);

  // Set light pin as output
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, HIGH);

  // Set fan control pin as output
  pinMode(fanPin, OUTPUT);
  digitalWrite(fanPin, HIGH);  // Start with fans off

  // Initialize DS18B20 sensor
  sensors.begin();
   
  // Initialize soil-humidity sensor
  pinMode(soilSensorPin, INPUT);
  
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nNodeMcu connected to WiFi: " + String(ssid));
  
  timeClient.begin();
  timeClient.update();
  
  PrintTimeAndStatus();

  // Initialize WiFi
  setup_wifi();

  // Set up MQTT
  client.setServer(mqtt_server, mqtt_port);

  // Ensure the ESP32 starts connected to the MQTT broker
  reconnect();
  
}

void controlLights() {
  int currentHour = (timeClient.getEpochTime() % 86400L) / 3600 + (utcOffsetInSeconds / 3600);
  if (currentHour >= 24) currentHour -= 24;

  if (currentHour >= 8 && currentHour < 12) {
    if (digitalRead(LIGHT_PIN) == HIGH) {
      digitalWrite(LIGHT_PIN, LOW);
      Serial.println("Turning lights ON");
    }
  } else {
    if (digitalRead(LIGHT_PIN) == LOW) {
      digitalWrite(LIGHT_PIN, HIGH);
      Serial.println("Turning lights OFF");
    }
  }
}

void controlFans() {
  sensors.requestTemperatures(); // Request temperature readings
  float temperature = sensors.getTempCByIndex(0); // Get temperature in °C

  if (temperature != DEVICE_DISCONNECTED_C) {
    Serial.println("Temperature: " + String(temperature) + "°C");
  }

    // Control fans based on temperature threshold
    if (!isnan(temperature)) {  // Ensure sensor is working
  if (temperature > temperatureThreshold) {
    digitalWrite(fanPin, LOW);
    Serial.println("Temperature too high. Fans ON.");
  } else {
    digitalWrite(fanPin, HIGH);
    Serial.println("Temperature normal. Fans OFF.");
  }
  } else {
  Serial.println("Error: DS18B20 sensor not connected or not responding.");
  }
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= printInterval) {
    previousMillis = currentMillis;
    
    if (WiFi.status() == WL_CONNECTED) {
      PrintTimeAndStatus();
      controlLights();
    } else {
      static unsigned long lastReconnectAttempt = 0;
      if (millis() - lastReconnectAttempt > 10000) {
        lastReconnectAttempt = millis();
        WiFi.begin(ssid, password);
        Serial.println("Attempting WiFi reconnection...");
      }
    }
  }
  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();

  int soilHumidity = analogRead(soilSensorPin);
  int humidityPercentage = map(soilHumidity, 0, 1600, 0, 100);
  

Serial.print("Soil Humidity: ");
Serial.print(humidityPercentage);
Serial.println("%");
  
  if (humidityPercentage < humidityThreshold) {
    performIrrigation();
  }
  
  delay(checkInterval);

  controlFans();  // Control fans based on the temperature

  delay(1000);  // Delay between sensor readings (1 second)
}
