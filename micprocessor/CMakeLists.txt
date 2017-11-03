cmake_minimum_required(VERSION 2.8)
project(mic_processor)

find_package(qibuild)
find_package(CURL REQUIRED) 

# set C++14 flags, ATOM CPU, and link static to libstdc++ (for C++14)
set(CMAKE_CXX_FLAGS 
    "-std=gnu++1y -static-libstdc++ -march=atom -mtune=atom -mfpmath=sse")

option(SOUNDPROCESSING_IS_REMOTE
      "module is compiled as a remote module (ON or OFF)"
        ON)
# Create a executable named mic_processor
# with the source file: main.cpp
qi_create_bin(mic_processor main.cpp 
                            alsoundprocessing.cpp 
                            speech_processor.cpp
                            AudioFile/AudioFile.cpp)

# link with
qi_use_lib(mic_processor 
           ALCOMMON 
           ALPROXIES 
           ALAUDIO 
           QI 
           boost_system 
           boost_thread 
           pthread
           curl)
