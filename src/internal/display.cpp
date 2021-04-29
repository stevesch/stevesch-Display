#include "display.h"
#include <Streaming.h>

#if ENABLE_MULTI_CORE_COPY
#include "board-hw/multi-core.h"
#endif

// using stevesch::Display::foo;

namespace {
  // local/static data and functions

} // namespace

namespace stevesch {

Display::Display(TFT_eSPI* pTft) :
  mTft(pTft),
  mRenderTarget(0),
  mViewportX(0), mViewportY(0),
#if ENABLE_TFT_DMA
  TFT_eSprite backbuffer { TFT_eSprite(pTft), TFT_eSprite(pTft) },
  renderTargetIndex(0),
  mRenderTarget(pTft) // default to display-- use backbuffer if creation succeeds
#elif ENABLE_MULTI_CORE_COPY
  TFT_eSprite backbuffer { TFT_eSprite(pTft), TFT_eSprite(pTft) },
  mRenderTarget(0)
#else
  mBackbuffer { TFT_eSprite(pTft) },
  mRenderTarget(pTft) // default to display-- use backbuffer if creation succeeds
#endif
{
  // TFT_eSPI display(SCREEN_WIDTH, SCREEN_HEIGHT); // Invoke custom library
}

Display::~Display()
{
}

///////////////////////////////////////

// use clearRenderTarget/finishRender instead, whenever possible.
void Display::clearDisplay(uint16_t fillColor)
{
  display.fillScreen(fillColor);
}

void Display::clearRenderTarget(uint16_t fillColor)
{
  if (mRenderTarget == mTft)
  {
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
  if (mRenderTarget != &display)
  {
    TFT_eSprite *src = (TFT_eSprite *)mRenderTarget;
    renderTargetIndex = (renderTargetIndex + 1) % renderTargetCount;
    mRenderTarget = &backbuffer[renderTargetIndex];
    display.dmaWait();
    // We're claiming SPI usage within setup, so no need for claim/yield here
    // displayClaimSPI();
    display.pushImageDMA(viewport_x, viewport_y,
                         src->width(), src->height(),
                         (uint16_t *)src->getPointer());
    // displayYieldSPI();
  }
#elif ENABLE_MULTI_CORE_COPY
  if (mRenderTarget && (mRenderTarget != &display))
  {
    signalDrawBufferReady((TFT_eSprite *)mRenderTarget, viewport_x, viewport_y);
    mRenderTarget = claimDrawBuffer();
  }
#else
  if (mRenderTarget != &display)
  {
    backbuffer[0].pushSprite(viewport_x, viewport_y);
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


void ICACHE_FLASH_ATTR Display::setupDisplay()
{
  // Serial << "Initializing display (" <<
  //   SCREEN_WIDTH << " x " <<
  //   SCREEN_HEIGHT << ")" << endl);
  display.init();
  Serial << "Display initialized (" << display.width() << " x " << display.height() << ")" << endl;

  clearDisplay();
#if ENABLE_TFT_DMA
  display.initDMA();
#endif
  display.setTextFont(DEFAULT_FONT);
  display.setTextSize(DEFAULT_TEXT_SIZE);
  display.setTextColor(DEFAULT_TEXT_COLOR);
  display.setCursor(0, 0);

#if !DISABLE_BACKBUFFER
  // backbuffer.createSprite(display.width(), display.height());
  const uint displayWidth = display.width();
  const uint displayHeight = display.height();
  uint targetWidth = displayWidth;
  uint targetHeight = displayHeight;
  float factor = 1.0f;
  do
  {
    int i;
    for (i = 0; i < renderTargetCount; ++i)
    {
      backbuffer[i].createSprite(targetWidth, targetHeight);
      if (!backbuffer[i].created())
      {
        break;
      }
    }
    if (i < renderTargetCount)
    {
      // couldn't create all of the backbuffers-- delete any we already created
      while (--i >= 0)
      {
        backbuffer[i].deleteSprite();
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

  viewport_x = ((int)displayWidth - targetWidth) / 2;
  viewport_y = ((int)displayHeight - targetHeight) / 2;

  for (int i = 0; i < renderTargetCount; ++i)
  {
    auto &target = backbuffer[i];
    if (target.created())
    {
      target.setTextFont(DEFAULT_FONT);
      target.setTextSize(DEFAULT_TEXT_SIZE);
      target.setTextColor(DEFAULT_TEXT_COLOR);
      target.setCursor(0, 0);
    }
  }

  if (backbuffer[0].created())
  {
    mRenderTarget = &backbuffer[0];
  }
  // else mRenderTarget = &display already

  Serial << "Backbuffer " << (backbuffer[0].created() ? "created" : "CREATION FAILED") << " (" << backbuffer[0].width() << " x " << backbuffer[0].height() << ")[" << renderTargetCount << "]" << endl;
  Serial << "Viewport: <" << viewport_x << ", " << viewport_y << ">" << endl;

#endif // !DISABLE_BACKBUFFER

#if ENABLE_MULTI_CORE_COPY
  initMultiCoreCopy(backbuffer, renderTargetCount);
  mRenderTarget = claimDrawBuffer();
#endif

  displayClaimSPI();
}


void Display::displayClaimSPI()
{
#if ENABLE_TFT_DMA
  display.startWrite(); // grab exclusive use of SPI bus
#endif
}
void Display::displayYieldSPI()
{
#if ENABLE_TFT_DMA
  display.endWrite(); // grab exclusive use of SPI bus
#endif
}

}
}
