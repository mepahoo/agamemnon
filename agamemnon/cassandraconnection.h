#ifndef CASSANDRACONNECTION_H
#define CASSANDRACONNECTION_H

#include <boost/shared_ptr.hpp>
#include <thrift/async/TAsyncChannel.h>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "agamemnon_types.h"
#include "agtimer.h"
#include "agcqlqueryresult.h"
#include "agpreparedquery.h"
#include "agglobalpreparedquerycache.h"

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
    void executeCQL(const std::string& cql, bool doCompress, ErrorFunction errorFunc, boost::function<void(CQLQueryResult::Ptr)> callback);
    
    void prepareQuery(const std::string& cql, bool doCompress, ErrorFunction errorFunc, boost::function<void(GlobalPreparedQueryCache::PreparedQueryInfo::Ptr)> callback);
    void executePreparedQuery(const PreparedQuery::Ptr & fieldData, bool doCompress, ErrorFunction errorFunc, boost::function<void(CQLQueryResult::Ptr)> callback);
    
    void setNeedToCloseWhenDone();
  private:
    typedef std::map<GlobalPreparedQueryCache::PreparedQueryInfo::Ptr, int> PreparedQueryMap;
    
    void setKeyspace_done(ErrorFunction errorFunc, boost::function<void()> callback);
    void setCQLVersion_done(ErrorFunction errorFunc, boost::function<void()> callback);
    void getClusterName_done(ErrorFunction errorFunc, boost::function<void(const std::string&)> callback);
    void executeCQL_done(ErrorFunction errorFunc, boost::function<void(CQLQueryResult::Ptr)> callback);
    void prepareQuery_done(ErrorFunction errorFunc, const std::string& cql, boost::function<void(GlobalPreparedQueryCache::PreparedQueryInfo::Ptr)> callback);
    void executePreparedQuery_done(ErrorFunction errorFunc, boost::function<void(CQLQueryResult::Ptr)> callback);

    AgCassandraCobClient* m_AgCassandraCobClient;
    std::string           m_Host;
    bool                  m_NeedToCloseWhenDone;
    Timer                 m_Timer;
    PreparedQueryMap      m_PreparedQueryMap;
};

} //namespace teamspeak
} //namespace agamemnon
#endif // CASSANDRACONNECTION_H
