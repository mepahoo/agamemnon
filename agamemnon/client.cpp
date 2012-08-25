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

} //namespace teamspeak
} //namespace agamemnon
