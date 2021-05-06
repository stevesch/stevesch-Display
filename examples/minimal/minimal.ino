// This is a test example for Display
#include <Arduino.h>
#include <stevesch-Display.h>

#define TFT_WIDTH 240
#define TFT_HEIGHT 320
stevesch::Display display(TFT_WIDTH, TFT_HEIGHT);

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Setup initializing...");

  // place setup code here

  Serial.println("Setup complete.");
}

void loop()
{
  // place loop code here
}
