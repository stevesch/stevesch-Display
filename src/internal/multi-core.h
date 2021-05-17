#if ENABLE_MULTI_CORE_COPY
/**
 * @file multi-core.h
 * @author Stephen Schlueter, github: stevesch
 * @brief Queing and copying of backbuffer data for multi-core systems
 * @version 0.1
 * @date 2021-05-16
 * 
 * @copyright Copyright (c) 2021
 * 
 */

class DataLock
{
  SemaphoreHandle_t lockedHandle;

public:
  DataLock(SemaphoreHandle_t h);
  ~DataLock();
};

class TFT_eSprite;

void initMultiCoreCopy(TFT_eSprite *backbuffers, int backbufferCount);

TFT_eSprite *claimDrawBuffer();
void signalDrawBufferReady(TFT_eSprite *buffer, int viewport_x, int viewport_y);
void cancelClaimedDrawBuffer(TFT_eSprite *buffer);

#endif
