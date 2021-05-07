#ifndef STEVESCH_DISPLAY_INTERNAL_DISPLAY_H_
#define STEVESCH_DISPLAY_INTERNAL_DISPLAY_H_
#include <Arduino.h>

#include <TFT_eSPI.h>
#include <SPI.h>

namespace stevesch
{
class Display
{
public:
  Display(int16_t width, int16_t height);
  ~Display();

  static constexpr uint8_t DEFAULT_FONT = 1;
  static constexpr uint8_t DEFAULT_TEXT_SIZE = 1;
  static constexpr uint16_t DEFAULT_TEXT_COLOR = TFT_WHITE;
  static constexpr uint16_t DEFAULT_CLEAR_COLOR = TFT_BLACK;

  void clearDisplay(uint16_t fillColor = DEFAULT_CLEAR_COLOR);
  void clearRenderTarget(uint16_t fillColor = DEFAULT_CLEAR_COLOR);
  void finishRender();
  void fullScreenMessage(const String& msg);
  void setup();

  // NOTE: there is no error checking in these-- they must be paired
  // (never call yield w/o first calling claim)
  void claimSPI(); // must call before rendering (before finishRender)
  void yieldSPI(); // call when rendering finished (after finishRender)

  TFT_eSPI* currentRenderTarget() { return mRenderTarget; }

  typedef void (*renderTargetCallback_t)(TFT_eSPI*, void*);
  void forEachRenderTarget(renderTargetCallback_t callback, void* context);

  // direct access to TFT (for issuing commands-- use sparingly)
  TFT_eSPI* tft() { return &mTft; }

private:
  TFT_eSPI mTft;

  // position on screen where backbuffer is rendered:
  int32_t mViewportX;
  int32_t mViewportY;

#if ENABLE_TFT_DMA
  static const int renderTargetCount = 2;
  int mRenderTargetIndex;
#elif ENABLE_MULTI_CORE_COPY
  static const int renderTargetCount = 2;
  // const int renderTargetCount = 3;
  // const int renderTargetCount = 5;
#else
  static const int renderTargetCount = 1;
#endif

  TFT_eSprite mBackbuffer[renderTargetCount];
  TFT_eSPI *mRenderTarget;
};

inline void Display::forEachRenderTarget(renderTargetCallback_t callback, void* context)
{
  int i;
  for (i = 0; i < renderTargetCount; ++i)
  {
    callback(&mBackbuffer[i], context);
  }
}

}

#endif
