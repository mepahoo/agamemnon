#include "agtimer.h"

#include <iostream>
#include <stdlib.h>

namespace teamspeak{
namespace agamemnon{

bool      Timer::m_ClockIdSet = false;
clockid_t Timer::m_ClockId = 0;

Timer::Timer()
{
  if (!m_ClockIdSet) getClockId();
  reset();
}

void Timer::reset()
{
  int res = clock_gettime(m_ClockId, & m_StartTime);
  if (res) {
    std::cerr <<"error during clock_gettime"<<std::endl;
    exit(1);
  }
}

int  Timer::msElapsed()
{
  struct timespec  currentTime;
  int res = clock_gettime(m_ClockId, & currentTime);
  if (res) {
    std::cerr <<"error during clock_gettime"<<std::endl;
    exit(1);
  }
  
  if (currentTime.tv_sec < m_StartTime.tv_sec) return -1;
  time_t delta = currentTime.tv_sec - m_StartTime.tv_sec;
  delta *= 1000; //ms
  
  long ndelta = currentTime.tv_nsec - m_StartTime.tv_nsec;
  ndelta /=1000000; //ms
  
  if (ndelta < 0) {
    ndelta += 1000000000;
    delta -= 1000;
  }
  ndelta += delta;
  return ndelta;
}

/*static*/
void Timer::getClockId()
{
  struct timespec  currentTime;
  int res;
  clockid_t clockid;

#ifdef CLOCK_MONOTONIC_COARSE
  clockid = CLOCK_MONOTONIC_COARSE;
  res = clock_gettime(clockid, & currentTime);
  if (!res) goto success;
#endif

#ifdef CLOCK_MONOTONIC_RAW
  clockid = CLOCK_MONOTONIC_RAW;
  res = clock_gettime(clockid, & currentTime);
  if (!res) goto success;
#endif

#ifdef _POSIX_MONOTONIC_CLOCK
  clockid = CLOCK_MONOTONIC;
  res = clock_gettime(clockid, & currentTime);
  if (!res) goto success;
#endif

  clockid = CLOCK_REALTIME;
  res = clock_gettime(clockid, & currentTime);
  if (!res) goto success;
  
  std::cerr<<"Could not find a working clock" << std::endl;
  exit(1);
  
success:
  std::cerr<<"using clock id" << clockid << std::endl;
  m_ClockId = clockid;
  m_ClockIdSet = true;
}

  
} //namespace teamspeak
} //namespace agamemnon
