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
  private:
    //returns canvas width
    int displayProperty(  int posY, String name, int value, String units){

      //Font is 17 heigh. Alligns to 12 Y, so add some whitespace of 20
      int canvasHeight = 20;
      int textY = 14;
      GFXcanvas16 canvas(ILI9341_TFTWIDTH, canvasHeight);

      canvas.fillScreen( ILI9341_ORANGE );
      canvas.setTextColor(ILI9341_RED);
      canvas.setFont( &FreeSansBold9pt7b);  
      canvas.setTextWrap(false);

      int16_t  x1, y1;
      uint16_t w, h;
      canvas.getTextBounds(name, 0, 0, &x1, &y1, &w, &h);
      Serial.print(name); Serial.print( " x=" );Serial.print( x1); Serial.print( " y="); Serial.print( y1);Serial.print( " w="); Serial.print( w);Serial.print( " h=");Serial.println( h);

      canvas.setCursor(0, textY);
      canvas.print( name);
      canvas.setCursor( canvas.width() / 2, textY);
      canvas.print( "= ");
      canvas.print( value);
      canvas.print( units);
      tft.drawRGBBitmap( 0, posY, canvas.getBuffer(), canvas.width(), canvas.height());
      delay(1000);
      return canvasHeight;
    }

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
    //Return final posY;
    int display(int posY ){      
      posY += displayProperty( posY, "Temperature", temperature, "C");
      posY += displayProperty( posY, "Humidity", humidity, "%");
      posY += displayProperty( posY, "Pressure", pressure, "Pa");
      return posY;
    }

} ;

Environment lastReading(0,0,0);


void setup() {
  Serial.begin(115200); //Use serial monitor for debugging
  delay(1000);
  Serial.println( "Running Dry_demo");

  if (!ENV.begin()) {
    Serial.println("Failed to initialize MKR ENV Shield!");
    while (1);
  }
  Serial.println( "Initilised MRK ENV");

  pinMode(TFT_LED, OUTPUT); // define as output for backlight control

  Serial.println("Init TFT and Touch...");
  tft.begin();
  touch.begin();
  Serial.print("tftx ="); Serial.print(tft.width()); Serial.print(" tfty ="); Serial.println(tft.height());
  tft.fillScreen(ILI9341_BLACK);
  
  IntroScreen();
  digitalWrite(TFT_LED, LOW);    // LOW to turn backlight on; 

  Serial.println( "setup return wait");
  delay(1000);
  Serial.println( "setup() return");
}

void loop() {
    Environment currentValue = readCurrentValue();
    if( !lastReading.equal( currentValue)){
      lastReading = currentValue;
      printEnv(lastReading);
      lastReading.display( ILI9341_TFTHEIGHT - 60);
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

  Serial.println("Done Intro screen");
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

