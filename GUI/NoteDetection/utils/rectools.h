#ifndef RECTOOLS_H
#define RECTOOLS_H

#include "portaudio.h"
#include "aubio/aubio.h"
#include "MidiFile.h"
#include "ui_detectormain.h"
#include <string>
#include <QMainWindow>
#include <utility>

#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (4096)
#define NUM_CHANNELS (2)
#define DITHER_FLAG (0)

#define PA_SAMPLE_TYPE paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE (0)
#define PRINTF_S_FORMAT "%d"

#define FFT_METHOD "default"
#define HOP_SIZE (int)(FRAMES_PER_BUFFER/2)
#define PERIOD_SIZE_IN_FRAME HOP_SIZE

#define NUM_NOTES (62)          // For Classification
#define MIDI_KEY_OFFSET (36)    // For saving midi file
#define BPM (112)               // Beats per minute

//Tunable Parameters
#define WINDOW_SIZE (10)        // For rolling mean
#define CONFIDENCE_THRESH (0.875)
#define MAXTIME (100)
//Tunable Parameters

#define WHOLE_NOTE_DURATION (int(60000*4/BPM))
#define HALF_NOTE_DURATION (int(WHOLE_NOTE_DURATION/2))
#define QUARTER_NOTE_DURATION (int(HALF_NOTE_DURATION/2))
#define EIGHTH_NOTE_DURATION (int(QUARTER_NOTE_DURATION/2))

typedef struct paTestData paTestData;

extern std::string note_names[NUM_NOTES];

class Recorder : public QObject
{
    Q_OBJECT

public:
    Recorder();
    ~Recorder();
    int get_device_count();
    PaDeviceInfo get_device_info(int numDevice);
    void stop_recording();
//    std::vector<std::pair<int, unsigned int>> get_midi_notes();
//    void generate_midi(std::vector<std::pair<int, unsigned int>>& midi_notes, std::string filename);

public slots:
    int begin_recording(int deviceID, bool generateMidi);

signals:
    void UpdateNote(int note);

private:
    std::vector<std::pair<int, unsigned int>> midi_notes;

};

#endif // RECTOOLS_H
