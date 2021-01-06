#include <iostream>
#include <string>
#include <math.h>

#include "aubio/aubio.h"
#include "portaudio.h"

#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (2048)
#define NUM_SECONDS (5)
#define NUM_CHANNELS (1)
#define DITHER_FLAG (0)

#define PA_SAMPLE_TYPE paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE (0)
#define PRINTF_S_FORMAT "%d"

#define METHOD "default"
#define HOP_SIZE (int)(FRAMES_PER_BUFFER/2)
#define PERIOD_SIZE_IN_FRAME HOP_SIZE

typedef struct
{
    int frameIndex;
    int maxFrameIndex;
    SAMPLE frameData[FRAMES_PER_BUFFER*NUM_CHANNELS] = {};
    SAMPLE *recordedSamples;
}
paTestData;

smpl_t find_pitch(aubio_pitch_t *o, SAMPLE ibuf[], fvec_t *pitch)
{   
    fvec_t *input = new_fvec(FRAMES_PER_BUFFER);
    for(int i = 0; i < FRAMES_PER_BUFFER; i++)
    {
        input->data[i] = (smpl_t)ibuf[i];
    }
    aubio_pitch_do(o, input, pitch);
    smpl_t freq = fvec_get_sample(pitch, 0);
    del_fvec(input);
    return freq;
}


static int recordCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData)
{
    paTestData *data = (paTestData*)userData;
    const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
    SAMPLE *wptr = &data->recordedSamples[data->frameIndex*NUM_CHANNELS];
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
            *wptr++ = data->frameData[i*NUM_CHANNELS] = SAMPLE_SILENCE;
            if(NUM_CHANNELS == 2) *wptr++ = data->frameData[i*NUM_CHANNELS + 1] = SAMPLE_SILENCE;
        }
    }
    else
    {
        for(i = 0; i < framesToCalc; i++)
        {
            *wptr++ = data->frameData[i*NUM_CHANNELS] = *rptr++;
            if(NUM_CHANNELS == 2) *wptr++ = data->frameData[i*NUM_CHANNELS + 1] = *rptr++;
        }
    }
    data->frameIndex += framesToCalc;
    return finished;
}

int main()
{
    PaStreamParameters  inputParameters,
                        outputParameters;
    
    PaStream* stream;
    PaError err = paNoError;
    paTestData data;
    int i;
    int totalFrames;
    int numSamples;
    int numBytes;
    SAMPLE val;
    long sum;
    int peak;
    float average;

    aubio_pitch_t *o;
    smpl_t silence = -60.0;
    fvec_t *pitch;

    std::cout<<"Testing recording"<<std::endl;

    data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE;
    data.frameIndex = 0;
    numSamples = totalFrames*NUM_CHANNELS;
    numBytes = numSamples * sizeof(SAMPLE);
    data.recordedSamples = (SAMPLE *)malloc(numBytes);
    if(data.recordedSamples == NULL)
    {
        std::cout<<"Could not allocate record array" << std::endl;
        goto done;
    }

    for(i = 0; i < numSamples; i++)
        data.recordedSamples[i] = 0;

    /*Initialize portaudio object -------------------------------------------------------------------------*/
    err = Pa_Initialize();
    if( err != paNoError)
        goto done;
    
    inputParameters.device = Pa_GetDefaultInputDevice();
    if(inputParameters.device == paNoDevice)
    {
        std::cerr << "Error: No default input device.\n";
        goto done;
    }
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    /*Initialize aubio object -----------------------------------------------------------------------------*/
    o = new_aubio_pitch(METHOD, FRAMES_PER_BUFFER, HOP_SIZE, SAMPLE_RATE);
    aubio_pitch_set_unit(o, "Hz");
    aubio_pitch_set_silence(o, silence);
    pitch = new_fvec(1);
    smpl_t freq_detected;


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

    while((err = Pa_IsStreamActive(stream)) == 1)
    {
        Pa_Sleep(10);
        sum = 0;
        for(i = 0; i < FRAMES_PER_BUFFER*NUM_CHANNELS; i++)
        {
            val = data.frameData[i];
            if(val < 0) val = -val;
            sum += val;
        }
        average = sum / (float)(FRAMES_PER_BUFFER*NUM_CHANNELS);
        freq_detected = find_pitch(o,data.frameData, pitch);
        peak = 100*average/pow(2.0, 16.0);
        std::string bars(peak, '#');
        std::cout << freq_detected << "\t" << bars << std::endl;
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
        if(data.recordedSamples)
            free(data.recordedSamples);
        
        if(err != paNoError)
        {
            std::cerr << "An error occured while using the portaudio stream\n";
            std::cerr << "Error Number: " << err << "\n";
            std::cerr << "Error Message: "<< Pa_GetErrorText(err) << '\n';
            err = 1;

        }
        return err;
}