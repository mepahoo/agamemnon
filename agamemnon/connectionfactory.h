#ifndef CONNECTIONFACTORY_H
#define CONNECTIONFACTORY_H

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include "cassandraconnection.h"
#include "agamemnon_types.h"
#include <vector>

namespace teamspeak{
namespace agamemnon{

struct ConnectionCommonSettings
{
  ConnectionCommonSettings() : port(9160), keyspace(), CQLVersion("3.0.0"), retryDelayOnTimeoutms(5000), connectTimeoutms(1000), ioTimeoutms(1000), replyTimeoutms(5000), maxConnectionReuseCount(100), maxRetryCount(3), compressCQL(true){}
  uint16_t    port;
  std::string keyspace;
  std::string CQLVersion;
  int         retryDelayOnTimeoutms;
  int         connectTimeoutms;
  int         ioTimeoutms;
  int         replyTimeoutms;
  int         maxConnectionReuseCount;
  int         maxRetryCount;
  bool        compressCQL;
};

class ConnectionFactory
{
  public:
    
    struct RequestQueueItem{
      RequestQueueItem():errorFunc(),callback(){}
      RequestQueueItem(ErrorFunction _errorFunc, boost::function<void(CassandraConnection::Ptr)> _callback)
	:errorFunc(_errorFunc)
	,callback(_callback)
      {
      }
      
      ErrorFunction errorFunc;
      boost::function<void(CassandraConnection::Ptr)> callback;
    };
    
    
    typedef boost::shared_ptr<ConnectionFactory> Ptr;
    
    virtual ~ConnectionFactory(){}
    
    virtual void getConnection(ErrorFunction errorFunc, boost::function<void(CassandraConnection::Ptr)> callback) = 0;
    virtual int  retryDelayOnTimeout() const = 0;
    virtual int  maxRetryCount() const = 0;
    virtual bool compressCQL() const = 0;
};

} //namespace teamspeak
} //namespace agamemnon
#endif // CONNECTIONFACTORY_H
