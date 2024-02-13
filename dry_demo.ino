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


// enable this line for AZ-Touch MKR Version 02.xx
#define touch_yellow_header_28_inch
/*______End of Calibration______*/

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046_Touchscreen touch(TOUCH_CS);

class Box {
public:
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  Box(int _x, int _y, int _w, int _h) {
    x = _x;
    y = _y;
    w = _w;
    h = _h;
  }
  bool isInside( TS_Point p){
    if( p.x < x ) return false;
    if( p.x > w + x ) return false;
    if( p.y < y ) return false;
    if( p.y > y+h ) return false;
    return true;
  }
  int xPercent( TS_Point p) {
    if( isInside( p)){
      int result = map( p.x-x, 0, w, 0, 100 );
      Serial.print( "mapped point "); Serial.println( result);
      return result;
    }else{
      return 0;
    }
  }
};

class Wait {

  unsigned long delayMiliSec;
  unsigned long timoutMiliSec;
public:

  Wait(unsigned long delay) {
    delayMiliSec = delay;
    reset();
  }
  void reset() {
    timoutMiliSec = millis() + delayMiliSec;
  }
  bool done() {
    if (millis() > timoutMiliSec) {
      reset();
      return true;
    }
    return false;
  }
};

class Environment {
private:
  //returns canvas height
  int displayProperty(int posY, String name, int value, String units) {
    //Font is 17 heigh. Alligns to 12 Y, so add some whitespace of 20S
    int canvasHeight = 20;
    GFXcanvas16 canvas(ILI9341_TFTWIDTH, canvasHeight);

    int textY = 14;

    canvas.fillScreen(ILI9341_WHITE);
    canvas.setTextColor(ILI9341_RED);
    canvas.setFont(&FreeSansBold9pt7b);
    canvas.setTextWrap(false);

    int16_t x1, y1;
    uint16_t w, h;
    canvas.getTextBounds(name, 0, 0, &x1, &y1, &w, &h);
    //     Serial.print(name); Serial.print( " x=" );Serial.print( x1); Serial.print( " y="); Serial.print( y1);Serial.print( " w="); Serial.print( w);Serial.print( " h=");Serial.println( h);
    canvas.setCursor(canvas.width() / 2 - w, textY);
    canvas.print(name);
    canvas.setCursor(canvas.width() / 2, textY);
    canvas.print(" = ");
    canvas.print(value);
    canvas.print(units);
    tft.drawRGBBitmap(0, posY, canvas.getBuffer(), canvas.width(), canvas.height());
    return canvasHeight;
  }

public:
  int temperature = 0;
  int humidity = 0;
  int pressure = 0;
  Environment(int temp, int hum, int pres) {
    temperature = temp;
    humidity = hum;
    pressure = pres;
  }
  bool equal(Environment other) {
    if (temperature != other.temperature) {
      return false;
    }
    if (humidity != other.humidity) {
      return false;
    }
    if (pressure != pressure) {
      return false;
    }
    return true;
  }
  //Return final posY;
  int display(int posY) {
    posY += displayProperty(posY, "Temperature", temperature, "C");
    posY += displayProperty(posY, "Humidity", humidity, "%");
    posY += displayProperty(posY, "Pressure", pressure * 10, "hPa");
    int time = millis() / 1000;
    if (time > 60) {
      posY += displayProperty(posY, "Up time", time / 60, " minutes");
    } else {
      posY += displayProperty(posY, "Up time", time, " seconds");
    }
    return posY;
  }
  void printEnv() {

    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Humidity    = ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Pressure    = ");
    Serial.print(pressure);
    Serial.println(" kPa");
  }
};

Environment lastReading(0, 0, 0);
Wait waitDisplayEnv(1000);
Wait waitScreenOff(60 * 1000);
Box boxFan(0, 0, 0, 0);

void setup() {
  Serial.begin(115200);  //Use serial monitor for debugging
  delay(1000);
  Serial.println("Running Dry_demo");

  if (!ENV.begin()) {
    Serial.println("Failed to initialize MKR ENV Shield!");
    while (1)
      ;
  }
  Serial.println("Initilised MRK ENV");

  pinMode(TFT_LED, OUTPUT);  // define as output for backlight control

  Serial.println("Init TFT and Touch...");
  tft.begin();
  touch.begin();
  touch.setRotation(2);
  Serial.print("tftx =");
  Serial.print(tft.width());
  Serial.print(" tfty =");
  Serial.println(tft.height());
  tft.fillScreen(ILI9341_BLACK);
  IntroScreen();
  backlightOn();

  delay(1000);
}


void loop() {

  if (waitDisplayEnv.done()) {
    if (!lastReading.equal(readCurrentValue())) {
      lastReading = readCurrentValue();
    }
    lastReading.display(ILI9341_TFTHEIGHT - 80);
  }
  if (waitScreenOff.done()) {
    backlightOff();
  }
  if (touch.touched()) {
    TS_Point touchPoint = getTouch();
    backlightOn();
    waitScreenOff.reset();
    if( boxFan.isInside( touchPoint)){
      boxFan = drawFanSpeed(  boxFan.xPercent( touchPoint));
    }
  }
}

/*____Calibrate Touchscreen_____*/
#define MINPRESSURE 10  // minimum required force for touch event
#define TS_MINX 370
#define TS_MINY 470
#define TS_MAXX 3700
#define TS_MAXY 3600

TS_Point getTouch() {

  TS_Point p = touch.getPoint();
  delay(100);

  char str[100];
  sprintf(str, "screen touched original x=%d y=%d z=%d", p.x, p.y, p.z);
  Serial.println(str);
  p.x = map(p.x, TS_MINX, TS_MAXX, 0, ILI9341_TFTWIDTH);
  p.y = map(p.y, TS_MINY, TS_MAXY, 0, ILI9341_TFTHEIGHT);
  sprintf(str, "screen touched mapped x=%d y=%d z=%d\n", p.x, p.y, p.z);
  Serial.println(str);
  return p;
}
void IntroScreen() {
  //Draw the Result Box
  tft.fillRect(0, 0, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, ILI9341_WHITE);
  tft.drawRGBBitmap(50, 10, image_data_DryLivingLogo, 150, 158);

  // tft.setTextSize(0);
  // tft.setTextColor(ILI9341_BLACK);
  // tft.setFont(&FreeSansBold9pt7b);

  // tft.setCursor(42, 190);
  // tft.println("Humidity Controller");

  Serial.println("Done Intro screen");
  boxFan = drawFanSpeed(65);
}
Box drawFanSpeed(int percentSpeed) {

  Serial.print( "drawFanSpeed value="); Serial.println( percentSpeed);
  tft.setTextSize(0);
  tft.setTextColor(ILI9341_RED);
  tft.setFont(&FreeSansBold9pt7b);

  tft.setCursor(10, 190);
  tft.println("Fan speed : ");

  tft.fillRect(boxFan.x, boxFan.y, boxFan.w, boxFan.h, ILI9341_WHITE);
  tft.drawRect(boxFan.x, boxFan.y, boxFan.w, boxFan.h, ILI9341_GREEN);

  int txtWidth = getTextWidth("Fan speed : ");
  int lineX = 10 + txtWidth + 10;
  int lineW = ILI9341_TFTWIDTH - lineX - 20;
  tft.drawFastHLine(lineX, 185, lineW, ILI9341_BLACK);

  int percentW = (lineW * percentSpeed / 100);
  int valueX = lineX + percentW;

  tft.fillCircle(valueX, 185, 5, ILI9341_RED);
  tft.fillRect(lineX, 185, percentW, 3, ILI9341_RED);
  return Box(lineX, 185 - 10, lineW, 10 + 10);
}
int getTextWidth(String text) {
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  return w;
}
Environment readCurrentValue() {
  return Environment(ENV.readTemperature(), ENV.readHumidity(), ENV.readPressure());
}
void backlightOn() {
  digitalWrite(TFT_LED, LOW);  // LOW to turn backlight on;
}
void backlightOff() {
  digitalWrite(TFT_LED, HIGH);  // HIGH to turn backlight on;
}