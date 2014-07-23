// ����ͷ�ļ�
#include "adev.h"

// �ڲ����Ͷ���
typedef struct
{
    HWAVEOUT hWaveOut;
    WAVEHDR *pWaveHdr;
    int      bufnum;
    int      buflen;
    HANDLE   bufsem;
    long     head;
    long     tail;
} ADEV;

// �ڲ�����ʵ��
static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    ADEV *dev = (ADEV*)dwInstance;
    switch (uMsg)
    {
    case WOM_DONE:
        dev->pWaveHdr[dev->head].dwBufferLength = dev->buflen;
        if (++dev->head == dev->bufnum) dev->head = 0;
        ReleaseSemaphore(dev->bufsem, 1, NULL);
        break;
    }
}

// ����ʵ��
void* adev_create(int bufnum, int buflen)
{
    ADEV *dev = malloc(sizeof(ADEV));
    if (dev)
    {
        WAVEFORMATEX wfx = {0};
        BYTE        *pwavbuf;
        int          i;

        // create semaphore
        dev->bufsem = CreateSemaphore(NULL, bufnum, bufnum, NULL);

        // init for audio
        wfx.cbSize          = sizeof(wfx);
        wfx.wFormatTag      = WAVE_FORMAT_PCM;
        wfx.wBitsPerSample  = 16;    // 16bit
        wfx.nSamplesPerSec  = 44100; // 44.1k
        wfx.nChannels       = 2;     // stereo
        wfx.nBlockAlign     = wfx.nChannels * wfx.wBitsPerSample / 8;
        wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
        waveOutOpen(&(dev->hWaveOut), WAVE_MAPPER, &wfx, (DWORD_PTR)waveOutProc, (DWORD)dev, CALLBACK_FUNCTION);

        dev->bufnum   = bufnum;
        dev->buflen   = buflen;
        dev->pWaveHdr = (WAVEHDR*)malloc(bufnum * (sizeof(WAVEHDR) + buflen));
        dev->head     = 0;
        dev->tail     = 0;

        // init
        pwavbuf = (BYTE*)(dev->pWaveHdr + bufnum);
        for (i=0; i<bufnum; i++) {
            dev->pWaveHdr[i].lpData         = (LPSTR)(pwavbuf + i * buflen);
            dev->pWaveHdr[i].dwBufferLength = buflen;
            waveOutPrepareHeader(dev->hWaveOut, &(dev->pWaveHdr[i]), sizeof(WAVEHDR));
        }
    }
    return dev;
}

void adev_destroy(void *ctxt)
{
    int i;
    ADEV *dev = (ADEV*)ctxt;
    if (dev == NULL) return;

    // unprepare
    for (i=0; i<dev->bufnum; i++) {
        waveOutUnprepareHeader(dev->hWaveOut, &(dev->pWaveHdr[i]), sizeof(WAVEHDR));
    }

    if (dev->hWaveOut) waveOutClose(dev->hWaveOut);
    if (dev->pWaveHdr) free(dev->pWaveHdr);
    if (dev->bufsem  ) CloseHandle(dev->bufsem);
    free(dev);
}

void adev_audio_buf_request(void *ctxt, AUDIOBUF **ppab)
{
    ADEV *dev = (ADEV*)ctxt;
    if (dev == NULL) return;

    WaitForSingleObject(dev->bufsem, -1);
    *ppab = (AUDIOBUF*)&(dev->pWaveHdr[dev->tail]);
}

void adev_audio_buf_post(void *ctxt, AUDIOBUF **ppab)
{
    ADEV *dev = (ADEV*)ctxt;
    if (dev == NULL) return;

    waveOutWrite(dev->hWaveOut, (LPWAVEHDR)*ppab, sizeof(WAVEHDR));
    if (++dev->tail == dev->bufnum) dev->tail = 0;
}
