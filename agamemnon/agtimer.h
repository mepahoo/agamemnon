#ifndef TIMER_H
#define TIMER_H

/* nb: for now this class is linux/posix only */
  
#include <time.h>

namespace teamspeak{
namespace agamemnon{
 
class Timer
{
  public:

    Timer();
    void reset();
    int  msElapsed();
  private:
    static void getClockId();
    static bool      m_ClockIdSet;
    static clockid_t m_ClockId;
    struct timespec  m_StartTime;
};

} //namespace teamspeak
} //namespace agamemnon

#endif // TIMER_H
