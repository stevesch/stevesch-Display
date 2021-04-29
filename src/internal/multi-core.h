#if ENABLE_MULTI_CORE_COPY

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
