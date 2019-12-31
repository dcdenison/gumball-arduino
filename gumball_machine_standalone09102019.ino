/*
An early version that I think worked okay, but we misplaced it for a few weeks.
*/

#include <Wire.h>
#include <SparkFun_APDS9960.h>
#include <Arduino.h>
#include "BasicStepperDriver.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <EEPROM.h>

int dispenses;
int lastUpdateMillis = 0;

//#define APDS9960_INT 2

#define MOTOR_STEPS 400
#define RPM 120
#define MICROSTEPS 1

SparkFun_APDS9960 apds = SparkFun_APDS9960();

#define DIR 3
#define STEP 4

BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);

//int isr_flag = 0;

#define STEPSTOTAKE 400 //Change this to how many steps you need to dispense a gumball

#define PIXEL_PIN 6
#define PIXEL_COUNT 60
#define BRIGHTNESS 50 //Between 0-100

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  //pinMode(APDS9960_INT, INPUT);
  Serial.begin(9600);
  if(EEPROM.read(12) != 1){
  EEPROM.write(12, 1);
  EEPROM.write(10, 0);
  EEPROM.write(11, 0);
  }

  dispenses = (EEPROM.read(10) << 8) + EEPROM.read(11);
  //attachInterrupt(0, interruptRoutine, FALLING);

  pixels.begin();
  pixels.setBrightness(BRIGHTNESS);
  pixels.show();
  stepper.begin(RPM, MICROSTEPS);

  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
    for(int pixel=0;pixel<PIXEL_COUNT;pixel++){
      pixels.setPixelColor(pixel,pixels.Color(160,0,0));
    }
    pixels.show();
    while(1);
  }
  if ( apds.enableGestureSensor(false) ) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }
}

void loop() {
handleGesture(); //Poll it manually
  /*if(isr_flag){
    detachInterrupt(0);
    handleGesture();
    delay(3000);
    attachInterrupt(0, interruptRoutine, FALLING);
    isr_flag = 0;
  }*/
  rainbow(40);
}

void dispense_gumball(){
  Serial.println("Dispensing gumball");
  dispenses++;
  EEPROM.write(10, (0xFF00 & dispenses) >> 8);
  EEPROM.write(11, dispenses & 0xFF);
  Serial.println(dispenses);
  //loop_around(4);
  chase(pixels.Color(0,255,0));
  for(int i=0;i<pixels.numPixels();i++) pixels.setPixelColor(i, pixels.Color(20,0,255));
  pixels.show();
  stepper.move(STEPSTOTAKE);
  chase(pixels.Color(0,255,0));
  Serial.println("Gumball Dispensed");
  lastUpdateMillis = millis();
}

/*void interruptRoutine() {
  if(isr_flag==0){
    isr_flag = 1;
  }
}*/

void loop_around(int loop_iterations){
  for(int loops=0;loops<loop_iterations;loops++){
  for(int i=0;i<PIXEL_COUNT;i++){
    pixels.setPixelColor(i,pixels.Color(0,0,0));
    for(int j=i;j<i+5;j++){
      pixels.setPixelColor(j,pixels.Color(0,0,240));
    }
    pixels.show();
    delay(40);
  }
  }
}

void handleGesture() {
    if ( apds.isGestureAvailable() ) {
    switch ( apds.readGesture() ) {
      case DIR_UP:
        Serial.println("UP");
        dispense_gumball();
        break;
      case DIR_DOWN:
        Serial.println("DOWN");
        dispense_gumball();
        break;
      case DIR_LEFT:
        Serial.println("LEFT");
        dispense_gumball();
        break;
      case DIR_RIGHT:
        Serial.println("RIGHT");
        dispense_gumball();
        break;
      case DIR_NEAR:
        Serial.println("NEAR");
        dispense_gumball();
        break;
      case DIR_FAR:
        Serial.println("FAR");
        dispense_gumball();
        break;
      default:
        Serial.println("NONE");
    }
  }
}

void theaterChaseRainbow(void) {
  for (int j=0; j < 16; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < pixels.numPixels(); i=i+3) {
          pixels.setPixelColor(i+q, Wheel( (i+j*16) % 255));    //turn every third pixel on
        }
        pixels.show();
        for (int i=0; i < pixels.numPixels(); i=i+3) {
          pixels.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

static void chase(uint32_t c) {
  for(uint16_t i=0; i<pixels.numPixels()+4; i++) {
      pixels.setPixelColor(i  , c); // Draw new pixel
      pixels.setPixelColor(i-4, 0); // Erase pixel a few steps back
      pixels.show();
      delay(25);
  }
}

void rainbow(uint8_t wait)
{
  static int direction = -1;
  static int pixelIndex = 0;

    if (pixelIndex <= 0 or pixelIndex >= 255)
    {
      direction *= -1;
    }
    pixelIndex += direction;
    for (int i = 0; i < pixels.numPixels(); i++)
    {
      pixels.setPixelColor( i, Wheel((i + pixelIndex) & 255));  // not sure about this...
    }
    pixels.show();
    lastUpdateMillis = millis();  //  Whoopsie!!
    delay(wait);
}

uint32_t Wheel(byte WheelPos) {

  if(WheelPos < 85) {
   return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
