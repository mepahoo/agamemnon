#ifndef TIMER_H
#define TIMER_H

/* nb: for now this class is linux/posix only */

#ifdef _WIN32
#include <windows.h>
#else  
#include <time.h>
#endif


namespace teamspeak{
namespace agamemnon{
 
class Timer
{
  public:

    Timer();
    void reset();
    int  msElapsed();
  private:
#ifdef _WIN32
	LARGE_INTEGER    m_StartTime;
#else
    static void getClockId();
    static bool      m_ClockIdSet;
    static clockid_t m_ClockId;
    struct timespec  m_StartTime;
#endif
};

} //namespace teamspeak
} //namespace agamemnon

#endif // TIMER_H
