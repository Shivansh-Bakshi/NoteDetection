#include "utils/rectools.h"
#include <iostream>
// #include <chrono>
// #include "portaudio.h"
// typedef std::chrono::steady_clock Clock;

// constexpr const std::chrono::milliseconds maxTime(1000);

int main()
{
    // auto t1 = Clock::now();
    // int counter = 0;
    // while(true)
    // {
    //     // Pa_Sleep(1000);
    //     auto t2 = Clock::now();
    //     auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1);
    //     if(elapsedTime > maxTime)
    //     {
    //         std::cout << elapsedTime.count() << "ms"<< std::endl;
    //         std::cout<<"true"<<std::endl;
    //         t1 = Clock::now();
    //         counter++;
    //     }
    //     if(counter > 5)
    //     {
    //         break;
    //     }
    // }
    // int device_count;
    // device_count = get_device_count();
    // PaDeviceInfo deviceInfo;

    // std::cout << device_count << std::endl;

    // for(int i = 0; i < device_count; i++)
    // {
    //     std::cout << "-------------------------Device No: " << i << std::endl;
    //     deviceInfo = get_device_info(i);
    //     std::cout << "Name = " << deviceInfo.name << std::endl;
    //     printf( "Max inputs = %d", deviceInfo.maxInputChannels  );
    //     printf( ", Max outputs = %d\n", deviceInfo.maxOutputChannels  );
    //     printf( "Default low input latency   = %8.4f\n", deviceInfo.defaultLowInputLatency  );
    //     printf( "Default low output latency  = %8.4f\n", deviceInfo.defaultLowOutputLatency  );
    //     printf( "Default high input latency  = %8.4f\n", deviceInfo.defaultHighInputLatency  );
    //     printf( "Default high output latency = %8.4f\n", deviceInfo.defaultHighOutputLatency  );
    // }

    int record;
    record = begin_recording();

    
}