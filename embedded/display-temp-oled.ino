#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_CCS811.h>
#include <WiFi.h>

// OLED display dimensions (commonly 128x64 or 128x32).
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// I2C OLED address (often 0x3C; can be 0x3D on some displays).
#define OLED_ADDRESS 0x3C

// define OLED text colors
#define COLOR_GREEN  ((0 << 11) | (63 << 5) | 0)  // Full green
#define COLOR_YELLOW ((31 << 11) | (63 << 5) | 0) // Full red + full green
#define COLOR_RED    ((31 << 11) | (0 << 5) | 0)  // Full red

// Create display object. 
// The final '-1' indicates there's no reset pin (or you are using software reset).
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Create BMP object
Adafruit_BMP3XX bmp;

// Create CCS811 object.
Adafruit_CCS811 ccs;

const char* ssid = "MatzFamily";
const char* password = "DadMomBoys19932014"
String serverUrl = "http://10.0.0.93:8000/sensor/log";

void setup() {
  // Start serial (optional for debugging).
  Serial.begin(115200);
  delay(1000);

  // Connect to WiFi (ESP32/ESP8266 approach)
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize the OLED display.
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Loop forever if display initialization fails
  }
  display.clearDisplay();
  display.display();

  // Initialize BMP390 sensor over I2C
  if (!bmp.begin_I2C()) {  // i.e. bmp.begin_I2C(BMP3XX_I2CADDR_DEFAULT, &Wire)
    Serial.println(F("Could not find a valid BMP390 sensor, check wiring!"));
    while (1) delay(10);
  }

  // Recommended sensor configuration
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  // Optional: print a message upon successful setup
  Serial.println("BMP390 sensor initialized!");

  // **Initialize the CCS811 sensor.**  
  if (!ccs.begin()) {
    Serial.println("Failed to start CCS811 sensor! Check wiring!");
    while (1);
  }
  // The CCS811 requires 20–30 seconds after power-up to stabilize readings.

  Serial.println("CCS811 sensor initialized!");
}

void loop() {
  // Check if sensor reading is available
  if (!bmp.performReading()) {
    Serial.println(F("Failed to perform reading :("));
    return;
  }

  // Read temperature (C) and pressure (Pa). Convert pressure from Pa to hPa:
  float temperature = bmp.temperature * 9.0 / 5.0 + 32.0;
  float pressure_hPa = bmp.pressure / 100.0;

  // **CCS811 reading: must check .available() first.**
  float co2 = -1, tvoc = -1;  
  if (ccs.available()) {
    if (!ccs.readData()) {
      co2  = ccs.geteCO2();
      tvoc = ccs.getTVOC();
    } else {
      Serial.println("CCS811 read error");
    }
  } else {
    Serial.println("CCS811 not available yet");
  }

  // Print values to Serial (for debugging)
  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" *F");

  Serial.print("Pressure = ");
  Serial.print(pressure_hPa);
  Serial.print(" hPa,  CCS811 eCO2 = ");
  Serial.print(co2);
  Serial.print(" ppm,  TVOC = ");
  Serial.print(tvoc);
  Serial.println(" ppb");

  Serial.println("Sending data to API...");
  // First, check if we're still connected to WiFi
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Begin connection
    http.begin(serverUrl);

    // Tell the server we'll send JSON
    http.addHeader("Content-Type", "application/json");

    // Construct JSON payload (example)
    // Adjust key names to match what your API expects
    String jsonPayload = "{";
    jsonPayload += "\"temperature\":";
    jsonPayload += temperature;
    jsonPayload += ",\"pressure\":";
    jsonPayload += pressure_hPa;
    jsonPayload += ",\"co2\":";
    jsonPayload += co2;
    jsonPayload += ",\"tvoc\":";
    jsonPayload += tvoc;
    jsonPayload += "}";

    // Perform the POST request
    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      // HTTP header has been sent and server response was received
      String response = http.getString(); // Get the response
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("Server response: ");
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    // End the HTTP connection
    http.end();
  } else {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.begin(ssid, password);
  }

  // Display on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // BMP390 lines  
  display.setCursor(0, 0);
  display.print("BMP390 Readings:");

  display.setCursor(0, 20);
  display.setTextSize(2);
  display.print(temperature);
  display.println(" F");
  
  display.setTextSize(1);
  // display.setCursor(0, 40);
  // display.print("Pres: ");
  // display.print(pressure_hPa);
  // display.println(" hPa");

  // CCS811 lines
  display.setCursor(0, 48);
  display.print("CO2 : ");
  display.print(co2);
  display.print(" ppm");
  if (co2 < 501) {
    display.println(" outd");
  } else if (co2 < 1001) {
    display.println(" ind");
  } else if (co2 < 2001) {
    display.println(" sfy");
  } else {
    display.println(" dng");
  }

  
  display.setCursor(0, 56);
  display.print("TVOC: ");
  display.print(tvoc);
  display.print(" ppb");
  if (tvoc < 51) {
    display.println(" good");
  } else if (tvoc < 201) {
    display.println(" ok");
  } else {
    display.println(" bad");
  }

  // Update the display with the new info
  display.display();

  // Delay for readability, adjust as needed
  delay(1000);
}
