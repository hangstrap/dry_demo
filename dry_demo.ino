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
class Environment {
  public:
    int temperature =0;
    int humidity = 0;
    int pressure = 0;
    Environment( int temp, int hum, int pres){
      temperature = temp;
      humidity = hum;
      pressure = pres;
    }
    bool equal( Environment other){
      if( temperature != other.temperature){
        return false;
      }
      if( humidity != other.humidity){
        return false;
      }
      if( pressure != pressure){
        return false;
      }
      return true;
    }
} ;

Environment lastReading(0,0,0);

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

  lastReading = readCurrentValue();
  displayEnv( lastReading);
  digitalWrite(TFT_LED, LOW);    // LOW to turn backlight on; 


}

void loop() {
    delay(1000);
    Environment currentValue = readCurrentValue();
    if( !lastReading.equal( currentValue)){
      lastReading = currentValue;
      displayEnv( lastReading);
    }
}

void IntroScreen()
{
  //Draw the Result Box
  tft.fillRect(0, 0, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, ILI9341_WHITE);
  tft.drawRGBBitmap(50,10,image_data_DryLivingLogo,150,158);

  tft.setTextSize(0);
  tft.setTextColor(ILI9341_BLACK);
  tft.setFont( &FreeSansBold9pt7b);  

  tft.setCursor(42, 190);
  tft.println("Humidity Controller");  

}

Environment readCurrentValue(){
  return  Environment ( ENV.readTemperature(),  ENV.readHumidity(), ENV.readPressure());
}
void printEnv( Environment environment){

  Serial.print("Temperature = ");
  Serial.print( environment.temperature);
  Serial.println(" °C");

  Serial.print("Humidity    = ");
  Serial.print( environment.humidity);
  Serial.println(" %");

  Serial.print("Pressure    = ");
  Serial.print( environment.pressure);
  Serial.println(" kPa");
}

void displayEnv( Environment environment){

  int xCursor = 10;
  int yCursor = 250;
  int yIncrement = 25;

  tft.fillRect(0, yCursor -14, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT-yCursor, ILI9341_ORANGE);

  tft.setTextColor(ILI9341_RED);

  tft.setCursor(xCursor, yCursor);
  tft.print("Temperature = ");
  tft.print( environment.temperature);
  tft.println(" °C");
  
  tft.setCursor(xCursor, yCursor += yIncrement);
  tft.print("Humidity    = ");
  tft.print( environment.humidity);
  tft.println(" %");


  tft.setCursor(xCursor, yCursor += yIncrement);
  tft.print("Pressure    = ");
  tft.print( environment.pressure);
  tft.println(" kPa");

}


