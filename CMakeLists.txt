cmake_minimum_required(VERSION 3.21)
project(InstrumentorTimer)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(InstrumentorTimer
        Demo.cpp
        InstrumentorMacro.h
        InstrumentorTimer.h)
