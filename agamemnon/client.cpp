
#ifdef _WIN32
#include <windows/TargetVersion.h>
#endif

#include "client.h"
#include "cassandraconnection.h"
#include <boost/bind.hpp>
#include <thrift/transport/TTransportException.h>
#include "../thriftcassandra/cassandra_types.h"

namespace teamspeak{
namespace agamemnon{

Client::Client(boost::asio::io_service& IOService, ConnectionFactory::Ptr& connectionFactory)
: boost::enable_shared_from_this<Client>()
, m_ConnectionFactory(connectionFactory)
, m_Connection()
, m_RetryCount(0)
, m_Timer(IOService)
{
}

void Client::getClusterName(ErrorFunction errorFunc, boost::function<void(const std::string&)> callback)
{
  boost::function<void()> retryFunc =
    boost::bind(&Client::getClusterName, shared_from_this(), errorFunc, callback);

  ErrorFunction retryErrorFunc =
    boost::bind(&Client::generic_RetryError, shared_from_this(), _1, errorFunc, retryFunc);
      
  boost::function<void(CassandraConnection::Ptr connection)> requestClusterNameFunc =
    boost::bind( &CassandraConnection::getClusterName, _1, retryErrorFunc, callback);
    
  ensureConnection(retryErrorFunc, requestClusterNameFunc);
}

void Client::prepareQuery(const std::string& cql, ErrorFunction errorFunc, boost::function<void(PreparedQuery::Ptr)> callback)
{
  GlobalPreparedQueryCache::PreparedQueryInfo::Ptr queryInfo = GlobalPreparedQueryCache::getInstance()->getPreparedQueryInfo(cql);
  if (queryInfo.get() != 0){
    callback(PreparedQuery::Ptr(new PreparedQuery(queryInfo)));
  }
  
  //else, we must prepare to get info
  boost::function<void()> retryFunc =
    boost::bind(&Client::prepareQuery, shared_from_this(), cql, errorFunc, callback);

  ErrorFunction retryErrorFunc =
    boost::bind(&Client::generic_RetryError, shared_from_this(), _1, errorFunc, retryFunc);
  
  boost::function<void(GlobalPreparedQueryCache::PreparedQueryInfo::Ptr)> transformCallback =
    boost::bind(&Client::prepareQuery_done, shared_from_this(), _1, callback);
      
  boost::function<void(CassandraConnection::Ptr connection)> prepareQueryFunc =
    boost::bind( &CassandraConnection::prepareQuery, _1, cql, m_ConnectionFactory->compressCQL(), retryErrorFunc, transformCallback );
    
  ensureConnection(retryErrorFunc, prepareQueryFunc);
}

void Client::excecuteCQL(const std::string& cql, ErrorFunction errorFunc, boost::function<void(CQLQueryResult::Ptr)> callback)
{
 boost::function<void()> retryFunc =
    boost::bind(&Client::excecuteCQL, shared_from_this(), cql, errorFunc, callback);

  ErrorFunction retryErrorFunc =
    boost::bind(&Client::generic_RetryError, shared_from_this(), _1, errorFunc, retryFunc);
      
  boost::function<void(CassandraConnection::Ptr connection)> executeCqlFunc =
    boost::bind( &CassandraConnection::executeCQL, _1, cql, m_ConnectionFactory->compressCQL(), retryErrorFunc, callback);
    
  ensureConnection(retryErrorFunc, executeCqlFunc);
}

void Client::executePreparedQuery(const PreparedQuery::Ptr & fieldData, ErrorFunction errorFunc, boost::function<void(CQLQueryResult::Ptr)> callback)
{
 boost::function<void()> retryFunc =
    boost::bind(&Client::executePreparedQuery, shared_from_this(), fieldData, errorFunc, callback);

  ErrorFunction retryErrorFunc =
    boost::bind(&Client::generic_RetryError, shared_from_this(), _1, errorFunc, retryFunc);
      
  boost::function<void(CassandraConnection::Ptr connection)> executePreparedQueryFunc =
    boost::bind( &CassandraConnection::executePreparedQuery, _1, fieldData, m_ConnectionFactory->compressCQL(), retryErrorFunc, callback);
    
  ensureConnection(retryErrorFunc, executePreparedQueryFunc);
}

void Client::ensureConnection(ErrorFunction errorFunc, boost::function<void(CassandraConnection::Ptr connection)> continuation)
{
  if (m_Connection.get()){
    continuation(m_Connection);
  } else {
    m_ConnectionFactory->getConnection(errorFunc, boost::bind(&Client::ensureConnection_done, shared_from_this(), _1, continuation));
  }
}

void Client::ensureConnection_done(CassandraConnection::Ptr connection, boost::function<void(CassandraConnection::Ptr connection)> continuation)
{
  m_Connection = connection;
  continuation(m_Connection);
}

void Client::generic_RetryError(const Error& error, ErrorFunction errorFunc, boost::function<void()> retryFunc){

  //drop the cached connection
  if (m_Connection.get()){
    m_Connection->setNeedToCloseWhenDone();
    m_Connection.reset();
  }

  /* abort */
  if (m_RetryCount++ >= m_ConnectionFactory->maxRetryCount() ||
      (error.type != Error::TransportException && error.type != Error::TimedOutException) 
      ){
    errorFunc(error);
    return;
  }

  /*delayed retry*/
  if (error.type == Error::TimedOutException){
    m_Timer.expires_from_now(boost::posix_time::milliseconds(m_ConnectionFactory->retryDelayOnTimeout()));
    m_Timer.async_wait(boost::bind(&Client::generic_ReconnectWaitDone, shared_from_this(), _1, errorFunc, retryFunc));
    return;
  }
  
  /*retry*/
  retryFunc();
}

void Client::generic_ReconnectWaitDone(const boost::system::error_code& error, ErrorFunction errorFunc, boost::function<void()> retryFunc)
{
  if (error){
    errorFunc(Error(Error::TransportException, "Timer error on retry wait"));
    return;
  }
  
  retryFunc();
}

void Client::prepareQuery_done(GlobalPreparedQueryCache::PreparedQueryInfo::Ptr& preparedQueryInfo, boost::function<void(PreparedQuery::Ptr)> callback)
{
 callback(PreparedQuery::Ptr(new PreparedQuery(preparedQueryInfo)));  
}

} //namespace teamspeak
} //namespace agamemnon
