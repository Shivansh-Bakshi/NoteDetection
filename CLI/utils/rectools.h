#ifndef RECTOOLS_H
#define RECTOOLS_H

#include "portaudio.h"
#include "aubio/aubio.h"
#include <string>

#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (4096)
#define NUM_CHANNELS (2)
#define DITHER_FLAG (0)
#define NUM_SECONDS (30)

/*  Tunable Params   */
#define WINDOW_SIZE (10)
#define CONFIDENCE_THRESH (0.885)
#define MAXTIME (100)
/*  Tunable Params   */
#define FILENAME "tests/WS_10_CT_885_MT_100.mid"

#define PA_SAMPLE_TYPE paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE (0)
#define PRINTF_S_FORMAT "%d"

#define METHOD "default"
#define HOP_SIZE (int)(FRAMES_PER_BUFFER/2)
#define PERIOD_SIZE_IN_FRAME HOP_SIZE

#define NUM_NOTES (62)
#define MIDI_KEY_OFFSET (36)
#define BPM (112)
#define WHOLE_NOTE_DURATION (int(60000*4/BPM))
#define HALF_NOTE_DURATION (int(WHOLE_NOTE_DURATION/2))
#define QUARTER_NOTE_DURATION (int(HALF_NOTE_DURATION/2))
#define EIGHTH_NOTE_DURATION (int(QUARTER_NOTE_DURATION/2))

typedef struct paTestData paTestData;

int begin_recording();

int get_device_count();

PaDeviceInfo get_device_info(int numDevice);

// void generate_midi(std::vector<std::pair<int, unsigned int>>);

#endif // RECTOOLS_H
