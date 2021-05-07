#include "display.h"
// #include <Streaming.h>

#if ENABLE_MULTI_CORE_COPY
#include "multi-core.h"
#endif

namespace stevesch {

Display::Display(int16_t width, int16_t height) :
  mTft(width, height),
  mViewportX(0), mViewportY(0),
#if ENABLE_TFT_DMA
  mRenderTargetIndex(0),
  mBackbuffer { TFT_eSprite(&mTft), TFT_eSprite(&mTft) },
  mRenderTarget(&mTft) // default to display-- use backbuffer if creation succeeds
#elif ENABLE_MULTI_CORE_COPY
  mBackbuffer { TFT_eSprite(&mTft), TFT_eSprite(&mTft) },
  mRenderTarget(0)
#else
  mBackbuffer { TFT_eSprite(&mTft) },
  mRenderTarget(&mTft) // default to display-- use backbuffer if creation succeeds
#endif
{
}

Display::~Display()
{
}

///////////////////////////////////////

// use clearRenderTarget/finishRender instead, whenever possible.
void Display::clearDisplay(uint16_t fillColor)
{
  tft()->fillScreen(fillColor);
}

void Display::clearRenderTarget(uint16_t fillColor)
{
  if (mRenderTarget == &mTft)
  {
    // If there is no backbuffer, we just fill the screen directly
    mRenderTarget->fillScreen(fillColor);
  }
  else
  {
    TFT_eSprite *sprite = (TFT_eSprite *)mRenderTarget;
    sprite->fillSprite(fillColor);
#if 0
    // draw red border for debugging
    int x1 = sprite->width() - 1;
    int y1 = sprite->height() - 1;
    sprite->drawLine(0, 0, x1, 0, TFT_RED);
    sprite->drawLine(0, y1, x1, y1, TFT_RED);
    sprite->drawLine(0, 0, 0, y1, TFT_RED);
    sprite->drawLine(x1, 0, x1, y1, TFT_RED);
#endif
  }
}


void Display::finishRender()
{
#if ENABLE_TFT_DMA
  if (mRenderTarget != &mTft)
  {
    TFT_eSprite *src = (TFT_eSprite *)mRenderTarget;
    mRenderTargetIndex = (mRenderTargetIndex + 1) % renderTargetCount;
    mRenderTarget = &mBackbuffer[mRenderTargetIndex];
    tft()->dmaWait();
    // We're claiming SPI usage within setup, so no need for claim/yield here
    // claimSPI();
    tft()->pushImageDMA(mViewportX, mViewportY,
                         src->width(), src->height(),
                         (uint16_t *)src->getPointer());
    // yieldSPI();
  }
#elif ENABLE_MULTI_CORE_COPY
  if (mRenderTarget && (mRenderTarget != &mTft))
  {
    signalDrawBufferReady((TFT_eSprite *)mRenderTarget, mViewportX, mViewportY);
    mRenderTarget = claimDrawBuffer();
  }
#else
  if (mRenderTarget != &mTft)
  {
    mBackbuffer[0].pushSprite(mViewportX, mViewportY);
  }
  // else no backbuffer-- no work to do
#endif
}

///////////////////////////////////////

void Display::fullScreenMessage(const String& msg)
{
  clearRenderTarget();
  mRenderTarget->setTextDatum(TL_DATUM); // restore to default
  mRenderTarget->setCursor(0, 0);
  mRenderTarget->println(msg);
  finishRender();
}


void ICACHE_FLASH_ATTR Display::setup()
{
  TFT_eSPI* pTft = tft();
  pTft->init();
  // Serial << "Display initialized (" << pTft->width() << " x " << pTft->height() << ")" << endl;

  clearDisplay();
#if ENABLE_TFT_DMA
  pTft->initDMA();
#endif
  pTft->setTextFont(DEFAULT_FONT);
  pTft->setTextSize(DEFAULT_TEXT_SIZE);
  pTft->setTextColor(DEFAULT_TEXT_COLOR);
  pTft->setCursor(0, 0);

#if !DISABLE_BACKBUFFER
  const uint displayWidth = pTft->width();
  const uint displayHeight = pTft->height();
  uint targetWidth = displayWidth;
  uint targetHeight = displayHeight;
  float factor = 1.0f;
  do
  {
    int i;
    for (i = 0; i < renderTargetCount; ++i)
    {
      mBackbuffer[i].createSprite(targetWidth, targetHeight);
      if (!mBackbuffer[i].created())
      {
        break;
      }
    }
    if (i < renderTargetCount)
    {
      // couldn't create all of the backbuffers-- delete any we already created
      while (--i >= 0)
      {
        mBackbuffer[i].deleteSprite();
      }
    }
    else
    {
      break; // created all the buffers successfully
    }

    // targetWidth = (targetWidth * 9) / 10;
    // targetHeight = (targetHeight * 9) / 10;
    factor -= 0.01f;
    targetWidth = (int)(displayWidth * factor);
    targetHeight = (int)(displayHeight * factor);
  } while (targetWidth > 10);

  mViewportX = ((int)displayWidth - targetWidth) / 2;
  mViewportY = ((int)displayHeight - targetHeight) / 2;

  for (int i = 0; i < renderTargetCount; ++i)
  {
    auto &target = mBackbuffer[i];
    if (target.created())
    {
      target.setTextFont(DEFAULT_FONT);
      target.setTextSize(DEFAULT_TEXT_SIZE);
      target.setTextColor(DEFAULT_TEXT_COLOR);
      target.setCursor(0, 0);
    }
  }

  if (mBackbuffer[0].created())
  {
    mRenderTarget = &mBackbuffer[0];
  }
  // else mRenderTarget = &mTft already

  // Serial << "Backbuffer " << (mBackbuffer[0].created() ? "created" : "CREATION FAILED") << " (" << mBackbuffer[0].width() << " x " << mBackbuffer[0].height() << ")[" << renderTargetCount << "]" << endl;
  // Serial << "Viewport: <" << mViewportX << ", " << mViewportY << ">" << endl;

#endif // !DISABLE_BACKBUFFER

#if ENABLE_MULTI_CORE_COPY
  initMultiCoreCopy(mBackbuffer, renderTargetCount);
  mRenderTarget = claimDrawBuffer();
#endif

  claimSPI();
}


void Display::claimSPI()
{
#if ENABLE_TFT_DMA
  tft()->startWrite(); // grab exclusive use of SPI bus
#endif
}

void Display::yieldSPI()
{
#if ENABLE_TFT_DMA
  tft()->endWrite(); // grab exclusive use of SPI bus
#endif
}

}
