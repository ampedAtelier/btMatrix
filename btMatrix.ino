/**
 * btMatrix
 * Amped Atelier
 * Sahrye Cohen
 * Hal Rodriguez
 * 
 * Arduino Settings
 * Board: Adafruit Feather M0
 * 
 * Adafruit Feather M0 Bluefruit LE
 * ATSAMD21 chip with Hardware SPI:
 * CS = 8 (chip select)
 * IRQ = 7 (interrupt request)
 * RST = 4 (Reset)
 * 
 */
#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_GFX.h>

#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"


// Set Up NeoPixels --------------------------------------
#define NEO_PIN 6  //default for the NeoPixel Feather Wing
//#define NEO_WIDTH  32 // one panel
//#define NEO_WIDTH  49 // Atrractions
#define NEO_WIDTH  64 // two panels, Reflections HoopSkirt
#define NEO_HEIGHT  8

Adafruit_NeoMatrix matrix(NEO_WIDTH, NEO_HEIGHT, NEO_PIN,
  NEO_MATRIX_TOP  + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB         + NEO_KHZ800);

char          msg[21]     = {0};            // BLE 20 char limit + NUL
uint8_t       msgLen      = 0;              // Empty message
int           msgX          = matrix.width(); // Start off right edge
unsigned long prevFrameTime = 0L;             // For animation timing
#define FPS 20                                // Scrolling speed

// 0 (off) to 255 (max brightness)
#define BRIGHTNESS 15


// Set Up Bluetooth --------------------------------------
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
/* Create the bluefruit object, hardware SPI, 
  using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


void setup() {
  matrix.begin();
  matrix.setTextWrap(false);   // Allow scrolling off left
  matrix.setTextColor(0x07FF); // Cyan by default
  //matrix.setTextColor(matrix.Color(255, 0, 255)); //magenta
  matrix.setBrightness(BRIGHTNESS);    // Batteries have limited sauce

  strcpy(msg, "CatwalkCharity.org");  // Default Starting Message
  //strcpy(msg, "Amped Atelier");  // Default Starting Message
  //strcpy(msg, "UnitedAgainstCancer");  // Default Starting Message
  msgLen = sizeof(msg);

//  while (!Serial);  // required for Flora & Micro
//  delay(500);
//  Serial.begin(115200);
//  Serial.println(F("Amped Atelier Attractions Bluetooth Debug"));
//  Serial.println(F("-----------------------------------------"));
  /* Initialise the module */
//  Serial.print(F("Initialising the Bluefruit LE module: "));
  if ( !ble.begin(VERBOSE_MODE) ){
//    Serial.print(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
//  Serial.println( F("OK!") );
  if ( FACTORYRESET_ENABLE ) {
    /* Perform a factory reset to make sure everything is in a known state */
//    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
//      Serial.println(F("Couldn't factory reset"));
    }
  }
  /* Disable command echo from Bluefruit */
  ble.echo(false);
//  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();
  ble.verbose(false);  // debug info is a little annoying after this point!
  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) ) {
    // Change Mode LED Activity
//    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }
}

void loop() {
  unsigned long t = millis(); // Current elapsed time, milliseconds.
  // millis() comparisons are used rather than delay() so that animation
  // speed is consistent regardless of message length & other factors.
  
  // Handle scrolling
  if ((t - prevFrameTime) >= (1000L / FPS)) { 
    matrix.fillScreen(0);
    matrix.setCursor(msgX, 0);
    matrix.print(msg);
    if (--msgX < (msgLen * -5)) {
      //Serial.println(F("resetting scrolling"));
      msgX = matrix.width(); // We must repeat!
    }
    matrix.show();
    prevFrameTime = t;
  }

  // Check for incoming characters from Bluefruit
  ble.println("AT+BLEUARTRX");
  ble.readline();
  if (strcmp(ble.buffer, "OK") == 0) {
    // no data
    return;
  }
  // Some data was found, its in the buffer
  //Serial.print(F("[Recv] ")); Serial.println(ble.buffer);
  strcpy(msg, ble.buffer);
  //msgLen = sizeof(msg);
  //msg[msgLen] = 0;
  //msgX = matrix.width(); // Reset scrolling
  ble.waitForOK();
}
