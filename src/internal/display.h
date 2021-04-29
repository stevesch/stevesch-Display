#ifndef STEVESCH_DISPLAY_INTERNAL_DISPLAY_H_
#define STEVESCH_DISPLAY_INTERNAL_DISPLAY_H_
#include <Arduino.h>

#include <TFT_eSPI.h>
#include <SPI.h>

namespace stevesch
{

// constexpr int16_t SCREEN_WIDTH = (int16_t)TFT_WIDTH;
// constexpr int16_t SCREEN_HEIGHT = (int16_t)TFT_HEIGHT;

#define DEFAULT_FONT        1
#define DEFAULT_TEXT_SIZE   1
#define DEFAULT_TEXT_COLOR  TFT_WHITE
#define CLEAR_COLOR         TFT_BLACK


class Display
{
public:
  Display(TFT_eSPI* pTft);
  ~Display();

  void clearDisplay(uint16_t fillColor = CLEAR_COLOR);
  void clearRenderTarget(uint16_t fillColor = CLEAR_COLOR);
  void finishRender();
  void fullScreenMessage(const String& msg);
  void setupDisplay();

  // NOTE: there is no error checking in these-- they must be paired
  // (never call yield w/o first calling claim)
  void displayClaimSPI(); // must call before rendering (before finishRender)
  void displayYieldSPI(); // call when rendering finished (after finishRender)

private:
  TFT_eSPI *mTft;
  TFT_eSPI *mRenderTarget;
  int mRenderTargetIndex;

  // position on screen where backbuffer is rendered:
  int32_t mViewportX;
  int32_t mViewportY;

#if ENABLE_TFT_DMA
  static const int renderTargetCount = 2;
  int renderTargetIndex;
#elif ENABLE_MULTI_CORE_COPY
  static const int renderTargetCount = 2;
  // const int renderTargetCount = 3;
  // const int renderTargetCount = 5;
#else
  static const int renderTargetCount = 1;
#endif
  TFT_eSprite mBackbuffer[renderTargetCount];
};

}

#endif
