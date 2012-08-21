#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H


#include <vector>

namespace teamspeak{
namespace agamemnon{
  
class ConnectionPool
{
  private:
    enum PoolState{
      INACTIVE,
      CONNECTING,
      ACTIVE,
      REINITIALIZE_WAIT
    };
    
    unsigned int               m_FailMetric; //every 60 secs -1 for no error. +1 for every error. Max 60; Determines reconnect wait time. (exponential backoff)
    std::vector<unsigned int>  m_QueryTimes;
    
    
    
      
};

} //namespace teamspeak
} //namespace agamemnon

#endif // CONNECTIONPOOL_H
