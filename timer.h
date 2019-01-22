#ifndef __INC_TIMER_H
#define __INC_TIMER_H
#include <stdint.h>

template<uint32_t (*GET_TIME)(void)> class Timer {
  uint32_t last;
  uint32_t time_length;
  bool bLoop;
  bool bRunning;

public:
  Timer() : time_length(0), bRunning(false) {}
  Timer(uint32_t len) : time_length(len), bRunning(false) {}

  void start(uint32_t new_time,  bool loop) {
    time_length = new_time;
    start(loop);
  }

  void start(bool loop = false) {
    last = GET_TIME();
    bLoop = loop;
    bRunning = true;
  }

  uint32_t sinceStart() {
    return GET_TIME() - last;
  }

  uint32_t untilDone() {
    return last + time_length - GET_TIME();
  }


  // The percent completed, represented as a 0-255 value suitable for scale8
  uint8_t perc8() {
    if(bRunning) {
      return (sinceStart() * 256) / time_length;
    }
    return 0;
  }

  // The percent completed, represented as a 0-65535 value suitable for scale8
  uint16_t perc16() {
    if(bRunning) {
      return (sinceStart() * 65536) / time_length;
    }
    return 0;
  }

  void stop() {
    bRunning = false;
  }

  // Has this timer tripped/fired since the last time we checked it?
  bool fired() {
    if(bRunning) {
      uint32_t curtime = GET_TIME();
      if((curtime - last) >= time_length) {
        bRunning = bLoop;
        last = curtime;
        return true;
      }
    }
    return false;
  }

  bool running() {
    return bRunning;
  }
};



typedef Timer<micros> MicrosTimer;
typedef Timer<millis> MillisTimer;

#endif

