#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>

//Default Config: Change to suit environment
#define LED_PIN 15 
#define NUM_LEDS 8
#define LED_TYPE NEOPIXEL
#define BRIGHTNESS 80

const char* ssid = "WIFI_NAME"; // Set your desired wireless name
const char* password = "WIFI_PASSWORD"; // Set your desired wireless password
const char* deviceUsername = "admin"; // set username for HTML & REST
const char* devicePassword = "admin"; // set password for HTML & REST

IPAddress staticIP(192, 168, 0, 5);  // Set your desired static IP address
IPAddress gateway(192, 168, 0, 1);     // Set your network gateway address
IPAddress subnet(255, 255, 255, 0);    // Set your network subnet mask

CRGB leds[NUM_LEDS];
uint8_t brightness = BRIGHTNESS;  // Initial brightness level
CRGB selectedColor = CRGB::White;  // Initial color

WebServer server(80);

bool isLightningActive = false;
bool isAuroraActive = false;

void handleRoot() {
  if (!server.authenticate(deviceUsername, devicePassword)) {
    return server.requestAuthentication();
  }
  String message = "<html><body><h1>LED Control</h1>";
  message += "<p>Click <a href=\"/on\">here</a> to turn the LED on</p>";
  message += "<p>Click <a href=\"/off\">here</a> to turn the LED off</p>";
  message += "<p>Set color:</p>";
  message += "<input type=\"color\" id=\"colorPicker\" onchange=\"setColor()\" value=\"" + String(selectedColor.r, HEX) + String(selectedColor.g, HEX) + String(selectedColor.b, HEX) + "\">";
  message += "<p>Set brightness:</p>";
  message += "<form action=\"/brightness\" method=\"POST\">";
  message += "<input type=\"range\" name=\"brightness\" min=\"1\" max=\"100\" value=\"" + String(brightness) + "\" onchange=\"setBrightness(value)\">";
  message += "<input type=\"submit\" value=\"Set\">";
  message += "</form>";
  message += "<p>Lightning effect:</p>";
  message += "<form action=\"/lightning\" method=\"POST\">";
  message += "<input type=\"hidden\" name=\"toggle\" value=\"" + String(isLightningActive ? "true" : "false") + "\">";
  message += "<input type=\"submit\" value=\"" + String(isLightningActive ? "Stop" : "Start") + " Lightning\">";
  message += "</form>";
  message += "<p>Aurora effect:</p>";
  message += "<form action=\"/aurora\" method=\"POST\">";
  message += "<input type=\"hidden\" name=\"toggle\" value=\"" + String(isAuroraActive ? "true" : "false") + "\">";
  message += "<input type=\"submit\" value=\"" + String(isAuroraActive ? "Stop" : "Start") + " Aurora\">";
  message += "</form>";
  message += "<script>";
  message += "function setColor() {";
  message += "  var color = document.getElementById('colorPicker').value;";
  message += "  var xhttp = new XMLHttpRequest();";
  message += "  xhttp.open('POST', '/color', true);";
  message += "  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');";
  message += "  xhttp.send('color=' + encodeURIComponent(color));";
  message += "}";
  message += "function setBrightness(value) {";
  message += "  var xhttp = new XMLHttpRequest();";
  message += "  xhttp.open('POST', '/brightness', true);";
  message += "  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');";
  message += "  xhttp.send('brightness=' + encodeURIComponent(value));";
  message += "}";
  message += "</script>";
  message += "</body></html>";
  server.send(200, "text/html", message);
}

void handleOn() {
  if (!server.authenticate(deviceUsername, devicePassword)) {
    return server.requestAuthentication();
  }
  FastLED.setBrightness(brightness);
  FastLED.showColor(selectedColor);
  server.send(200, "text/plain", "LED turned on");
}

void handleOff() {
  if (!server.authenticate(deviceUsername, devicePassword)) {
    return server.requestAuthentication();
  }
  FastLED.clear();
  FastLED.show();
  server.send(200, "text/plain", "LED turned off");
}

void handleColor() {
  if (!server.authenticate(deviceUsername, devicePassword)) {
    return server.requestAuthentication();
  }
  if (server.method() == HTTP_POST) {
    String color = server.arg("color");

    // Extract the individual color components
    int red = strtol(color.substring(1, 3).c_str(), NULL, 16);
    int green = strtol(color.substring(3, 5).c_str(), NULL, 16);
    int blue = strtol(color.substring(5, 7).c_str(), NULL, 16);

    selectedColor = CRGB(red, green, blue);

    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = selectedColor;
    }
    FastLED.show();
  }
  server.send(200, "text/plain", "Color set");
}

void handleBrightness() {
  if (!server.authenticate(deviceUsername, devicePassword)) {
    return server.requestAuthentication();
  }
  if (server.method() == HTTP_POST) {
    brightness = server.arg("brightness").toInt();
    FastLED.setBrightness(brightness);
    handleOn();
  }
  server.send(200, "text/plain", "Brightness set to: " + String(brightness));
}

void handleLightning() {
  if (!server.authenticate(deviceUsername, devicePassword)) {
    return server.requestAuthentication();
  }
  String toggleValue = server.arg("toggle");
  if (toggleValue == "true") {
    isLightningActive = false;
    FastLED.clear();
    FastLED.show();
  } else {
    isLightningActive = true;
  }
  server.sendHeader("Location", "/");
  server.send(302);
}

void handleAurora() {
  if (!server.authenticate(deviceUsername, devicePassword)) {
    return server.requestAuthentication();
  }
  String toggleValue = server.arg("toggle");
  if (toggleValue == "true") {
    isAuroraActive = false;
    FastLED.clear();
    FastLED.show();
  } else {
    isAuroraActive = true;
  }
  server.sendHeader("Location", "/");
  server.send(302);
}

void performLightningEffect() {
  // Perform the lightning effect here
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
  }
  FastLED.show();
  delay(50);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  delay(random(20, 200));
}

void performAuroraEffect() {
  // Perform the aurora effect here
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(random(0, 255), 255, brightness);
  }
  FastLED.show();
  delay(50);
}

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN>(leds, NUM_LEDS);

  Serial.begin(115200);
  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/color", handleColor); // Add the color handler
  server.on("/brightness", handleBrightness);
  server.on("/lightning", handleLightning);
  server.on("/aurora", handleAurora);
  server.begin();
  Serial.println("HTTP server started");

  Serial.print("Static IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();

  if (isLightningActive) {
    performLightningEffect();
  }

  if (isAuroraActive) {
    performAuroraEffect();
  }
}
