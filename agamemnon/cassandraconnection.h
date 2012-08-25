#ifndef CASSANDRACONNECTION_H
#define CASSANDRACONNECTION_H

#include <boost/shared_ptr.hpp>
#include <thrift/async/TAsyncChannel.h>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "agamemnon_types.h"
#include "agtimer.h"
#include "agcqlqueryresult.h"

namespace teamspeak{
namespace agamemnon{

//forward ref
class AgCassandraCobClient;

  
class CassandraConnectionChannel;

class CassandraConnection : public boost::enable_shared_from_this<CassandraConnection>
{
  public:
    typedef boost::shared_ptr<CassandraConnection> Ptr;
    
    CassandraConnection(AgCassandraCobClient* con);
    ~CassandraConnection();
    
    void setKeyspace(const std::string& keyspace, ErrorFunction errorFunc, boost::function<void()> callback);
    void setCQLVersion(const std::string& version, ErrorFunction errorFunc, boost::function<void()> callback);
    
    const std::string& getHost() const {return m_Host;}
    const bool needToCloseWhenDone() const { return m_NeedToCloseWhenDone;}
    
    void getClusterName(ErrorFunction errorFunc, boost::function<void(const std::string&)> callback);
    void executeCQL(const std::string& cql, bool compress, ErrorFunction errorFunc, boost::function<void(CQLQueryResult::Ptr)> callback);
    
    void setNeedToCloseWhenDone();
  private:
    void setKeyspace_done(ErrorFunction errorFunc, boost::function<void()> callback);
    void setCQLVersion_done(ErrorFunction errorFunc, boost::function<void()> callback);
    void getClusterName_done(ErrorFunction errorFunc, boost::function<void(const std::string&)> callback);
    void executeCQL_done(ErrorFunction errorFunc, boost::function<void(CQLQueryResult::Ptr)> callback);
    
    AgCassandraCobClient* m_AgCassandraCobClient;
    std::string           m_Host;
    bool                  m_NeedToCloseWhenDone;
    Timer                 m_Timer;
};

} //namespace teamspeak
} //namespace agamemnon
#endif // CASSANDRACONNECTION_H
