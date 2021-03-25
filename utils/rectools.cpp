#include "rectools.h"
#include <math.h>
#include <QTextStream>
#include <chrono>

#include "boost/accumulators/accumulators.hpp"
#include "boost/accumulators/statistics.hpp"
#include "boost/accumulators/statistics/rolling_mean.hpp"

namespace ba = boost::accumulators;
namespace bt = ba::tag;

typedef ba::accumulator_set <smpl_t, ba::stats <bt::rolling_mean>> MeanAccumulator;
typedef std::chrono::steady_clock Clock;

MeanAccumulator acc(bt::rolling_window::window_size = WINDOW_SIZE);

void generate_midi(std::vector<std::pair<int, unsigned int>>& midi_notes);

std::string note_names[NUM_NOTES] = {   "C2", "C2#", "D2", "D2#", "E2", "F2", "F2#", "G2", "G2#", "A2", "A2#", "B2",
                                        "C3", "C3#", "D3", "D3#", "E3", "F3", "F3#", "G3", "G3#", "A3", "A3#", "B3",
                                        "C4", "C4#", "D4", "D4#", "E4", "F4", "F4#", "G4", "G4#", "A4", "A4#", "B4",
                                        "C5", "C5#", "D5", "D5#", "E5", "F5", "F5#", "G5", "G5#", "A5", "A5#", "B5",
                                        "C6", "C6#", "D6", "D6#", "E6", "F6", "F6#", "G6", "G6#", "A6", "A6#", "B6",
                                        "C7", "C7#"};

int range_low[NUM_NOTES] = {    63, 67, 71, 75, 80, 85, 90, 95, 101, 107, 113, 120,
                                127, 134, 142, 151, 160, 170, 180, 190, 202, 215, 228, 241,
                                256, 272, 287, 306, 323, 344, 364, 387, 411, 435, 461, 488,
                                518, 551, 583, 618, 654, 693, 734, 778, 825, 875, 928, 982,
                                1041, 1103, 1169, 1239, 1313, 1391, 1474, 1562, 1656, 1755, 1859, 1970,
                                2088, 2207};

int range_high[NUM_NOTES] = {   67, 71, 75, 79, 84, 89, 94, 101, 107, 113, 119, 126,
                                134, 142, 150, 159, 168, 178, 190, 200, 212, 225, 238, 251,
                                266, 282, 298, 314, 335, 353, 374, 397, 419, 445, 471, 498,
                                528, 558, 593, 626, 664, 703, 744, 788, 835, 885, 937, 992,
                                1051, 1113, 1179, 1249, 1323, 1401, 1484, 1572, 1666, 1765, 1869, 1980,
                                2098, 2227};


int isRecording = paContinue;

struct paTestData
{
    SAMPLE frameData[FRAMES_PER_BUFFER*NUM_CHANNELS] = {};
};

Recorder::Recorder() {
//    Constructor
}

Recorder::~Recorder(){
//    Destructor
}

void generate_midi(std::vector<std::pair<int, unsigned int>>& midi_notes, QString filename)
{
    std::string fname;
    smf::MidiFile midifile;
    int track = 0;
    int channel = 0;
    int instrument = 0;
    midifile.addTimbre(track, 0, channel, instrument);
    int starttick;
    int endtick = 0;
    for(const auto& i: midi_notes)
    {
        starttick = endtick;
        endtick = starttick + int(BPM*i.second/500);
        if(i.first != -1)
        {
            midifile.addNoteOn(track, starttick, channel, i.first + MIDI_KEY_OFFSET, 60);
            midifile.addNoteOff(track, endtick, channel, i.first + MIDI_KEY_OFFSET);
        }
    }
    midifile.sortTracks();
    if(filename == "")
        fname = "testing.mid";
    else
        fname = filename.toStdString();
    midifile.write(fname);
}

int find_pitch(aubio_pitch_t *o, SAMPLE ibuf[], fvec_t *pitch)
{
    fvec_t *input = new_fvec(FRAMES_PER_BUFFER);
    int i;
    for(int i = 0; i < FRAMES_PER_BUFFER; i++)
    {
        if(NUM_CHANNELS==2)
        {
            smpl_t mid;
            mid = (ibuf[i*NUM_CHANNELS] + ibuf[i*NUM_CHANNELS+1])/2;
            input->data[i] = mid;
        }
        else
            input->data[i] = (smpl_t)ibuf[i];
    }
    aubio_pitch_do(o, input, pitch);
    smpl_t freq = fvec_get_sample(pitch, 0);
    acc(freq);
    smpl_t meanFreq = ba::rolling_mean(acc);
    std::string det = "";
    del_fvec(input);
    for(i = 0; i < NUM_NOTES; i++)
    {
        if(meanFreq > range_low[i] && meanFreq < range_high[i])
        {
            det = note_names[i];
            break;
        }
    }
    if(i == NUM_NOTES)
        i = -1;
    return i;
}

static int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData)
{
    paTestData *data = (paTestData*)userData;
    const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
    long framesToCalc = framesPerBuffer;
    long i;
    int finished = isRecording;

    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;

    if(inputBuffer == NULL)
    {
        for(i = 0; i < framesToCalc; i++)
        {
            data->frameData[i*NUM_CHANNELS] = SAMPLE_SILENCE;
            if(NUM_CHANNELS == 2) data->frameData[i*NUM_CHANNELS + 1] = SAMPLE_SILENCE;
        }
    }
    else
    {
        for(i = 0; i < framesToCalc; i++)
        {
            data->frameData[i*NUM_CHANNELS] = *rptr++;
            if(NUM_CHANNELS == 2) data->frameData[i*NUM_CHANNELS + 1] = *rptr++;
        }
    }
    return finished;
}

int Recorder::begin_recording(int deviceID, bool generateMidi = false, QString filename = "testing.mid")
{
    isRecording = paContinue;
    PaStreamParameters  inputParameters;
    PaStream* stream;
    PaError err = paNoError;
    paTestData data;
    int i;
    SAMPLE val;
    int sum;
    int peak;
    float average;
    float confidence = 0;
    int prev_note = -1;
    int prev_note_final = -1;
    int std_durations[4] = {WHOLE_NOTE_DURATION, HALF_NOTE_DURATION, QUARTER_NOTE_DURATION, EIGHTH_NOTE_DURATION};

    aubio_pitch_t *o;
    smpl_t silence = -60.0;
    fvec_t *pitch;
    int note_detected = -1;
    auto durStart = Clock::now();
    auto durEnd = Clock::now();

    constexpr const std::chrono::milliseconds maxTime(MAXTIME);

    auto t1 = Clock::now();

    /*Initialize portaudio object -------------------------------------------------------------------------*/
    err = Pa_Initialize();
    if( err != paNoError)
        goto done;

    if(deviceID == -1)
    {
        inputParameters.device = Pa_GetDefaultInputDevice();
    }
    else
    {
        inputParameters.device = deviceID;
    }

    if(inputParameters.device == paNoDevice)
    {
        QTextStream(stdout) << "Error: No default input device.\n";
        goto done;
    }
    QTextStream(stdout) << "Using Device : " << Pa_GetDeviceInfo(inputParameters.device)->name << "\n";
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    /*Initialize aubio object -----------------------------------------------------------------------------*/
    o = new_aubio_pitch(FFT_METHOD, FRAMES_PER_BUFFER, HOP_SIZE, SAMPLE_RATE);
    aubio_pitch_set_unit(o, "Hz");
    aubio_pitch_set_silence(o, silence);
    pitch = new_fvec(1);

    /*Record some audio -----------------------------------------------------------------------------------*/
    err  = Pa_OpenStream(   &stream,
                            &inputParameters,
                            NULL,
                            SAMPLE_RATE,
                            FRAMES_PER_BUFFER,
                            paClipOff,
                            recordCallback,
                            &data);

    if(err != paNoError){
        goto done;
    }

    err = Pa_StartStream(stream);
    if(err != paNoError){
        goto done;
    }

    QTextStream(stdout)<<"\nNow Recording! Please speak into the microphone.\n";

    durStart = Clock::now();
    while((err = Pa_IsStreamActive(stream)) == 1)
    {
        auto t2 = Clock::now();
        sum = 0;
        for(i = 0; i < FRAMES_PER_BUFFER*NUM_CHANNELS; i++)
        {
            val = data.frameData[i];
            if(val < 0) val = -val;
                sum += val;
        }
        average = sum / (float)(FRAMES_PER_BUFFER*NUM_CHANNELS);
        peak = 100*average/pow(2.0, 14.0);
        if(peak != 0)
        {
            note_detected = find_pitch(o,data.frameData, pitch);
        }
        else
        {
            note_detected = -1;
        }
        if(note_detected == prev_note)
            confidence += 0.0005;
        else
            confidence = 0;
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1);
        if(elapsedTime > maxTime)
        {
            if(confidence > CONFIDENCE_THRESH)
            {
                if(note_detected != prev_note_final)
                {
                    int distances[4] = {0, 0, 0, 0};
                    int smallest = 0;
                    durEnd = Clock::now();
                    unsigned int duration = std::chrono::duration_cast<std::chrono::milliseconds>(durEnd - durStart).count();
                    for (int j = 0; j < 4; j++)
                    {
                        distances[j] = std::abs(std_durations[j] - int(duration));
                        if(distances[j] < distances[smallest])
                        smallest = j;
                    }
                    if(generateMidi)
                        midi_notes.push_back(std::make_pair(prev_note_final, std_durations[smallest]));
                    emit UpdateNote(note_detected);
                    prev_note_final = note_detected;
                    durStart = Clock::now();
                }
            }
            t1 = Clock::now();
        }
        prev_note = note_detected;
    }
    if(err < 0){
        goto done;
    }

    err = Pa_CloseStream(stream);
    if(err != paNoError){
        goto done;
    }

    QTextStream(stdout)<<"Done!\n";

    done:
        Pa_Terminate();
        del_aubio_pitch(o);
        del_fvec(pitch);

        if(err != paNoError)
        {
            QTextStream(stdout) << "An error occured while using the portaudio stream\n";
            QTextStream(stdout) << "Error Number: " << err << "\n";
            QTextStream(stdout) << "Error Message: "<< Pa_GetErrorText(err) << '\n';
            err = 1;
        }

        if(generateMidi)
        {
            QTextStream(stdout)<<"Generating Midi\n";
            generate_midi(midi_notes, filename);
        }

        return err;
}

void Recorder::stop_recording()
{
    isRecording = paComplete;
}

int Recorder::get_device_count()
{
    int numDevices = 0;
    PaError err;

    err = Pa_Initialize();
    if(err != paNoError)
    {
        goto error;
    }

    numDevices = Pa_GetDeviceCount();

    error:
        Pa_Terminate();

    return numDevices;
}

PaDeviceInfo Recorder::get_device_info(int numDevice)
{
    PaError err;
    const PaDeviceInfo *deviceInfo = NULL;
    PaDeviceInfo dev_info;

    err = Pa_Initialize();
    if(err != paNoError)
    {
        goto error;
    }


    deviceInfo = Pa_GetDeviceInfo(numDevice);
    dev_info.name = deviceInfo->name;
    if(numDevice == Pa_GetDefaultInputDevice())
    {
        std::string name = "[ Default ]";
        name += dev_info.name;
        dev_info.name = name.c_str();
    }
    dev_info.maxInputChannels = deviceInfo->maxInputChannels;
    dev_info.maxOutputChannels = deviceInfo->maxOutputChannels;
    dev_info.defaultLowInputLatency = deviceInfo->defaultLowInputLatency;
    dev_info.defaultLowOutputLatency = deviceInfo->defaultLowOutputLatency;
    dev_info.defaultHighInputLatency = deviceInfo->defaultHighInputLatency;
    dev_info.defaultHighOutputLatency = deviceInfo->defaultHighOutputLatency;
    dev_info.defaultSampleRate = deviceInfo->defaultSampleRate;
    dev_info.hostApi = deviceInfo->hostApi;
    dev_info.structVersion = deviceInfo->structVersion;

    error:
        Pa_Terminate();

    return dev_info;
}

//std::vector<std::pair<int, unsigned int>> Recorder::get_midi_notes()
//{
//    return midi_notes;
//}

