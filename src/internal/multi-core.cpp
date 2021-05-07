#if ENABLE_MULTI_CORE_COPY

#include <FreeRTOS.h>
#include <deque>
// #include <Streaming.h>
#include <TFT_eSPI.h>

#include "multi-core.h"

DataLock::DataLock(SemaphoreHandle_t h) : lockedHandle(0)
{
  xSemaphoreTake(h, portMAX_DELAY);
  lockedHandle = h;
}

DataLock::~DataLock()
{
  xSemaphoreGive(lockedHandle);
}

SemaphoreHandle_t availableToDraw_key;
std::deque<TFT_eSprite *> availableToDraw;

SemaphoreHandle_t readyToCopy_key;
struct copyInfo
{
  TFT_eSprite *buffer;
  int viewport_x;
  int viewport_y;
};
std::deque<copyInfo> readyToCopy;

TaskHandle_t xTask_pushSprite = 0;

TFT_eSprite *claimDrawBuffer()
{
  TFT_eSprite *buffer = 0;
  // {
  //   DataLock key(availableToDraw_key);
  //   buffer = availableToDraw.front();
  //   availableToDraw.pop_front();
  // }

  for (;;)
  {
    xSemaphoreTake(availableToDraw_key, portMAX_DELAY);
    // while (!xSemaphoreTake(availableToDraw_key, 1)) {
    //   // vTaskDelay(1 / portTICK_PERIOD_MS);
    //   vTaskDelay(0);
    // }
    if (!availableToDraw.empty())
    {
      break;
    }
    xSemaphoreGive(availableToDraw_key);
    // vTaskDelay(1 / portTICK_PERIOD_MS);
    vTaskDelay(0);
  }

  buffer = availableToDraw.front();
  availableToDraw.pop_front();
  xSemaphoreGive(availableToDraw_key);

  // Serial << "# Taking draw buffer ";
  // Serial.printf("0x%08x", (uint32_t)buffer);
  // Serial << endl;

  return buffer;
}

void signalDrawBufferReady(TFT_eSprite *buffer, int viewport_x, int viewport_y)
{
  {
    DataLock key(readyToCopy_key);
    readyToCopy.push_back({buffer, viewport_x, viewport_y});
  }
  // Serial << "# Giving draw buffer ";
  // Serial.printf("0x%08x", (uint32_t)buffer);
  // Serial << endl;
  xTaskNotifyGive(xTask_pushSprite);
}

void cancelClaimedDrawBuffer(TFT_eSprite *buffer)
{
  DataLock key(availableToDraw_key);
  availableToDraw.push_back(buffer);
}

void pushSprite_task(void *parameter)
{
  for (;;)
  {
    uint32_t n;
    n = ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    // do {
    //   n = ulTaskNotifyTake( pdFALSE, 2 / portTICK_PERIOD_MS );
    //   if (!n) {
    //     vTaskDelay(1);
    //   }
    // } while (!n);

    if (n)
    {
      copyInfo inf = {0, 0, 0};
      {
        DataLock key(readyToCopy_key);
        if (!readyToCopy.size())
        {
          // Serial << "### Error: no readyToCopy buffer when signalled" << endl;
          vTaskDelay(1 / portTICK_PERIOD_MS);
          continue; // invalid
        }
        inf = readyToCopy.front();
        readyToCopy.pop_front();
      }
      // Serial << "# Copying draw buffer ";
      // Serial.printf("0x%08x", (uint32_t)buffer);
      // Serial << endl;
      // vTaskDelay(1);
      inf.buffer->pushSprite(inf.viewport_x, inf.viewport_y);
      vTaskDelay(1);
      {
        DataLock key(availableToDraw_key);
        availableToDraw.push_back(inf.buffer);
      }
      // vTaskDelay(1);
    }
    else
    {
      vTaskDelay(1 / portTICK_PERIOD_MS);
    }
  }
}

void initMultiCoreCopy(TFT_eSprite *backbuffers, int backbufferCount)
{
  availableToDraw_key = xSemaphoreCreateMutex();
  readyToCopy_key = xSemaphoreCreateMutex();
  for (size_t i = 0; i < backbufferCount; ++i)
  {
    availableToDraw.push_back(&backbuffers[i]);
  }

  // Note: Arduino default core is 1, so we run the copy task on core 0 here:
  xTaskCreatePinnedToCore(pushSprite_task, "copyBuffer",
                          10000, NULL, tskIDLE_PRIORITY + 1, &xTask_pushSprite, 0);
  // Serial << "# Multi-core copy initialized (" << backbufferCount << " buffers)" << endl;
}

#endif
