#ifndef RECTOOLS_H
#define RECTOOLS_H

#include "portaudio.h"
#include "aubio/aubio.h"
#include <string>
#include <QMainWindow>
#include "ui_detectormain.h"

#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (2048)
#define NUM_CHANNELS (1)
#define DITHER_FLAG (0)

#define PA_SAMPLE_TYPE paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE (0)
#define PRINTF_S_FORMAT "%d"

#define FFT_METHOD "default"
#define HOP_SIZE (int)(FRAMES_PER_BUFFER/2)
#define PERIOD_SIZE_IN_FRAME HOP_SIZE

#define NUM_NOTES (62)

typedef struct paTestData paTestData;

//int begin_recording();

//int get_device_count();

//PaDeviceInfo get_device_info(int numDevice);

class Recorder : public QObject
{
    Q_OBJECT

public:
    Recorder();
    ~Recorder();
    int get_device_count();
    PaDeviceInfo get_device_info(int numDevice);
    int stop_recording();

public slots:
    int begin_recording();


signals:
    void UpdateNote(std::string note);
    void UpdateBars(std::string bar);

private:
    PaStream* stream;

};

#endif // RECTOOLS_H
