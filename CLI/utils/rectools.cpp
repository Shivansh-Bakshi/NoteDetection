#include "rectools.h"
#include <string>
#include <iostream>
#include <math.h>
// #include <cmath>
#include <chrono>
#include "MidiFile.h"


#include "boost/accumulators/accumulators.hpp"
#include "boost/accumulators/statistics.hpp"
#include "boost/accumulators/statistics/rolling_mean.hpp"

namespace ba = boost::accumulators;
namespace bt = ba::tag;

typedef ba::accumulator_set <smpl_t, ba::stats <bt::rolling_mean>> MeanAccumulator;
typedef std::chrono::steady_clock Clock;

MeanAccumulator acc(bt::rolling_window::window_size = WINDOW_SIZE);

std::string note_names[NUM_NOTES] = { "C2", "C2#", "D2", "D2#", "E2", "F2", "F2#", "G2", "G2#", "A2", "A2#", "B2",
                                "C3", "C3#", "D3", "D3#", "E3", "F3", "F3#", "G3", "G3#", "A3", "A3#", "B3",
                                "C4", "C4#", "D4", "D4#", "E4", "F4", "F4#", "G4", "G4#", "A4", "A4#", "B4",
                                "C5", "C5#", "D5", "D5#", "E5", "F5", "F5#", "G5", "G5#", "A5", "A5#", "B5",
                                "C6", "C6#", "D6", "D6#", "E6", "F6", "F6#", "G6", "G6#", "A6", "A6#", "B6",
                                "C7", "C7#"};

int range_low[NUM_NOTES] = {   63, 67, 71, 75, 80, 85, 90, 95, 101, 107, 113, 120,
                        127, 134, 142, 151, 160, 170, 180, 190, 202, 215, 228, 241,
                        256, 272, 287, 306, 323, 344, 364, 387, 411, 435, 461, 488,
                        518, 551, 583, 618, 654, 693, 734, 778, 825, 875, 928, 982,
                        1041, 1103, 1169, 1239, 1313, 1391, 1474, 1562, 1656, 1755, 1859, 1970,
                        2088, 2207};

int range_high[NUM_NOTES] = {  67, 71, 75, 79, 84, 89, 94, 101, 107, 113, 119, 126,
                        134, 142, 150, 159, 168, 178, 190, 200, 212, 225, 238, 251,
                        266, 282, 298, 314, 335, 353, 374, 397, 419, 445, 471, 498,
                        528, 558, 593, 626, 664, 703, 744, 788, 835, 885, 937, 992,
                        1051, 1113, 1179, 1249, 1323, 1401, 1484, 1572, 1666, 1765, 1869, 1980,
                        2098, 2227};

struct paTestData
{
    int frameIndex;
    int maxFrameIndex;
    SAMPLE frameData[FRAMES_PER_BUFFER*NUM_CHANNELS] = {};
};

void generate_midi(std::vector<std::pair<int, unsigned int>> notes)
{
    smf::MidiFile midifile;
    int track = 0;
    int channel = 0;
    int instrument = 0;
    midifile.addTimbre(track, 0, channel, instrument);
    int tpq = midifile.getTPQ();
    int bpm = 112;
    int starttick;
    int endtick = 0;
    for(const auto i: notes)
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
    midifile.write(FILENAME);
}


// smpl_t find_pitch(aubio_pitch_t *o, SAMPLE ibuf[], fvec_t *pitch)
int find_pitch(aubio_pitch_t *o, SAMPLE ibuf[], fvec_t *pitch)
{
    fvec_t *input = new_fvec(FRAMES_PER_BUFFER);
    int i;
    for(int i = 0; i < FRAMES_PER_BUFFER; i+=2)
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
    // return meanFreq;
}

static int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData)
{
   paTestData *data = (paTestData*)userData;
   const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
   long framesToCalc;
   long i;
   int finished;
   unsigned long framesLeft =  data->maxFrameIndex - data->frameIndex;

   (void) outputBuffer;
   (void) timeInfo;
   (void) statusFlags;
   (void) userData;

   if( framesLeft < framesPerBuffer)
   {
       framesToCalc = framesLeft;
       finished = paComplete;
   }
   else
   {
       framesToCalc = framesPerBuffer;
       finished = paContinue;
   }

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
   data->frameIndex += framesToCalc;
   return finished;
}

int begin_recording()
{
PaStreamParameters  inputParameters,
                        outputParameters;
    
    PaStream* stream;
    PaError err = paNoError;
    paTestData data;
    int i;
    int totalFrames;
    SAMPLE val;
    int sum;
    int peak;
    float average;
    float confidence = 0;
    int prev_note = -1;
    int prev_note_final = -1;
    int std_durations[4] = {WHOLE_NOTE_DURATION, HALF_NOTE_DURATION, QUARTER_NOTE_DURATION, EIGHTH_NOTE_DURATION};
    // std::string prev_note = "";
    // std::string prev_note_final = "";

    aubio_pitch_t *o;
    smpl_t silence = -60.0;
    fvec_t *pitch;
    int note_detected = -1;
    // std::string note_detected = "";
    // unsigned int duration = 0;
    auto durStart = Clock::now();
    auto durEnd = Clock::now();

    std::vector<std::pair<int, unsigned int>> notes;

    constexpr const std::chrono::milliseconds maxTime(MAXTIME);

    std::cout<<"Testing recording"<<std::endl;

    data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE;
    data.frameIndex = 0;
    auto t1 = Clock::now();
    /*Initialize portaudio object -------------------------------------------------------------------------*/
    err = Pa_Initialize();
    if( err != paNoError)
        goto done;
    
    inputParameters.device = Pa_GetDefaultInputDevice();
    // inputParameters.device = 2;

    if(inputParameters.device == paNoDevice)
    {
        std::cerr << "Error: No default input device.\n";
        goto done;
    }
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    std::cout<<"Using device: " << Pa_GetDeviceInfo(inputParameters.device)->name << std::endl; 
    /*Initialize aubio object -----------------------------------------------------------------------------*/
    o = new_aubio_pitch(METHOD, FRAMES_PER_BUFFER, HOP_SIZE, SAMPLE_RATE);
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

    std::cout<<"\nNow Recording! Please speak into the microphone." << std::endl;
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
        peak = 100*average/pow(2.0, 13.0);
        
        if(peak != 0)
        {
            note_detected = find_pitch(o,data.frameData, pitch);
        }
        else
        {
            note_detected = -1;  
            // note_detected = 0;   
        }
        if(note_detected == prev_note)
            confidence += 0.0005;
        else
            confidence = 0;

        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1);
        if(elapsedTime > maxTime)
        {
            // std::string bars(peak, '#');
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
                    std::cout << "prev_note: "<< prev_note_final << "  " << "\tduration: " << duration;
                    std::cout << "\tconfidence: " << confidence << "set duration: " << std_durations[smallest] << std::endl;
                    notes.push_back(std::make_pair(prev_note_final, std_durations[smallest]));
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

    std::cout<<"Done!"<<std::endl;

    done:
        Pa_Terminate();
        del_aubio_pitch(o);
        del_fvec(pitch);
        
        if(err != paNoError)
        {
            std::cerr << "An error occured while using the portaudio stream\n";
            std::cerr << "Error Number: " << err << "\n";
            std::cerr << "Error Message: "<< Pa_GetErrorText(err) << '\n';
            err = 1;

        }

        std::cout<<"Printing contents of vector:\n"<<std::endl;
        for(const auto i: notes)
        {
            std::cout<<"Note: " << i.first << "\tDuration: " << i.second << std::endl;
        }

        generate_midi(notes);

        return err;
}

int get_device_count()
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

PaDeviceInfo get_device_info(int numDevice)
{
    PaError err;
    const PaDeviceInfo *deviceInfo = NULL;
    PaDeviceInfo dev_info;

    err = Pa_Initialize();
    if(err != paNoError)
    {
        std::cout << "Error" << std::endl;
        goto error;
    }

    deviceInfo = Pa_GetDeviceInfo(numDevice);
    if(numDevice == Pa_GetDefaultInputDevice())
    {
        std::cout<< "[ Default Input ]" << std::endl;
    }
    if(numDevice == Pa_GetDefaultOutputDevice())
    {
        std::cout<< "[ Default Output ]" << std::endl;
    }
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
        std::cout << "Done" << std::endl;
        Pa_Terminate();

    return dev_info;
}


