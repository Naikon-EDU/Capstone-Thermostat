#include <WiFi.h>                      // For WiFi connectivity
#include <Adafruit_GFX.h>              // OLED graphics library
#include <Adafruit_ILI9341.h>          // Giga Display Shield library
#include <DHT.h>                       // Grove Temp & Humidity Sensor library

// WiFi credentials
const char* ssid = "Wifi";         // Replace with your WiFi SSID
const char* password = "Wifi pass"; // Replace with your WiFi password

// DHT Sensor settings
#define DHTPIN A0
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Display settings
#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

// Relay pins
#define HEAT_RELAY_PIN 6
#define COOL_RELAY_PIN 7
#define FAN_RELAY_PIN 8

// WiFi server
WiFiServer server(80);  // HTTP server on port 80

// Variables
float actualTemp = 0.0;
float actualHumidity = 0.0;
int setTemp = 72;         // Default set temperature
bool isHeatingMode = true;  // True = Heating, False = Cooling
bool isFanOn = false;       // True = Fan ON, False = Fan AUTO

void setup() {
  Serial.begin(115200);

  // Initialize relays
  pinMode(HEAT_RELAY_PIN, OUTPUT);
  pinMode(COOL_RELAY_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);

  // Initialize DHT sensor
  dht.begin();

  // Initialize display
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Start the server
  server.begin();
}

void loop() {
  // Read temperature and humidity
  actualTemp = dht.readTemperature(true);  // Fahrenheit
  actualHumidity = dht.readHumidity();

  // Handle web client connections
  WiFiClient client = server.available();
  if (client) {
    handleClient(client);
  }

  // Update display
  updateDisplay();

  // Control heating, cooling, and fan based on the set temperature and mode
  controlHVAC();

  delay(500);
}

void handleClient(WiFiClient client) {
  String request = client.readStringUntil('\r');
  client.flush();

  // Respond to the client
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  client.println("<!DOCTYPE html>");
  client.println("<html>");
  client.println("<head><title>Thermostat</title></head>");
  client.println("<body>");
  client.println("<h1>Interactive Thermostat</h1>");
  client.println("<p>Current Temp: " + String(actualTemp) + " F</p>");
  client.println("<p>Humidity: " + String(actualHumidity) + " %</p>");
  client.println("<p>Set Temp: " + String(setTemp) + " F</p>");
  client.println("<p>Mode: " + String(isHeatingMode ? "Heating" : "Cooling") + "</p>");
  client.println("<p>Fan: " + String(isFanOn ? "ON" : "AUTO") + "</p>");
  client.println("</body></html>");
  client.stop();
}

void updateDisplay() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);

  tft.setCursor(10, 10);
  tft.println("Thermostat");

  tft.setTextSize(3);
  tft.setCursor(10, 50);
  tft.print("Temp: ");
  tft.print(actualTemp);
  tft.println(" F");

  tft.setTextSize(2);
  tft.setCursor(10, 100);
  tft.print("Set: ");
  tft.print(setTemp);
  tft.println(" F");

  tft.setCursor(10, 150);
  tft.print("Humidity: ");
  tft.print(actualHumidity);
  tft.println(" %");

  tft.setCursor(10, 200);
  tft.print("Mode: ");
  tft.println(isHeatingMode ? "Heating" : "Cooling");

  tft.setCursor(10, 250);
  tft.print("Fan: ");
  tft.println(isFanOn ? "ON" : "AUTO");
}

void controlHVAC() {
  if (isHeatingMode) {
    if (actualTemp < setTemp) {
      digitalWrite(HEAT_RELAY_PIN, HIGH); // Turn on heat
      digitalWrite(COOL_RELAY_PIN, LOW); // Turn off cooling
      digitalWrite(FAN_RELAY_PIN, HIGH); // Turn on fan
    } else {
      digitalWrite(HEAT_RELAY_PIN, LOW); // Turn off heat
      if (!isFanOn) digitalWrite(FAN_RELAY_PIN, LOW); // Turn off fan if in auto mode
    }
  } else {
    if (actualTemp > setTemp) {
      digitalWrite(COOL_RELAY_PIN, HIGH); // Turn on cooling
      digitalWrite(HEAT_RELAY_PIN, LOW);  // Turn off heating
      digitalWrite(FAN_RELAY_PIN, HIGH); // Turn on fan
    } else {
      digitalWrite(COOL_RELAY_PIN, LOW); // Turn off cooling
      if (!isFanOn) digitalWrite(FAN_RELAY_PIN, LOW); // Turn off fan if in auto mode
    }
  }
}
