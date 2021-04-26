#include <Arduino.h>
// This is a test example for Display

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
