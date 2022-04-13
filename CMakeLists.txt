cmake_minimum_required(VERSION 2.8.12)
project(InstrumentorTimer)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(InstrumentorTimer
        Demo.cpp
        InstrumentorMacro.h
        InstrumentorTimer.h)
