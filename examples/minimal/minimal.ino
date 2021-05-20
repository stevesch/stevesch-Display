/**
 * @file minimal.ino
 * @author Stephen Schlueter (https://github.com/stevesch)
 * @brief Test example for stevesch-Display
 * @version 0.1
 * @date 2021-05-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <Arduino.h>
#include <stevesch-Display.h>

#include "boardSettings.h"

#ifdef AXP192_ADDRESS
	// AXP192 power management chip support (M5StickC)
#include "board-hw/axp192.h"
#endif

stevesch::Display display(TFT_WIDTH, TFT_HEIGHT);

long lastDraw = 0;
float averageFps = 0.0f;

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Setup initializing...");

#ifdef I2C_SDA
  // M5StickC/M5StickC Plus support
	Wire.begin(I2C_SDA, I2C_SCL);

	#ifdef AXP192_ADDRESS
	Wire.setClock(400000);
	axpSetup();
	#endif
#endif

  display.setup();

  // simple blocking message:
  display.fullScreenMessage("Display demo");
  delay(2000);

  display.currentRenderTarget()->setTextColor(TFT_RED);
  display.fullScreenMessage("Red text example");
  delay(1000);

  display.currentRenderTarget()->setTextColor(TFT_GREEN);
  display.fullScreenMessage("Green text example");
  delay(1000);

  display.currentRenderTarget()->setTextColor(TFT_BLUE);
  display.fullScreenMessage("Blue text example");
  delay(1000);

  // Note: setting text color is per backbuffer, so to return text to default color,
  // we do this for all buffers.  (alternatively, we could just call setTextColor
  // everwhere in the app before drawing text).
  display.forEachRenderTarget([](TFT_eSPI* target, void* context) {
    target->setTextColor(stevesch::Display::DEFAULT_TEXT_COLOR);
  }, 0);

  lastDraw = micros();
  Serial.println("Setup complete.");
}

// i.e. std::clamp (not on all compilers yet)
int clamp(int x, int a, int b) {
  if (x < a)
    return a;
  if (x > b)
    return b;
  return x;
}

float exampleAccumTime = 0.0f;
void advanceTime(float dt)
{
  // simple delta-time based
  // (simple example-- real-world use would perform all time updates here)
  exampleAccumTime += dt;
}

void renderExample()
{
  // ideally, don't perform updates within your render-- just draw based on state
  display.clearRenderTarget();

  TFT_eSPI* renderTarget = display.currentRenderTarget();

  int w = renderTarget->getViewportWidth();
  int h = renderTarget->getViewportHeight();

  int cx = w / 2;
  int cy = h / 2;
  float r = (float)std::min(w/2 - 2, h/2 - 2);

  const float kRotationsPerSecond = 0.5f;
  const float kRotationRate = M_PI * 2.0f * kRotationsPerSecond;
  float theta = exampleAccumTime * kRotationRate;
  float rx = r*cosf(theta);
  float ry = r*sinf(theta);
  float x0 = cx + rx;
  float y0 = cy - ry;
  float x1 = cx - rx;
  float y1 = cy + ry;
  int ix0 = clamp((int)roundf(x0), 0, w - 1);
  int iy0 = clamp((int)roundf(y0), 0, h - 1);
  int ix1 = clamp((int)roundf(x1), 0, w - 1);
  int iy1 = clamp((int)roundf(y1), 0, h - 1);

  const uint16_t color = TFT_GREEN;
  renderTarget->drawLine(ix0, iy0, ix1, iy1, color);
 
  String str("Elapsed time: ");
  str += String(exampleAccumTime, 2);
  str += "\nfps: ";
  str += String(averageFps, 1);

  renderTarget->setTextDatum(TL_DATUM);
  renderTarget->setCursor(0, 0);
  renderTarget->println(str);

  // flip backbuffer (via either copy, DMA, or multi-core copy, depending on board settings):
  display.finishRender();
}

void loop()
{
  long now = micros();
  long delta = now - lastDraw;

  // const long kDrawInterval = 500000; // Limit fps to ~2fps
  // const long kDrawInterval = 16666; // Limit fps to ~60fps
  // const long kDrawInterval = 33333;  // Limit fps to ~30fps
  const long kDrawInterval = 0; // Render without framerate cap (as fast as possible)

  if (delta >= kDrawInterval) {
    lastDraw = now;
    float dt = (float)delta * 1.0e-6f;  // delta time (in seconds) since last render

    // compute average fps:
    float fpsThisFrame = 1.0f / dt;
    const float kAvWeight = 0.1f; // between 0 and 1.  use lower value to average over a longer time
    // The following results in roughly geometric weighting w/previous frames--
    // this weighting varies if framerate changes drastically, but we just want
    // some smoothing, so it's sufficient and easy:
    averageFps = (1.0f - kAvWeight)*averageFps + kAvWeight*fpsThisFrame;

    // In practice, we could separate update and render so that
    // updates could happen more often than render if necessary, e.g. if rendering
    // is taking too long but we want to maintain proper physics behavior
    // (or visa-versa with multiple interpolated renders for a single update
    // if it was appropriate for a particular case).
    // For this simple example, we're just updating and rendering each time.
    advanceTime(dt);
    renderExample();
  }
}
