#include <M5Unified.h>
#include <M5GFX.h>
#include <Wire.h>
#include <VL53L1X.h>

// Fonts
#include "NotoSansBold15.h"
#include "tinyFont.h"
#include "smallFont.h"
#include "middleFont.h"
#include "bigFont.h"
#include "font18.h"

// Button dimensions 
const int BUTTON_WIDTH = 220;
const int BUTTON_HEIGHT = 80;
const int BUTTON_RADIUS = 6;

// Button positions - evenly spaced near top middle
const int BUTTON_Y = 18;
const int BUTTON_BOTTOM_Y = 620; // Position for bottom buttons
const int BUTTON_SPACING = 40;
const int TOTAL_BUTTONS_WIDTH = (3 * BUTTON_WIDTH) + (2 * BUTTON_SPACING);
const int BUTTON_START_X = (1280 - TOTAL_BUTTONS_WIDTH) / 2;

// Button coordinates
const int BUTTON1_X = BUTTON_START_X;
const int BUTTON2_X = BUTTON_START_X + BUTTON_WIDTH + BUTTON_SPACING;
const int BUTTON3_X = BUTTON_START_X + 2 * (BUTTON_WIDTH + BUTTON_SPACING);

// Pre-Defined Colours - It uses the slightly odd "16-bit RGB565 color format" 
// https://rgbcolorpicker.com/565
const unsigned short PINK_COLOR = 0xb9f2;  // Pink-ish or RGB (23,15,18) 
const unsigned short BLUE_COLOR = 0x3af8;  // Blue-ish or RGB (7,23,24)
const unsigned short GREEN_COLOR = 0x4607;  // Green-ish or RGB (8,48,7)
const unsigned short YELLOW_COLOR = 0xefa1;  // Yellow-ish or RGB (29,61,1)
const unsigned short RED_COLOR = 0xe904;  // Red-ish or RGB (29,8,4)
const unsigned short WHITE_COLOR = 0xffff;  // White or RGB (31,63,31)
const unsigned short ORANGE_COLOR = 0xfde1;  // Orange-ish or RGB (31,47,1)
const unsigned short BLACK_COLOR = 0x0000;  // Black or RGB (0,0,0)
const unsigned short BACKGROUND_COLOR = 0x10E4; // Dark Blue/Black or RGB (2,7,4)
const unsigned short BACKGROUND_COLOR_BLACK = 0x0000;  // Black or RGB (0,0,0)

// ToF Sensor
VL53L1X sensor;
bool tofInitialized = false;
unsigned long lastSensorRead = 0;
const unsigned long SENSOR_READ_INTERVAL = 100; // Read sensor every 100ms

int brightness = 125;
bool deb = 0;
String currentRangeMode = "Long"; // Track current range mode

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  
  M5.Display.setRotation(3);
  Serial.begin(115200);

  M5.Speaker.setVolume(128);

  // Initialize ToF sensor on Port A (GPIO53=SDA, GPIO54=SCL)
  Wire.begin(53, 54, 400000); // SDA, SCL, frequency
  sensor.setBus(&Wire);
  sensor.setTimeout(500);
  
  if (!sensor.init()) {
    Serial.println("Failed to detect and initialize ToF sensor!");
    tofInitialized = false;
  } else {
    Serial.println("ToF sensor initialized successfully!");
    tofInitialized = true;
    
    // Configure sensor for long distance mode (default)
    sensor.setDistanceMode(VL53L1X::Long);
    sensor.setMeasurementTimingBudget(50000);
    
    // Start continuous readings
    sensor.startContinuous(50);
  }

  // Draw UI
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.fillScreen(BACKGROUND_COLOR_BLACK);
  drawButtons();
  
  // Display sensor status
  displaySensorStatus();
  // Display current range mode
  displayRangeMode();
}

void drawButtons() {
  M5.Display.loadFont(bigFont);
  
  // Top Row Buttons
  // Button 1 - String 1
  M5.Display.fillRoundRect(BUTTON1_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_RADIUS, ORANGE_COLOR);
  M5.Display.setTextColor(BLACK_COLOR, ORANGE_COLOR);
  M5.Display.setTextDatum(middle_center);
  M5.Display.drawString("String 1", BUTTON1_X + BUTTON_WIDTH / 2, BUTTON_Y + BUTTON_HEIGHT / 2);
  
  // Button 2 - String 2
  M5.Display.fillRoundRect(BUTTON2_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_RADIUS, ORANGE_COLOR);
  M5.Display.setTextColor(BLACK_COLOR, ORANGE_COLOR);
  M5.Display.drawString("String 2", BUTTON2_X + BUTTON_WIDTH / 2, BUTTON_Y + BUTTON_HEIGHT / 2);
  
  // Button 3 - String 3
  M5.Display.fillRoundRect(BUTTON3_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_RADIUS, ORANGE_COLOR);
  M5.Display.setTextColor(BLACK_COLOR, ORANGE_COLOR);
  M5.Display.drawString("String 3", BUTTON3_X + BUTTON_WIDTH / 2, BUTTON_Y + BUTTON_HEIGHT / 2);
  
  // Bottom Row Buttons
  // Button 4 - Short
  M5.Display.fillRoundRect(BUTTON1_X, BUTTON_BOTTOM_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_RADIUS, ORANGE_COLOR);
  M5.Display.setTextColor(BLACK_COLOR, ORANGE_COLOR);
  M5.Display.drawString("Short", BUTTON1_X + BUTTON_WIDTH / 2, BUTTON_BOTTOM_Y + BUTTON_HEIGHT / 2);
  
  // Button 5 - Med
  M5.Display.fillRoundRect(BUTTON2_X, BUTTON_BOTTOM_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_RADIUS, ORANGE_COLOR);
  M5.Display.setTextColor(BLACK_COLOR, ORANGE_COLOR);
  M5.Display.drawString("Med", BUTTON2_X + BUTTON_WIDTH / 2, BUTTON_BOTTOM_Y + BUTTON_HEIGHT / 2);
  
  // Button 6 - Long
  M5.Display.fillRoundRect(BUTTON3_X, BUTTON_BOTTOM_Y, BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_RADIUS, ORANGE_COLOR);
  M5.Display.setTextColor(BLACK_COLOR, ORANGE_COLOR);
  M5.Display.drawString("Long", BUTTON3_X + BUTTON_WIDTH / 2, BUTTON_BOTTOM_Y + BUTTON_HEIGHT / 2);
  
  M5.Display.unloadFont();
}

void displaySensorStatus() {
  // Use bigFont for status display
  M5.Display.loadFont(bigFont);
  M5.Display.setTextColor(WHITE_COLOR, BACKGROUND_COLOR_BLACK);
  M5.Display.setTextDatum(top_left);
  
  int statusY = BUTTON_Y + BUTTON_HEIGHT + 30; // Increased spacing
  
  if (tofInitialized) {
    M5.Display.drawString("ToF Sensor: Connected", 10, statusY);
  } else {
    M5.Display.drawString("ToF Sensor: Not Found", 10, statusY);
    M5.Display.setTextColor(RED_COLOR, BACKGROUND_COLOR_BLACK);
    M5.Display.drawString("Check Unit ToF4M on Port A", 10, statusY + 50); // Increased spacing
  }
  M5.Display.unloadFont();
}

void displayRangeMode() {
  // Display current range mode centered above the bottom buttons
  M5.Display.loadFont(bigFont);
  M5.Display.setTextColor(GREEN_COLOR, BACKGROUND_COLOR_BLACK);
  M5.Display.setTextDatum(middle_center);
  
  // Position: centered horizontally, about 60 pixels above button 5
  int rangeModeY = BUTTON_BOTTOM_Y - 60;
  
  // Clear the area where the range mode text will be displayed
  M5.Display.fillRect(0, rangeModeY - 25, 1280, 50, BACKGROUND_COLOR_BLACK);
  
  M5.Display.drawString("Range Mode: " + currentRangeMode, 1280 / 2, rangeModeY);
  M5.Display.unloadFont();
}

void displayLargeDistance(int distance) {
  // Display large distance value in the center of the screen
  M5.Display.loadFont(bigFont);
  M5.Display.setTextColor(WHITE_COLOR, BACKGROUND_COLOR_BLACK);
  M5.Display.setTextDatum(middle_center);
  
  // Center of screen coordinates
  int centerX = 1280 / 2;
  int centerY = 720 / 2;
  
  // Clear the area where the large distance will be displayed
  M5.Display.fillRect(centerX - 200, centerY - 50, 400, 100, BACKGROUND_COLOR_BLACK);
  
  // Convert distance to string and display
  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%d", distance);
  M5.Display.drawString(buffer, centerX, centerY);
  
  M5.Display.unloadFont();
}

void displaySensorData(int distance, const char* status, int signal, int ambient) {
  // Use the same bigFont and style as sensor status
  M5.Display.loadFont(bigFont);
  M5.Display.setTextColor(WHITE_COLOR, BACKGROUND_COLOR_BLACK);
  M5.Display.setTextDatum(top_left);
  
  int dataY = BUTTON_Y + BUTTON_HEIGHT + 100;
  
  // Clear previous data area - increase height to accommodate larger spacing
  M5.Display.fillRect(10, dataY, 1000, 300, BACKGROUND_COLOR_BLACK);
  
  char buffer[100];
  
  // Distance
  M5.Display.setTextColor(WHITE_COLOR, BACKGROUND_COLOR_BLACK);
  snprintf(buffer, sizeof(buffer), "Distance: %d mm", distance);
  M5.Display.drawString(buffer, 10, dataY);
  
  // Status - increased spacing
  snprintf(buffer, sizeof(buffer), "Status: %s", status);
  M5.Display.drawString(buffer, 10, dataY + 70); // Increased from 50 to 70
  
  // Signal strength - increased spacing
  snprintf(buffer, sizeof(buffer), "Signal: %d MCPS", signal);
  M5.Display.drawString(buffer, 10, dataY + 140); // Increased from 100 to 140
  
  // Ambient light - increased spacing
  snprintf(buffer, sizeof(buffer), "Ambient: %d MCPS", ambient);
  M5.Display.drawString(buffer, 10, dataY + 210); // Increased from 150 to 210
  
  M5.Display.unloadFont();
}

void setRangeMode(int mode) {
  if (!tofInitialized) return;
  
  // Stop continuous mode before changing settings
  sensor.stopContinuous();
  
  switch(mode) {
    case 0: // Short
      sensor.setDistanceMode(VL53L1X::Short);
      currentRangeMode = "Short";
      Serial.println("Range mode set to: Short");
      break;
    case 1: // Medium
      sensor.setDistanceMode(VL53L1X::Medium);
      currentRangeMode = "Medium";
      Serial.println("Range mode set to: Medium");
      break;
    case 2: // Long
      sensor.setDistanceMode(VL53L1X::Long);
      currentRangeMode = "Long";
      Serial.println("Range mode set to: Long");
      break;
  }
  
  // Restart continuous mode with new settings
  sensor.startContinuous(50);
  
  // Update display to show new range mode
  displayRangeMode();
}

void readAndDisplaySensor() {
  if (!tofInitialized) return;
  
  sensor.read();
  
  // Display on screen using consistent font and style
  displaySensorData(
    sensor.ranging_data.range_mm,
    VL53L1X::rangeStatusToString(sensor.ranging_data.range_status),
    sensor.ranging_data.peak_signal_count_rate_MCPS,
    sensor.ranging_data.ambient_count_rate_MCPS
  );
  
  // Display large distance value in center
  displayLargeDistance(sensor.ranging_data.range_mm);
  
  // Also print to Serial
  Serial.print("ToF - range: ");
  Serial.print(sensor.ranging_data.range_mm);
  Serial.print(" mm\tstatus: ");
  Serial.print(VL53L1X::rangeStatusToString(sensor.ranging_data.range_status));
  Serial.print("\tpeak signal: ");
  Serial.print(sensor.ranging_data.peak_signal_count_rate_MCPS);
  Serial.print(" MCPS\tambient: ");
  Serial.print(sensor.ranging_data.ambient_count_rate_MCPS);
  Serial.println(" MCPS");
}

void loop() {
  M5.update();
  auto touchDetail = M5.Touch.getDetail();

  // Handle touch events
  if (touchDetail.isPressed()) {
    // Button 1 - Send Test Message 1
    if (touchDetail.x > BUTTON1_X && touchDetail.x < BUTTON1_X + BUTTON_WIDTH && 
        touchDetail.y > BUTTON_Y && touchDetail.y < BUTTON_Y + BUTTON_HEIGHT) {
      if (deb == 0) {
        deb = 1;
        Serial.println("Button 1 pressed");
        M5.Speaker.tone(3000, 50);
      }
    }
    
    // Button 2 - Send Test Message 2
    else if (touchDetail.x > BUTTON2_X && touchDetail.x < BUTTON2_X + BUTTON_WIDTH && 
             touchDetail.y > BUTTON_Y && touchDetail.y < BUTTON_Y + BUTTON_HEIGHT) {
      if (deb == 0) {
        deb = 1;
        Serial.println("Button 2 pressed");
        M5.Speaker.tone(3000, 50);
      }
    }
    
    // Button 3 - Send Test Message 3
    else if (touchDetail.x > BUTTON3_X && touchDetail.x < BUTTON3_X + BUTTON_WIDTH && 
             touchDetail.y > BUTTON_Y && touchDetail.y < BUTTON_Y + BUTTON_HEIGHT) {
      if (deb == 0) {
        deb = 1;
        Serial.println("Button 3 pressed");
        M5.Speaker.tone(3000, 50);
      }
    }
    
    // Button 4 - Short Range
    else if (touchDetail.x > BUTTON1_X && touchDetail.x < BUTTON1_X + BUTTON_WIDTH && 
             touchDetail.y > BUTTON_BOTTOM_Y && touchDetail.y < BUTTON_BOTTOM_Y + BUTTON_HEIGHT) {
      if (deb == 0) {
        deb = 1;
        Serial.println("Button 4 (Short Range) pressed");
        setRangeMode(0); // Set to Short range
        M5.Speaker.tone(3000, 50);
      }
    }
    
    // Button 5 - Medium Range
    else if (touchDetail.x > BUTTON2_X && touchDetail.x < BUTTON2_X + BUTTON_WIDTH && 
             touchDetail.y > BUTTON_BOTTOM_Y && touchDetail.y < BUTTON_BOTTOM_Y + BUTTON_HEIGHT) {
      if (deb == 0) {
        deb = 1;
        Serial.println("Button 5 (Medium Range) pressed");
        setRangeMode(1); // Set to Medium range
        M5.Speaker.tone(3000, 50);
      }
    }
    
    // Button 6 - Long Range
    else if (touchDetail.x > BUTTON3_X && touchDetail.x < BUTTON3_X + BUTTON_WIDTH && 
             touchDetail.y > BUTTON_BOTTOM_Y && touchDetail.y < BUTTON_BOTTOM_Y + BUTTON_HEIGHT) {
      if (deb == 0) {
        deb = 1;
        Serial.println("Button 6 (Long Range) pressed");
        setRangeMode(2); // Set to Long range
        M5.Speaker.tone(3000, 50);
      }
    }
  } else {
    deb = 0;
  }

  // Read and display sensor data periodically
  if (millis() - lastSensorRead >= SENSOR_READ_INTERVAL) {
    readAndDisplaySensor();
    lastSensorRead = millis();
  }

  delay(1);
}