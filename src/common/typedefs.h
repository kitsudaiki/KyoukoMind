#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <chrono>

typedef std::chrono::microseconds  chronoMicroSec;
typedef std::chrono::nanoseconds  chronoNanoSec;
typedef std::chrono::seconds  chronoSec;
typedef std::chrono::high_resolution_clock::time_point chronoTimePoint;
typedef std::chrono::high_resolution_clock chronoClock;

typedef uint32_t ClusterID;

#endif // TYPEDEFS_H
