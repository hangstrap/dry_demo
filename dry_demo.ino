#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "Dry_Living_Logo.c"

#include <Arduino_MKRENV.h>

// pin definitions for Arduino MKR Boards
#define TFT_CS A3
#define TFT_DC 0
#define TFT_MOSI 8
#define TFT_CLK 9
#define TFT_MISO 10
#define TFT_LED A2
#define HAVE_TOUCHPAD
#define TOUCH_CS A4
#define TOUCH_IRQ 1

#define BEEPER 2
#define RELAY A0  // optional relay output
/*____Calibrate Touchscreen_____*/
#define MINPRESSURE 10      // minimum required force for touch event
#define TS_MINX 370
#define TS_MINY 470
#define TS_MAXX 3700
#define TS_MAXY 3600

// enable this line for AZ-Touch MKR Version 02.xx
#define touch_yellow_header_28_inch
/*______End of Calibration______*/

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046_Touchscreen touch(TOUCH_CS);

void setup() {
   Serial.begin(115200); //Use serial monitor for debugging
  delay(1000);


  if (!ENV.begin()) {
    Serial.println("Failed to initialize MKR ENV Shield!");
    while (1);
  }

  pinMode(TFT_LED, OUTPUT); // define as output for backlight control

  Serial.println("Init TFT and Touch...");
  tft.begin();
  touch.begin();
  Serial.print("tftx ="); Serial.print(tft.width()); Serial.print(" tfty ="); Serial.println(tft.height());
  tft.fillScreen(ILI9341_BLACK);
  
  IntroScreen();
  displayEnv();
  digitalWrite(TFT_LED, LOW);    // LOW to turn backlight on; 


}

void loop() {
    delay(1000);
    displayEnv();
}

void IntroScreen()
{
  //Draw the Result Box
  tft.fillRect(0, 0, 240, 320, ILI9341_WHITE);
  tft.drawRGBBitmap(50,10,image_data_DryLivingLogo,150,158);

  tft.setTextSize(0);
  tft.setTextColor(ILI9341_BLACK);
  tft.setFont(&FreeSansBold9pt7b);  

  tft.setCursor(42, 190);
  tft.println("Humidity Controller");  

}
void displayEnv(){
  int xCursor = 10;
  int yCursor = 251;
  int yIncrement = 25;
  float temperature = ENV.readTemperature();
  float humidity    = ENV.readHumidity();
  float pressure    = ENV.readPressure();

  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity    = ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Pressure    = ");
  Serial.print(pressure);
  Serial.println(" kPa");

  tft.setTextColor(ILI9341_RED);

  tft.setCursor(xCursor, yCursor);
  tft.print("Temperature = ");
  tft.print(temperature);
  tft.println(" °C");
  
  tft.setCursor(xCursor, yCursor += yIncrement);
  tft.print("Humidity    = ");
  tft.print(humidity);
  tft.println(" %");


  tft.setCursor(xCursor, yCursor += yIncrement);
  tft.print("Pressure    = ");
  tft.print(pressure);
  tft.println(" kPa");

}


