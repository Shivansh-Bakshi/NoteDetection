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

std::string note_names[NUM_NOTES] = {  "C2", "C2#", "D2", "D2#", "E2", "F2", "F2#", "G2", "G2#", "A2", "A2#", "B2",
                                "C3", "C3#", "D3", "D3#", "E3", "F3", "F3#", "G3", "G3#", "A3", "A3#", "B3",
                                "C4", "C4#", "D4", "D4#", "E4", "F4", "F4#", "G4", "G4#", "A4", "A4#", "B4",
                                "C5", "C5#", "D5", "D5#", "E5", "F5", "F5#", "G5", "G5#", "A5", "A5#", "B5",
                                "C6", "C6#", "D6", "D6#", "E6", "F6", "F6#", "G6", "G6#", "A6", "A6#", "B6",
                                "C7", "C7#"};

int range_low[NUM_NOTES] = {   63, 67, 72, 76, 80, 85, 90, 95, 101, 107, 113, 120,
                        127, 135, 144, 151, 163, 171, 182, 193, 204, 217, 230, 243,
                        259, 274, 290, 308, 329, 347, 367, 389, 413, 437, 463, 492,
                        522, 552, 587, 620, 660, 698, 739, 783, 831, 880, 930, 987,
                        1047, 1107, 1174, 1244, 1319, 1396, 1484, 1571, 1667, 1768, 1877, 1967,
                        2106, 2236};

int range_high[NUM_NOTES] = {  67, 71, 76, 79, 84, 89, 94, 101, 107, 113, 119, 126,
                        134, 143, 150, 159, 167, 177, 189, 200, 212, 223, 236, 251,
                        265, 280, 297, 314, 335, 353, 373, 395, 419, 443, 469, 498,
                        528, 558, 593, 626, 666, 704, 745, 789, 837, 890, 940, 995,
                        1053, 1115, 1181, 1251, 1328, 1404, 1491, 1579, 1675, 1776, 1885, 1975,
                        2114, 2244};


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

std::string find_pitch(aubio_pitch_t *o, SAMPLE ibuf[], fvec_t *pitch)
{
    fvec_t *input = new_fvec(FRAMES_PER_BUFFER);
    int i;
    for(int i = 0; i < FRAMES_PER_BUFFER; i++)
    {
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
    return det;
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

int Recorder::begin_recording()
{
    isRecording = paContinue;
    PaStreamParameters  inputParameters;
    PaStream* stream;
    PaError err = paNoError;
    paTestData data;
    int i;
    SAMPLE val;
    long sum;
    int peak;
    float average;

    aubio_pitch_t *o;
    smpl_t silence = -60.0;
    fvec_t *pitch;
    std::string note_detected = "";

    constexpr const std::chrono::milliseconds maxTime(100);

    auto t1 = Clock::now();


    /*Initialize portaudio object -------------------------------------------------------------------------*/
    err = Pa_Initialize();
    if( err != paNoError)
        goto done;

    inputParameters.device = Pa_GetDefaultInputDevice();
    if(inputParameters.device == paNoDevice)
    {
        QTextStream(stdout) << "Error: No default input device.\n";
        goto done;
    }
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
        peak = 100*average/pow(2.0, 16.0);
        if(peak != 0)
        {
            note_detected = find_pitch(o,data.frameData, pitch);
        }
        else
        {
            note_detected = "";
        }
        average = sum / (float)(FRAMES_PER_BUFFER*NUM_CHANNELS);
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1);
        if(elapsedTime > maxTime)
        {
            std::string bars(peak, '#');
            emit UpdateNote(note_detected);
            t1 = Clock::now();
        }

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


